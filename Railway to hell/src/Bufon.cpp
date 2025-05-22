#include "Bufon.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "Map.h"
#include "EntityManager.h"

Bufon::Bufon() : Entity(EntityType::BUFON)
{

}

Bufon::~Bufon() {
    delete pathfinding;
    if (jumpAttackArea != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(jumpAttackArea);
    }
}
bool Bufon::Awake() {
    return true;
}

bool Bufon::Start() {

    //initilize textures
    texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
    position.setX(parameters.attribute("x").as_int());
    position.setY(parameters.attribute("y").as_int());
    texW = parameters.attribute("w").as_int();
    texH = parameters.attribute("h").as_int();

    //Load animations
    idle.LoadAnimations(parameters.child("animations").child("idle"));
    hurt.LoadAnimations(parameters.child("animations").child("hurt"));
    die.LoadAnimations(parameters.child("animations").child("die"));
    disparoR.LoadAnimations(parameters.child("animations").child("disparoR"));
	disparoG.LoadAnimations(parameters.child("animations").child("disparoG"));
	jumping.LoadAnimations(parameters.child("animations").child("jumping"));
	going_up.LoadAnimations(parameters.child("animations").child("going_up"));
	going_down.LoadAnimations(parameters.child("animations").child("going_down"));
	impacting.LoadAnimations(parameters.child("animations").child("impacting"));

    currentAnimation = &idle;

    //initialize enemy parameters 
    moveSpeed = 30.0f;
    patrolSpeed = 30.0f;
    savedPosX = 0.0f;

    //Add a physics to an item - initialize the physics body
    pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texW / 2, texH, bodyType::DYNAMIC);
    jumpAttackArea = Engine::GetInstance().physics.get()->CreateRectangleSensor((int)position.getX() + texH / 2, (int)position.getY() + texH, texW / 2 - 24, 8, bodyType::DYNAMIC);

    pbody->listener = this;
	jumpAttackArea->listener = this;
    //Assign collider type
    pbody->ctype = ColliderType::BUFON;
	jumpAttackArea->ctype = ColliderType::BUFON_JUMP_ATTACK_AREA;

    pbody->body->SetFixedRotation(true);
	jumpAttackArea->body->SetFixedRotation(true);
    pbody->body->SetGravityScale(1.0f);
    pbody->body->GetFixtureList()->SetDensity(10);
	pbody->body->ResetMassData();

 //   b2Fixture* fixture = pbody->body->GetFixtureList();
	//fixture->SetFriction(0.0f); // Set friction to 0.0f

    // Set the gravity of the body
    if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(2.0f);

    // Initialize pathfinding
    pathfinding = new Pathfinding();
    ResetPath();

    return true;
}

