#include "Boss.h"
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
#include "DialogoM.h"
#include "UI.h"


Boss::Boss() : Entity(EntityType::BOSS)
{

}

Boss::~Boss() {
	CleanUp();
    delete pathfinding;
    if (area != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(area);
    }
}
bool Boss::Awake() {
    return true;
}

bool Boss::Start() {

    //initilize textures
    texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
    position.setX(parameters.attribute("x").as_int());
    position.setY(parameters.attribute("y").as_int());
    texW = parameters.attribute("w").as_int();
    texH = parameters.attribute("h").as_int();
	enemyID = parameters.attribute("name").as_string();
    ref = parameters.attribute("ref").as_string();
    initX = parameters.attribute("x").as_int();
    initY = parameters.attribute("y").as_int();

    //Load animations
    idle.LoadAnimations(parameters.child("animations").child("idle"));
    attack.LoadAnimations(parameters.child("animations").child("whipAttack"));
    running.LoadAnimations(parameters.child("animations").child("walking"));
    die.LoadAnimations(parameters.child("animations").child("die"));
    hurt.LoadAnimations(parameters.child("animations").child("hurt"));
    currentAnimation = &idle;

    dialogTriggered = false;
    battleStarted = false;
    isLookingLeft = true;

    // Start with idle animation until dialog is shown
    currentAnimation = &idle;

    //initialize enemy parameters 
    moveSpeed = 80.0f;
    patrolSpeed = 30.0f;
    savedPosX = 0.0f;
    // Physics body initialization - now with two circular collisions
    int radius = texW / 3.1; // Smaller radius than before
    int centerX = (int)position.getX() + texW / 2;
    int centerY = (int)position.getY() + texH / 2;
    // Place upper body higher up
    pbodyUpper = Engine::GetInstance().physics.get()->CreateCircle(
        centerX,
        centerY - radius,  // upper part
        radius,
        bodyType::DYNAMIC);
    pbodyUpper->listener = this;
    pbodyUpper->ctype = ColliderType::BOSS;
    pbodyUpper->body->SetFixedRotation(true);
    // Lower body (legs)
    pbodyLower = Engine::GetInstance().physics.get()->CreateCircle(
        centerX,
        centerY + radius,  // lower part
        radius,
        bodyType::DYNAMIC);
    pbodyLower->listener = this;
    pbodyLower->ctype = ColliderType::BOSS;
    pbodyLower->body->SetFixedRotation(true);
    // Disable gravity
    if (!parameters.attribute("gravity").as_bool()) {
        pbodyUpper->body->SetGravityScale(0.5);
        pbodyLower->body->SetGravityScale(0.5);
    }
    // Verify that bodies exist
    if (!pbodyUpper || !pbodyLower || !pbodyUpper->body || !pbodyLower->body) {
        LOG("Error: One or both Box2D bodies are null");
        return false;
    }
    // Create a weld joint between the two circles
    b2WeldJointDef jointDef;
    jointDef.bodyA = pbodyUpper->body;
    jointDef.bodyB = pbodyLower->body;
    jointDef.collideConnected = false;
    // The point where they touch: right between both
    b2Vec2 worldAnchor = pbodyUpper->body->GetWorldCenter();
    worldAnchor.y += radius; // connection point between both
    jointDef.localAnchorA = pbodyUpper->body->GetLocalPoint(worldAnchor);
    jointDef.localAnchorB = pbodyLower->body->GetLocalPoint(worldAnchor);
    jointDef.referenceAngle = 0;
    // Stiffness configuration
    jointDef.stiffness = 1000.0f;
    jointDef.damping = 0.5f;
    bodyJoint = Engine::GetInstance().physics.get()->world->CreateJoint(&jointDef);
    if (bodyJoint) {
        LOG("Weld joint created successfully");
    }
    else {
        LOG("Failed to create weld joint");
    }
    b2Fixture* fixtureUpper = pbodyUpper->body->GetFixtureList();
    b2Fixture* fixtureLower = pbodyLower->body->GetFixtureList();

    if (fixtureUpper && fixtureLower) {
        fixtureUpper->SetFriction(0.005f);
        fixtureLower->SetFriction(0.005f);
    }

    pbodyUpper->body->GetFixtureList()->SetDensity(1000.0f);
    pbodyLower->body->GetFixtureList()->SetDensity(1000.0f);
    pbodyUpper->body->ResetMassData();
    pbodyLower->body->ResetMassData();

    // Initialize pathfinding
    pathfinding = new Pathfinding();
    ResetPath();

    a = 0;
    kill = 1;

    return true;
}

