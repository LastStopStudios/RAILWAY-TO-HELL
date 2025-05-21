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
#include "UI.h"


Bufon::Bufon() : Entity(EntityType::BUFON)
{

}

Bufon::~Bufon() {
    delete pathfinding;
    if (area != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(area);
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

    pbody->listener = this;
    //Assign collider type
    pbody->ctype = ColliderType::BUFON;

    pbody->body->SetFixedRotation(false);
    pbody->body->SetGravityScale(1.0f);
    pbody->body->GetFixtureList()->SetDensity(10);
	pbody->body->ResetMassData();
	pbody->body->SetFixedRotation(true);

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

        float jumpThreshold = 8.0f;
        if (distanceToPlayer > jumpThreshold) {
			phase_One = true;
            currentAnimation = &jumping;
			isJumpAttacking = true;

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
        float jumpForceX = 7.0f;
        float jumpForceY = -10.0f;
        float direction = isLookingLeft ? -1.0f : 1.0f;

        pbody->body->SetLinearVelocity(b2Vec2(direction * jumpForceX, jumpForceY));
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
        Engine::GetInstance().ui->vidab2/*= life boss*/;
    }


    //TO SHOW BOSS LIVE ADD On his dialogue before the figth o whatever happens before the boss fight starts
    /*Engine::GetInstance().ui->figth2 = true;//show boss2 health*/

    return true;
}

bool Bufon::CleanUp()
{
    if (area != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(area);
        area = nullptr;
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

void Bufon::OnCollision(PhysBody* physA, PhysBody* physB) {
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
    }
}

void Bufon::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
    switch (physB->ctype)
    {
    case ColliderType::PLAYER:
        LOG("Collision player");
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