bool Bufon::Update(float dt)
{

    bool isGameplay = Engine::GetInstance().scene->GetCurrentState() == SceneState::GAMEPLAY;

    if (!isGameplay) {

        if (pbody != nullptr && pbody->body != nullptr) {
            pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            pbody->body->SetGravityScale(0.0f);
        }
        return true;
    }
    else {

        if (pbody != nullptr && pbody->body != nullptr) {
            pbody->body->SetGravityScale(1.0f);
        }
    }

    if (jumpAttackArea != nullptr) {
        int sensorX = position.getX() + texH / 2;
        int sensorY = position.getY() + texH;
        jumpAttackArea->body->SetTransform(b2Vec2(PIXEL_TO_METERS(sensorX), PIXEL_TO_METERS(sensorY)), 0);
    }

    enemyPos = GetPosition();
    Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());
    Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
    Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

    b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);

    float dx = playerTilePos.getX() - enemyTilePos.getX();
    float dy = playerTilePos.getY() - enemyTilePos.getY();
    // Calculate the distance between the enemy and the player only on the X axis
    float distanceToPlayer = abs(dx);

    // Limit movement towards the player only if within X tiles
    float patrolDistance = 12.0f; // Set the maximum distance for the enemy to chase the player 

    if (ishurt) {
        if (hurt.HasFinished()) {
            ishurt = false;
            currentAnimation = &idle;
        }
    }

    if (isDying || isDead) {

        currentAnimation->Update();

        // If death animation finished, start the timer
        if (currentAnimation->HasFinished() && isDying || currentAnimation->HasFinished() && isDead) {
            // Create the key item before deleting the entity
            Item* item = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
            item->SetParameters(Engine::GetInstance().scene.get()->ballItemConfigNode);
            Engine::GetInstance().scene.get()->itemList.push_back(item);
            item->Start();
            Vector2D pos(position.getX() + texW, position.getY());
            item->SetPosition(pos);
            item->SavePosition("Ball");
            item->SetCreatedTrueInXML();
            if (changeMusicBoss)
            {
                Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/Background.ogg", 0.0f);
                changeMusicBoss = false;
            }
            if (pendingDisable) {
                SetEnabled(false);
                pendingDisable = false;
                SetDeathInXML();
            }
        }

        // Draw the death animation
        SDL_RendererFlip flip = isLookingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY() - 32, &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, flip);

        //UI Lives
        Engine::GetInstance().ui->figth2 = false;

        // When dying, don't process any other logic
        return true;
    }
    if (Engine::GetInstance().ui->figth2 == true) {
        //UI Lives
        Engine::GetInstance().ui->vidab2 = lives;
    }

    if (!ishurt) {
        if (currentAnimation == &going_up || currentAnimation == &going_down) {
            pathfindingTimer += dt;

            int maxIterations = 100;
            int iterations = 0;

            while (pathfinding->pathTiles.empty() && iterations < maxIterations && pathfindingTimer < maxPathfindingTime) {
                pathfinding->PropagateAStar(SQUARED);
                iterations++;

            }
            if (pathfindingTimer >= maxPathfindingTime) {
                // stop movement in X
                b2Vec2 currentVelocity = pbody->body->GetLinearVelocity();
                pbody->body->SetLinearVelocity(b2Vec2(0.0f, currentVelocity.y));
            }
            if (pathfinding->pathTiles.size() >= 2) {
                auto it = pathfinding->pathTiles.end();
                --it;
                --it;
                Vector2D nextTile = *it;
                Vector2D nextPos = Engine::GetInstance().map.get()->MapToWorld(nextTile.getX(), nextTile.getY());

                float x2 = nextPos.getX();
                float x1 = enemyPos.getX();
                float dx = x2 - x1;
                float distance = abs(dx);

                if (distance < 5.0f) {
                    pathfinding->pathTiles.pop_back();
                }
                else {
                    float jumpForceX = isLookingLeft ? 8.0f : 8.0f;
                    float jumpForceY = -10.0f;
                    float direction = isLookingLeft ? -1.0f : 1.0f;

                    pbody->body->SetLinearVelocity(b2Vec2(direction * jumpForceX, pbody->body->GetLinearVelocity().y));
                }
            }
        }
        if (currentAnimation != &going_up && currentAnimation != &going_down) {
            pathfindingTimer = 0.0f;
        }
        if (!canAttack) { // update the attack cooldown
            currentAttackCooldown -= dt;
            if (currentAttackCooldown <= 0) {
                canAttack = true;
                currentAttackCooldown = 0.0f;
            }
        }

        if ((isLookingLeft && dx <= -attackDistance)) {
            //pbody->body->SetLinearVelocity(b2Vec2(0.0f, pbody->body->GetLinearVelocity().y));
        }

        isLookingLeft = dx < 0; // Set the direction of the enemy

        if (distanceToPlayer <= attackDistance && canAttack && !isAttacking) {

            float jumpThreshold = 10.0f;
            if (distanceToPlayer > jumpThreshold) {
                phase_One = true;
                currentAnimation = &jumping;
                isJumpAttacking = true;
                float jumpForceX = isLookingLeft ? 5.0f : 5.0f;
                float jumpForceY = -10.0f;
                float direction = isLookingLeft ? -1.0f : 1.0f;
                pbody->body->SetLinearVelocity(b2Vec2(direction * jumpForceX, jumpForceY));

            }
            else {
                currentAnimation = &disparoR;
            }

            isAttacking = true;
            canAttack = false;
            currentAttackCooldown = attackCooldown;
            currentAnimation->Reset();
        }

        if (jumping.HasFinished()) {
            phase_Two = true;
            currentAnimation = &going_up;


            //float jumpForceY = -10.0f;
            //float direction = isLookingLeft ? -1.0f : 1.0f;

            //pbody->body->SetLinearVelocity(b2Vec2(direction * jumpForceX, jumpForceY));
           // pbody->body->ApplyLinearImpulse(b2Vec2(direction * jumpForceX, jumpForceY), pbody->body->GetWorldCenter(), true);
            jumping.Reset();

        }
        if (going_up.HasFinished() && pbody->body->GetLinearVelocity().y > 0) {
            phase_Three = true;
            currentAnimation = &going_down;
            going_up.Reset();
        }

        if (isAttacking) {
            if (currentAnimation == &going_up) {
                pbody->body->SetLinearVelocity(b2Vec2(pbody->body->GetLinearVelocity().x, pbody->body->GetLinearVelocity().y));

            }
            else {
                pbody->body->SetLinearVelocity(b2Vec2(pbody->body->GetLinearVelocity().x, pbody->body->GetLinearVelocity().y));
            }

            if (disparoR.HasFinished()) { // stop attacking
                Projectiles* projectile = (Projectiles*)Engine::GetInstance().entityManager->CreateEntity(EntityType::PROJECTILE);
                projectile->SetParameters(Engine::GetInstance().scene.get()->normalProjectileConfigNode);
                projectile->Start();
                Vector2D bufonPosition = GetPosition();
                // Adjust horizontal position based on facing direction
                if (isLookingLeft) bufonPosition.setX(bufonPosition.getX() - 50);
                else bufonPosition.setX(bufonPosition.getX() + 20);
                projectile->SetPosition(bufonPosition);
                projectile->SetDirection(!isLookingLeft);
                isAttacking = false;
                currentAnimation = &idle;
                disparoR.Reset();
            }
            if (impacting.HasFinished()) { // stop attacking
                phase_One, phase_Two, phase_Three = false;
                isJumpAttacking = false;
                isAttacking = false;
                currentAnimation = &idle;
                impacting.Reset();
                pbody->body->GetFixtureList()->SetDensity(500);
                pbody->body->ResetMassData();
            }
        }

        if (!isAttacking) {
            if (distanceToPlayer <= patrolDistance) { // if player is within patrol distance
                //if (!resting) { // start chasing player
                //    resting = true;
                //    currentAnimation = &salto;
                //    salto.Reset();
                //}
            }
            else { // idle (not chasing player)
                if (resting) {
                    resting = false;
                    currentAnimation = &idle;
                    idle.Reset();
                }
                b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);
                pbody->body->SetLinearVelocity(velocity);
            }
        }
    }

    SDL_Rect frame = currentAnimation->GetCurrentFrame();
    int offsetX = 0; // used to adjust the position of the sprite

    // change sprite direction
    if (currentAnimation == &idle || currentAnimation == &hurt) {
        if (isLookingLeft) {
            flip = SDL_FLIP_NONE;
            offsetX = (frame.w - texW) ;
        }
        else {
            flip = SDL_FLIP_HORIZONTAL;
            offsetX = -8;
        }
    }
    else if (currentAnimation == &disparoR || currentAnimation == &disparoG || currentAnimation == &jumping || currentAnimation == &going_up || currentAnimation == &going_down || currentAnimation == &impacting || currentAnimation == &die) {
        if (isLookingLeft) {
            flip = SDL_FLIP_NONE;
            offsetX = ((frame.w - texW)/2 - 24);
        }
        else {
            flip = SDL_FLIP_HORIZONTAL;
            offsetX = -8;
        }
    }
    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

    if (currentAnimation == &idle || currentAnimation == &hurt) {
        Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() -4 - offsetX, (int)position.getY(), &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, flip);
    }
    else if (currentAnimation == &disparoR || currentAnimation == &disparoG || currentAnimation == &jumping || currentAnimation == &going_up || currentAnimation == &going_down || currentAnimation == &impacting || currentAnimation == &die) {
        Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() - 36 + offsetX, (int)position.getY() - 64, &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, flip);
    }


    currentAnimation->Update();

    // pathfinding drawing
    pathfinding->DrawPath();
    pathfinding->ResetPath(enemyTilePos);

    //Inside isDying add
    /* //UI Lives
    Engine::GetInstance().ui->figth2 = false;*/

    if (Engine::GetInstance().ui->figth2 == true) {
        //UI Lives
        Engine::GetInstance().ui->vidab2 = lives;
    }

    //TO SHOW BOSS LIVE ADD On his dialogue before the figth o whatever happens before the boss fight starts
    /*Engine::GetInstance().ui->figth2 = true;//show boss2 health*/

    return true;
}