void Boss::TriggerBossDialog() {
    dialogTriggered = true;
}

bool Boss::Update(float dt)
{
    if (!IsEnabled()) return true;

    bool isGameplay = Engine::GetInstance().scene->GetCurrentState() == SceneState::GAMEPLAY;

    if (!isGameplay) {
        // Stop physics movement when not in gameplay
        if (pbodyUpper != nullptr && pbodyUpper->body != nullptr) {
            pbodyUpper->body->SetLinearVelocity(b2Vec2(0, 0));
            pbodyUpper->body->SetGravityScale(0.0f);
        }
        if (pbodyLower != nullptr && pbodyLower->body != nullptr) {
            pbodyLower->body->SetLinearVelocity(b2Vec2(0, 0));
            pbodyLower->body->SetGravityScale(0.0f);
        }
        return true;
    }
    else {
        // Restore gravity when back in gameplay
        if (pbodyUpper != nullptr && pbodyUpper->body != nullptr) {
            pbodyUpper->body->SetGravityScale(1.0f);
        }
        if (pbodyLower != nullptr && pbodyLower->body != nullptr) {
            pbodyLower->body->SetGravityScale(1.0f);
        }
    }

    if (ishurt) {
        if (pbodyUpper != nullptr && pbodyUpper->body != nullptr) {
            pbodyUpper->body->SetLinearVelocity(b2Vec2(0, 0));
            pbodyUpper->body->SetGravityScale(1.0f);
        }
        if (pbodyLower != nullptr && pbodyLower->body != nullptr) {
            pbodyLower->body->SetLinearVelocity(b2Vec2(0, 0));
            pbodyLower->body->SetGravityScale(1.0f);
        }
        if (hurt.HasFinished()) {
            ishurt = false;
            currentAnimation = &running;

            b2Vec2 velocity = b2Vec2(0, 0);
            if (Engine::GetInstance().dialogoM->bossFightReady && !isAttacking) {
                float speed;
                if (isLookingLeft) {
                    speed = -moveSpeed;
                }
                else {
                    speed = moveSpeed;
                }
                velocity = b2Vec2(PIXEL_TO_METERS(speed), 0);
            }
            pbodyUpper->body->SetLinearVelocity(velocity);
            pbodyLower->body->SetLinearVelocity(velocity);
        }
    }

    if (isDying || isDead) {
        // Asegurar que no haya movimiento durante la muerte
        if (pbodyUpper != nullptr && pbodyUpper->body != nullptr) {
            pbodyUpper->body->SetLinearVelocity(b2Vec2(0, 0));
            pbodyUpper->body->SetGravityScale(1.0f);
        }
        if (pbodyLower != nullptr && pbodyLower->body != nullptr) {
            pbodyLower->body->SetLinearVelocity(b2Vec2(0, 0));
            pbodyLower->body->SetGravityScale(1.0f);
        }

        currentAnimation->Update();

        // If death animation finished, start the timer
        if (currentAnimation->HasFinished() && isDying || currentAnimation->HasFinished() && isDead) {
            // Create the key item before deleting the entity
            Item* item = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
            item->SetParameters(Engine::GetInstance().scene.get()->whipItemConfigNode);
            Engine::GetInstance().scene.get()->itemList.push_back(item);
            item->Start();
            Vector2D pos(position.getX() + texW, position.getY());
            item->SetPosition(pos);
            item->SavePosition("Whip");
            item->SetCreatedTrueInXML();
            
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
        Engine::GetInstance().ui->figth = false;

        // When dying, don't process any other logic
        return true;
    }
    if (Engine::GetInstance().ui->figth == true) {
        //UI Lives
        Engine::GetInstance().ui->vidab = lives;
    }

    enemyPos = GetPosition();
    Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());
    Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
    Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

    if (!ishurt) {
        if (Engine::GetInstance().entityManager->dialogo == false) {

            // Get current velocity of upper body
            b2Vec2 velocityUpper = pbodyUpper->body->GetLinearVelocity();
            b2Vec2 velocityLower = pbodyLower->body->GetLinearVelocity();

            float dx = playerTilePos.getX() - enemyTilePos.getX();
            float dy = playerTilePos.getY() - enemyTilePos.getY();
            // Calculate the distance between the enemy and the player only on the X axis
            float distanceToPlayer = abs(dx);

            // Limit movement towards the player only if within X tiles
            float patrolDistance = 7.0f; // Set the maximum distance for the enemy to chase the player 

            if (!canAttack) { // update the attack cooldown
                currentAttackCooldown -= dt;
                if (currentAttackCooldown <= 0) {
                    canAttack = true;
                    currentAttackCooldown = 0.0f;
                }
            }

            if ((isLookingLeft && dx <= -attackDistance)) {
                pbodyUpper->body->SetLinearVelocity(b2Vec2(0.0f, velocityUpper.y));
                pbodyLower->body->SetLinearVelocity(b2Vec2(0.0f, velocityLower.y));
            }

            isLookingLeft = dx < 0; // Set the direction of the enemy

            if (distanceToPlayer <= attackDistance && canAttack && !isAttacking) { // start attacking
                isAttacking = true;
                canAttack = false;
                currentAttackCooldown = attackCooldown;
                currentAnimation = &attack;
                currentAnimation->Reset();

                if (area == nullptr) { // create the attack area
                    area = Engine::GetInstance().physics.get()->CreateRectangleSensor(
                        (int)position.getX() + (isLookingLeft ? -30 : 160),
                        (int)position.getY() + 50,
                        160, //modify according to the length of the whip
                        40,
                        bodyType::KINEMATIC
                    );
                    area->ctype = ColliderType::BOSS_ATTACK;
                }
            }

            if (isAttacking) {
                pbodyUpper->body->SetLinearVelocity(b2Vec2(0, velocityUpper.y));
                pbodyLower->body->SetLinearVelocity(b2Vec2(0, velocityLower.y));

                if (attack.HasFinished()) { // stop attacking
                    isAttacking = false;
                    currentAnimation = &running;
                    attack.Reset();

                    if (area != nullptr) { // delete the attack area
                        Engine::GetInstance().physics.get()->DeletePhysBody(area);
                        area = nullptr;
                    }
                }
            }
            bool bossFightActive = Engine::GetInstance().dialogoM->bossFightReady;
            if (!isAttacking) {
                if (bossFightActive) { // if player is within patrol distance
                    if (!resting) { // start chasing player
                        resting = true;
                        currentAnimation = &running;
                        running.Reset();
                    }

                    if (currentAnimation == &running) { // pathfinding logic
                        int maxIterations = 100;
                        int iterations = 0;

                        while (pathfinding->pathTiles.empty() && iterations < maxIterations) {
                            pathfinding->PropagateAStar(SQUARED);
                            iterations++;
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
                                float velocityX = isLookingLeft ? -moveSpeed : moveSpeed;
                                // Apply velocity to both bodies
                                pbodyUpper->body->SetLinearVelocity(b2Vec2(PIXEL_TO_METERS(velocityX), velocityUpper.y));
                                pbodyLower->body->SetLinearVelocity(b2Vec2(PIXEL_TO_METERS(velocityX), velocityLower.y));
                            }
                        }
                    }
                }
                else { // idle (not chasing player)
                    if (resting) {
                        resting = false;
                        currentAnimation = &idle;
                        idle.Reset();
                    }
                    pbodyUpper->body->SetLinearVelocity(b2Vec2(0, velocityUpper.y));
                    pbodyLower->body->SetLinearVelocity(b2Vec2(0, velocityLower.y));
                }
            }

            SDL_Rect frame = currentAnimation->GetCurrentFrame();
            offsetX = 0; // used to adjust the position of the sprite

            // change sprite direction
            if (isLookingLeft) {
                flip = SDL_FLIP_HORIZONTAL;
                offsetX = (frame.w - texW); //96-48 
            }
            else {
                flip = SDL_FLIP_NONE;
            }

            // Update position from the upper body's transform
            b2Transform pbodyUpperPos = pbodyUpper->body->GetTransform();
            position.setX(METERS_TO_PIXELS(pbodyUpperPos.p.x) - texH / 2);
            position.setY(METERS_TO_PIXELS(pbodyUpperPos.p.y) - 32);
        }else { pbodyUpper->body->SetLinearVelocity(b2Vec2(0, 0)); pbodyLower->body->SetLinearVelocity(b2Vec2(0, 0));/*frenar el cuerpo*/ currentAnimation = &idle; }
    }

    SDL_Rect frame = currentAnimation->GetCurrentFrame();

    int renderX = (int)position.getX() - offsetX + 16;
    int renderY = (int)position.getY() + texH - 5 - frame.h; // Centrado vertical

    Engine::GetInstance().render.get()->DrawTexture(
        texture,
        renderX,
        renderY,
        &frame,
        1.0f,
        0.0,
        INT_MAX,
        INT_MAX,
        flip
    );

    currentAnimation->Update();
    
    
    // pathfinding drawing
    pathfinding->DrawPath();
    pathfinding->ResetPath(enemyTilePos);

    return true;
}