void Bufon::SetDeathInXML()
{
    // Load XML
    pugi::xml_document doc;
    if (!doc.load_file("config.xml")) {
        LOG("Error loading config.xml");
        return;
    }

    pugi::xml_node bossNode;
    int currentScene = Engine::GetInstance().sceneLoader.get()->GetCurrentLevel();

    if (currentScene == 1) {
        bossNode = doc.child("config")
            .child("scene")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 2) {
        bossNode = doc.child("config")
            .child("scene2")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 3) {
        bossNode = doc.child("config")
            .child("scene3")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 4) {
        bossNode = doc.child("config")
            .child("scene4")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 5) {
        bossNode = doc.child("config")
            .child("scene5")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 6) {
        bossNode = doc.child("config")
            .child("scene6")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 7) {
        bossNode = doc.child("config")
            .child("scene7")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 8) {
        bossNode = doc.child("config")
            .child("scene8")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 9) {
        bossNode = doc.child("config")
            .child("scene9")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 10) {
        bossNode = doc.child("config")
            .child("scene10")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 11) {
        bossNode = doc.child("config")
            .child("scene11")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }

    if (!bossNode) {
        LOG("Could not find the node for boss in the XML");
        return;
    }

    bossNode.attribute("death").set_value(1); // 1 enemy is death

    if (!doc.save_file("config.xml")) {
        LOG("Error saving config.xml");
    }
    else {
        LOG("death status updated in the XML for boss");
    }
    DeathValue = 1;
}