void Boss::Matar() {//eliminating the enemy once dead 
    if (kill == 1) {
        kill = 2;
        Disable();//when it has to be activated use  Enable();
        //Engine::GetInstance().entityManager.get()->DestroyEntity(this);
    }
}

void Boss::SetDeathInXML()
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

void Boss::SetAliveInXML()
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

void Boss::SetSavedDeathToDeathInXML()
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

void Boss::SetSavedDeathToAliveInXML()
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

void Boss::SetEnabled(bool active) {
    isEnabled = active;
    pbodyUpper->body->SetEnabled(active);
    pbodyUpper->body->SetAwake(active);
    pbodyLower->body->SetEnabled(active);
    pbodyLower->body->SetEnabled(active);
}

bool Boss::CleanUp()
{
    // First delete all physics bodies
    if (area != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(area);
        area = nullptr;
    }

    if (pbodyUpper != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(pbodyUpper);
        pbodyUpper = nullptr;
    }

    if (pbodyLower != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(pbodyLower);
        pbodyLower = nullptr;
    }

    // Set joint to null after deleting bodies
    bodyJoint = nullptr;

    // Make sure textures are properly released
    // We don't need to destroy the texture as it's managed elsewhere
    // Just set our pointer to null to avoid double deletion issues
    //if (texture != nullptr) {
    //    SDL_DestroyTexture(texture);
    //    texture = nullptr;
    //}
    return true;
}

void Boss::SetPosition(Vector2D pos) {
    float adjustment = 3.95;

    // Establish upper body position
    pos.setX(pos.getX());
    pos.setY(pos.getY() + texH / 3 + adjustment);
    b2Vec2 upperPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
    pbodyUpper->body->SetTransform(upperPos, 0);

    // Establish lower body position
    Vector2D lowerPos = pos;
    lowerPos.setY(pos.getY() + texH / 3 + adjustment);
    b2Vec2 lowerPosB2 = b2Vec2(PIXEL_TO_METERS(lowerPos.getX()), PIXEL_TO_METERS(lowerPos.getY()));
    pbodyLower->body->SetTransform(lowerPosB2, 0);
}

Vector2D Boss::GetPosition() {
    float adjustment = 3.95;

    // We use the upper body for position tracking
    b2Vec2 bodyPos = pbodyUpper->body->GetTransform().p;
    Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
    pos.setY(pos.getY() - texH / 3 + adjustment);
    return pos;
}

void Boss::ResetPath() {
    Vector2D pos = GetPosition();
    Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
    pathfinding->ResetPath(tilePos);
}

void Boss::ResetPosition() {
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

void Boss::SavePosition(std::string name) {
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

void Boss::ResetLives() {
    lives = 12;
	currentAnimation = &idle;
    if(isDying) isDying = false;
    if(isDead) isDead = false;
}

// Fix for hurt animation in OnCollision
void Boss::OnCollision(PhysBody* physA, PhysBody* physB) {
    // First check if the collision involves one of our bodies
    if (physA != pbodyUpper && physA != pbodyLower) {
        return;
    }
    switch (physB->ctype)
    {
    case ColliderType::PLAYER:
        // Player collision handling
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

                    // Add cleanup when getting hurt
                    if (isAttacking) {
                        isAttacking = false;
                        if (area != nullptr) {
                            Engine::GetInstance().physics.get()->DeletePhysBody(area);
                            area = nullptr;
                        }
                    }
                }
                else if (lives <= 0 && !isDying) {
                    // Same cleanup for death case
                    if (isAttacking) {
                        isAttacking = false;
                        if (area != nullptr) {
                            Engine::GetInstance().physics.get()->DeletePhysBody(area);
                            area = nullptr;
                        }
                    }
                    isDying = true;
                    currentAnimation = &die;
                    currentAnimation->Reset();
                    // Stop both bodies
                    pbodyUpper->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
                    pbodyLower->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
                    Engine::GetInstance().scene->DesbloquearSensor();//Unblock scene change sensors
                    // Engine::GetInstance().dialogoM->Texto("2");//text after boss death
                     // Engine::GetInstance().audio.get()->PlayFx(deathFx);
                }
            }
        }
        break; 

    case ColliderType::PLAYER_WHIP_ATTACK:
        if(Hiteado == false){
            Hiteado = true;

            if (lives > 0) {
                lives = lives - 2;  //It should be - 2, 6 - 2 = 4 so the boss should die on the third hit but for some reason if i do it like that he dies in two hits
                if (lives > 0) {
                    ishurt = true;
                    currentAnimation = &hurt;
                    hurt.Reset();
                }
                else if (lives <= 0 && !isDead) {
                    isDead = true;
                    currentAnimation = &die;
                    a = 1;
                    Engine::GetInstance().scene->DesbloquearSensor();//Unblock scene change sensors
                    Engine::GetInstance().dialogoM->Texto("2");//text after boss death 

                    // Stop both bodies
                    pbodyUpper->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
                    pbodyLower->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));

                    // Disable gravity on both bodies
                    pbodyUpper->body->SetGravityScale(0.0f);
                    pbodyLower->body->SetGravityScale(0.0f);
                }
            }
        }
        break;
                                  
    }
}

void Boss::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
    // Check if the collision involves one of our bodies
    if (physA != pbodyUpper && physA != pbodyLower) {
        return;
    }

    switch (physB->ctype)
    {
    case ColliderType::PLAYER:
        break;
    case ColliderType::PLAYER_WHIP_ATTACK:
        Hiteado = false;
        break;
    case ColliderType::PLAYER_ATTACK:
        Hiteado = false;
        break;
    }
}