void Bufon::SetAliveInXML()
{
    // Load XML file
    pugi::xml_document doc;
    if (!doc.load_file("config.xml")) {
        LOG("Error loading config.xml");
        return;
    }

    pugi::xml_node bossNode;
    int currentScene = Engine::GetInstance().sceneLoader.get()->GetCurrentLevel();

    if (currentScene == 1) {
        bossNode = doc.child("config")
            .child("scene")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 2) {
        bossNode = doc.child("config")
            .child("scene2")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 3) {
        bossNode = doc.child("config")
            .child("scene3")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 4) {
        bossNode = doc.child("config")
            .child("scene4")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 5) {
        bossNode = doc.child("config")
            .child("scene5")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 6) {
        bossNode = doc.child("config")
            .child("scene6")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 7) {
        bossNode = doc.child("config")
            .child("scene7")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 8) {
        bossNode = doc.child("config")
            .child("scene8")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 9) {
        bossNode = doc.child("config")
            .child("scene9")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 10) {
        bossNode = doc.child("config")
            .child("scene10")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 11) {
        bossNode = doc.child("config")
            .child("scene11")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }

    if (!bossNode) {
        LOG("Could not find the node for boss in the XML");
        return;
    }

    bossNode.attribute("death").set_value(0); // 0 enemy is alive

    if (!doc.save_file("config.xml")) {
        LOG("Error saving config.xml");
    }
    else {
        LOG("death status updated in the XML for boss");
    }
    DeathValue = 0;
}

void Bufon::SetSavedDeathToDeathInXML()
{
    // Load XML
    pugi::xml_document doc;
    if (!doc.load_file("config.xml")) {
        LOG("Error loading config.xml");
        return;
    }

    pugi::xml_node bossNode;
    int currentScene = Engine::GetInstance().sceneLoader.get()->GetCurrentLevel();

    if (currentScene == 1) {
        bossNode = doc.child("config")
            .child("scene")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 2) {
        bossNode = doc.child("config")
            .child("scene2")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 3) {
        bossNode = doc.child("config")
            .child("scene3")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 4) {
        bossNode = doc.child("config")
            .child("scene4")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 5) {
        bossNode = doc.child("config")
            .child("scene5")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 6) {
        bossNode = doc.child("config")
            .child("scene6")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 7) {
        bossNode = doc.child("config")
            .child("scene7")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 8) {
        bossNode = doc.child("config")
            .child("scene8")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 9) {
        bossNode = doc.child("config")
            .child("scene9")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 10) {
        bossNode = doc.child("config")
            .child("scene10")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 11) {
        bossNode = doc.child("config")
            .child("scene11")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }

    if (!bossNode) {
        LOG("Could not find the node for boss in the XML");
        return;
    }

    bossNode.attribute("savedDeath").set_value(1); // 1 enemy is death

    if (!doc.save_file("config.xml")) {
        LOG("Error saving config.xml");
    }
    else {
        LOG("death status updated in the XML for boss");
    }

    SavedDeathValue = 1;
}

void Bufon::SetSavedDeathToAliveInXML()
{
    // Load XML file
    pugi::xml_document doc;
    if (!doc.load_file("config.xml")) {
        LOG("Error loading config.xml");
        return;
    }

    pugi::xml_node bossNode;
    int currentScene = Engine::GetInstance().sceneLoader.get()->GetCurrentLevel();

    if (currentScene == 1) {
        bossNode = doc.child("config")
            .child("scene")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 2) {
        bossNode = doc.child("config")
            .child("scene2")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 3) {
        bossNode = doc.child("config")
            .child("scene3")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 4) {
        bossNode = doc.child("config")
            .child("scene4")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 5) {
        bossNode = doc.child("config")
            .child("scene5")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 6) {
        bossNode = doc.child("config")
            .child("scene6")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 7) {
        bossNode = doc.child("config")
            .child("scene7")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 8) {
        bossNode = doc.child("config")
            .child("scene8")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 9) {
        bossNode = doc.child("config")
            .child("scene9")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 10) {
        bossNode = doc.child("config")
            .child("scene10")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 11) {
        bossNode = doc.child("config")
            .child("scene11")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }

    if (!bossNode) {
        LOG("Could not find the node for item in the XML");
        return;
    }

    bossNode.attribute("savedDeath").set_value(0); // 0 enemy is alive

    if (!doc.save_file("config.xml")) {
        LOG("Error saving config.xml");
    }
    else {
        LOG("death status updated in the XML for boss");
    }

    SavedDeathValue = 0;
}

void Bufon::SetEnabled(bool active) {
    isEnabled = active;
    pbody->body->SetEnabled(active);
    pbody->body->SetAwake(active);
}

bool Bufon::CleanUp()
{
    if (jumpAttackArea != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(jumpAttackArea);
        jumpAttackArea = nullptr;
    }

    Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
    return true;
}

void Bufon::SetPosition(Vector2D pos) {
    pos.setX(pos.getX() + texW / 2);
    pos.setY(pos.getY() + texH / 2);
    b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
    pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Bufon::GetPosition() {
    b2Vec2 bodyPos = pbody->body->GetTransform().p;
    Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
    return pos;
}

void Bufon::ResetPath() {
    Vector2D pos = GetPosition();
    Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
    pathfinding->ResetPath(tilePos);
}

void Bufon::ResetPosition() {
    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");

    if (result == NULL)
    {
        LOG("Could not load file. Pugi error: %s", result.description());
        return;
    }

    pugi::xml_node sceneNode;

    int currentScene = Engine::GetInstance().sceneLoader.get()->GetCurrentLevel();
    if (currentScene == 1) {
        sceneNode = loadFile.child("config").child("scene");
    }
    if (currentScene == 2) {
        sceneNode = loadFile.child("config").child("scene2");
    }
    if (currentScene == 3) {
        sceneNode = loadFile.child("config").child("scene3");
    }
    if (currentScene == 4) {
        sceneNode = loadFile.child("config").child("scene4");
    }
    if (currentScene == 5) {
        sceneNode = loadFile.child("config").child("scene5");
    }
    if (currentScene == 6) {
        sceneNode = loadFile.child("config").child("scene6");
    }
    if (currentScene == 7) {
        sceneNode = loadFile.child("config").child("scene7");
    }
    if (currentScene == 8) {
        sceneNode = loadFile.child("config").child("scene8");
    }
    if (currentScene == 9) {
        sceneNode = loadFile.child("config").child("scene9");
    }
    if (currentScene == 10) {
        sceneNode = loadFile.child("config").child("scene10");
    }
    if (currentScene == 11) {
        sceneNode = loadFile.child("config").child("scene11");
    }

    //bosses
    pugi::xml_node bossesNode = sceneNode.child("entities").child("bosses");
    if (!Engine::GetInstance().scene.get()->bossList.empty()) {
        for (pugi::xml_node bossNode : bossesNode.children("boss")) {
            std::string xmlRef = bossNode.attribute("ref").as_string();
            for (const auto& boss : Engine::GetInstance().scene.get()->bossList) {
                if (boss->GetRef() == xmlRef) {
                    bossNode.attribute("x").set_value(initX);
                    bossNode.attribute("y").set_value(initY);
                }
            }
        }
    }

    //Saves the modifications to the XML 
    loadFile.save_file("config.xml");

}

void Bufon::SavePosition(std::string name) {
    // Save the current position of the boss in the XML file
    pugi::xml_document doc;
    if (!doc.load_file("config.xml")) {
        LOG("Error loading config.xml");
        return;
    }

    pugi::xml_node sceneNode;
    int currentScene = Engine::GetInstance().sceneLoader.get()->GetCurrentLevel();

    if (currentScene == 1) {
        sceneNode = doc.child("config")
            .child("scene")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 2) {
        sceneNode = doc.child("config")
            .child("scene2")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 3) {
        sceneNode = doc.child("config")
            .child("scene3")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 4) {
        sceneNode = doc.child("config")
            .child("scene4")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 5) {
        sceneNode = doc.child("config")
            .child("scene5")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 6) {
        sceneNode = doc.child("config")
            .child("scene6")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 7) {
        sceneNode = doc.child("config")
            .child("scene7")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 8) {
        sceneNode = doc.child("config")
            .child("scene8")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 9) {
        sceneNode = doc.child("config")
            .child("scene9")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 10) {
        sceneNode = doc.child("config")
            .child("scene10")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }
    else if (currentScene == 11) {
        sceneNode = doc.child("config")
            .child("scene11")
            .child("entities")
            .child("enemies")
            .find_child_by_attribute("enemy", "name", enemyID.c_str());
    }

    if (!sceneNode) {
        LOG("Could not find the node for item in the XML");
        return;
    }

    //Save info to XML 

    //boss
    pugi::xml_node bossesNode = sceneNode.child("entities").child("bosses");
    if (!Engine::GetInstance().scene.get()->bossList.empty()) {
        int i = 0;
        for (pugi::xml_node bossNode : bossesNode.children("boss")) {
            if (i < Engine::GetInstance().scene.get()->bossList.size()) {
                std::string enemyID = bossNode.attribute("name").as_string();
                if (enemyID == name) {
                    bossNode.attribute("x").set_value(Engine::GetInstance().scene.get()->bossList[i]->GetPosition().getX());
                    bossNode.attribute("y").set_value(Engine::GetInstance().scene.get()->bossList[i]->GetPosition().getY());
                }
                i++;
            }

        }
    }

    if (!doc.save_file("config.xml")) {
        LOG("Error saving config.xml");
    }
}

void Bufon::ResetLives() {
    lives = 6;
    currentAnimation = &idle;
    if (isDying) isDying = false;
    if (isDead) isDead = false;
}

void Bufon::OnCollision(PhysBody* physA, PhysBody* physB) {

    if (physA == jumpAttackArea) {
        if (physA->ctype == ColliderType::BUFON_JUMP_ATTACK_AREA && physB->ctype == ColliderType::PLAYER) {
            if (isJumpAttacking && phase_One && phase_Two && phase_Three) {
                pbody->body->GetFixtureList()->SetDensity(1000);
                pbody->body->ResetMassData();
            }
            return;
        }
    }

    switch (physB->ctype)
    {
    case ColliderType::PLAYER:
        LOG("Collided with player - DESTROY");

        //Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        break;

    case ColliderType::PLATFORM:
        if (isJumpAttacking && phase_One && phase_Two && phase_Three) {
			pbody->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
            currentAnimation = &impacting;
        }
        break;
    case ColliderType::PLAYER_ATTACK:
        if (Hiteado == false) {
            Hiteado = true;
            if (lives > 0) {
                lives = lives - 1;
                if (lives > 0) {
                    ishurt = true;
                    currentAnimation = &hurt;
                    hurt.Reset();

					// stop attacking
                    if (isAttacking) {
                        isAttacking = false;
                        pbody->body->SetLinearVelocity(b2Vec2(0.0f, pbody->body->GetLinearVelocity().y));
                    }
                }
                else if (lives <= 0 && !isDying) {
                    // Same cleanup for death case
                    if (isAttacking) {
                        isAttacking = false;
                        pbody->body->SetLinearVelocity(b2Vec2(0.0f, pbody->body->GetLinearVelocity().y));
                    }
                    pendingDisable = true;
                    isDying = true;
                    currentAnimation = &die;
                    currentAnimation->Reset();
                    Engine::GetInstance().scene->DesbloquearSensor();//Unblock scene change sensors
                    // Engine::GetInstance().dialogoM->Texto("2");//text after boss death
                     // Engine::GetInstance().audio.get()->PlayFx(deathFx);
                    changeMusicBoss = true;
                }
            }
        }
        break;

    case ColliderType::PLAYER_WHIP_ATTACK:
        if (Hiteado == false) {
            Hiteado = true;

            if (lives > 0) {
                lives = lives - 2;  //It should be - 2, 6 - 2 = 4 so the boss should die on the third hit but for some reason if i do it like that he dies in two hits
                if (lives > 0) {
                    ishurt = true;
                    currentAnimation = &hurt;
                    hurt.Reset();
                    if (isAttacking) {
                        isAttacking = false;
                        pbody->body->SetLinearVelocity(b2Vec2(0.0f, pbody->body->GetLinearVelocity().y));
                    }
                }
                else if (lives <= 0 && !isDead) {
                    pendingDisable = true;
                    isDead = true;
                    if (isAttacking) {
                        isAttacking = false;
                        pbody->body->SetLinearVelocity(b2Vec2(0.0f, pbody->body->GetLinearVelocity().y));
                    }
                    currentAnimation = &die;
                    a = 1;
                    Engine::GetInstance().scene->DesbloquearSensor();//Unblock scene change sensors

                    changeMusicBoss = true;
                }
            }
        }
        break;

    }

}

void Bufon::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
    switch (physB->ctype)
    {
    case ColliderType::PLAYER:
        LOG("Collision player");
        break;
    case ColliderType::PLAYER_WHIP_ATTACK:
        Hiteado = false;
        break;
    case ColliderType::PLAYER_ATTACK:
        Hiteado = false;
        break;
    }

}

// Helper methods for interacting with Animation
int Bufon::GetCurrentFrameId() const {
    if (currentAnimation != nullptr) {
        // Use the GetCurrentFrameIndex method from Animation
        return currentAnimation->GetCurrentFrameIndex();
    }
    return 0;
}

int Bufon::GetTotalFrames() const {
    if (currentAnimation != nullptr) {
        // Access totalFrames from Animation
        return currentAnimation->totalFrames;
    }
    return 0;
}
