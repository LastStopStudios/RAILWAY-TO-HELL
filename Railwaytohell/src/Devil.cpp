
#include "Devil.h"
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

Devil::Devil() : Entity(EntityType::DEVIL)
{
    currentPhase = 1;
}

Devil::~Devil() {
    if (punchAttackArea != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(punchAttackArea);
    }
}

bool Devil::Awake() {
    return true;
}

bool Devil::Start() {
    // Initialize textures
    texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
    position.setX(parameters.attribute("x").as_int());
    position.setY(parameters.attribute("y").as_int());
    texW = parameters.attribute("w").as_int();
    texH = parameters.attribute("h").as_int();
    initX = parameters.attribute("x").as_int();
    initY = parameters.attribute("y").as_int();
    intitalPosX = parameters.attribute("x").as_int();

    // Load animations
    idle.LoadAnimations(parameters.child("animations").child("idle"));
    walk.LoadAnimations(parameters.child("animations").child("walk"));
    punch.LoadAnimations(parameters.child("animations").child("punch"));
    defeat.LoadAnimations(parameters.child("animations").child("defeat"));

    currentAnimation = &idle;

    // Initialize enemy parameters 
    moveSpeed = 2.0f;
    patrolSpeed = 3.0f;

    // Add physics body
    pbody = Engine::GetInstance().physics.get()->CreateCircle(
        (int)position.getX() + texW / 2,
        (int)position.getY() + texH / 2,
        texW / 2,
        bodyType::DYNAMIC
    );

    // Assign collider types
    pbody->ctype = ColliderType::DEVIL;

    pbody->body->SetGravityScale(1.0f);

    return true;
}

bool Devil::Update(float dt) {
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

    // Update punch attack area position
    if (punchAttackArea != nullptr) {
        int punchX = position.getX() + texW / 2;
        int punchY = position.getY() + texH / 2;
        if (!isLookingLeft) {
            punchX += 30; // Offset punch area to the right
        }
        else {
            punchX -= 30; // Offset punch area to the left
        }
        punchAttackArea->body->SetTransform(b2Vec2(PIXEL_TO_METERS(punchX), PIXEL_TO_METERS(punchY)), 0);
    }

    enemyPos = GetPosition();
    Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
    Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());
    Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

    b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);

    float dx = playerTilePos.getX() - enemyTilePos.getX();
    float distanceToPlayer = abs(dx);

    float patrolDistance = 12.0f;

    // Handle defeat state
    if (isDying || isDead) {
        HandleDefeatState();
        return true;
    }

    // Update UI if fighting
    if (Engine::GetInstance().ui->figth2 == true) {
        Engine::GetInstance().ui->vidab2 = lives;
    }

    // Handle hurt state
    if (ishurt) {
        currentAnimation->Update();
        if (currentAnimation->HasFinished()) {
            ishurt = false;
            escaping = true;
            currentAnimation = &walk;

            // Move away from player after being hurt
            float direction = (dx > 0) ? -1.0f : 1.0f;
            pbody->body->SetLinearVelocity(b2Vec2(direction * moveSpeed * 1.2f, pbody->body->GetLinearVelocity().y));
        }

        RenderSprite();
        return true;
    }

    // Update attack cooldown
    if (!canAttack) {
        currentAttackCooldown -= dt;
        if (currentAttackCooldown <= 0) {
            canAttack = true;
            currentAttackCooldown = 0.0f;
        }
    }

    // Update escaping behavior
    if (escaping) {
        currentAnimation->Update();

        // Check if far enough from initial position to stop escaping
        float distanceFromStart = abs(position.getX() - intitalPosX);
        if (distanceFromStart > 100.0f) {
            escaping = false;
            currentAnimation = &idle;
            pbody->body->SetLinearVelocity(b2Vec2(0, pbody->body->GetLinearVelocity().y));
        }

        RenderSprite();
        return true;
    }

    // Set looking direction
    isLookingLeft = dx < 0;

    // Handle punch attack
    if (isAttacking && currentAnimation == &punch) {
        pbody->body->SetLinearVelocity(b2Vec2(0, pbody->body->GetLinearVelocity().y));

        // Update punch attack area position during animation
        if (punchAttackArea != nullptr) {
            int punchX = position.getX() + texW / 2;
            int punchY = position.getY() + texH / 2;
            if (!isLookingLeft) {
                punchX += 30; // Offset punch area to the right
            }
            else {
                punchX -= 30; // Offset punch area to the left
            }
            punchAttackArea->body->SetTransform(b2Vec2(PIXEL_TO_METERS(punchX), PIXEL_TO_METERS(punchY)), 0);
        }

        if (currentAnimation->HasFinished()) {
            isAttacking = false;
            canAttack = false;
            currentAttackCooldown = attackCooldown;
            currentAnimation = &idle;

            // Delete punch attack area when attack finishes
            if (punchAttackArea != nullptr) {
                Engine::GetInstance().physics.get()->DeletePhysBody(punchAttackArea);
                punchAttackArea = nullptr;
            }

            punch.Reset();
        }
    }
    // Combat behavior - simplified logic
    else if (!isAttacking) {
        // Si puede atacar y está muy cerca, atacar
        if (distanceToPlayer <= 3.0f && canAttack) {
            CreatePunchAttack();
        }
        // Si ve al jugador (dentro del rango de detección), perseguir
        else if (distanceToPlayer <= patrolDistance) {
            currentAnimation = &walk;
            float direction = isLookingLeft ? -1.0f : 1.0f;
            pbody->body->SetLinearVelocity(b2Vec2(direction * moveSpeed, pbody->body->GetLinearVelocity().y));
        }
        // Si no ve al jugador, idle
        else {
            currentAnimation = &idle;
            pbody->body->SetLinearVelocity(b2Vec2(0, pbody->body->GetLinearVelocity().y));
        }
    }

    UpdatePosition();
    RenderSprite();
    currentAnimation->Update();

    return true;
}
void Devil::CreatePunchAttack() {
    isAttacking = true;
    currentAnimation = &punch;
    currentAnimation->Reset();

    // Create punch attack area when attack starts
    if (punchAttackArea == nullptr) {
        int punchX = position.getX() + texW / 2;
        int punchY = position.getY() + texH / 2;

        if (!isLookingLeft) {
            punchX += 30; // Offset punch area to the right
        }
        else {
            punchX -= 30; // Offset punch area to the left
        }

        punchAttackArea = Engine::GetInstance().physics.get()->CreateRectangleSensor(
            punchX, punchY,
            texW + 20, texH / 2,
            bodyType::DYNAMIC
        );

        punchAttackArea->listener = this;
        punchAttackArea->ctype = ColliderType::DEVIL_PUNCH_ATTACK1;
        punchAttackArea->body->SetFixedRotation(true);
    }
}

void Devil::HandleDefeatState() {
    currentAnimation->Update();

    RenderSprite();
    Engine::GetInstance().ui->figth2 = false;
}

void Devil::UpdatePosition() {
    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texW / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);
}

void Devil::RenderSprite() {
    SDL_Rect frame = currentAnimation->GetCurrentFrame();
    int offsetX = 0;

    // Adjust sprite direction and offset
    if (isLookingLeft) {
        flip = SDL_FLIP_HORIZONTAL;
        offsetX = (frame.w - texW) / 2;
    }
    else {
        flip = SDL_FLIP_NONE;
        offsetX = -(frame.w - texW) / 2;
    }

    Engine::GetInstance().render.get()->DrawTexture(
        texture,
        (int)position.getX() + offsetX,
        (int)position.getY(),
        &currentAnimation->GetCurrentFrame(),
        1.0f, 0.0, INT_MAX, INT_MAX, flip
    );
}

void Devil::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLAYER:
        // Handle player collision if needed
        break;

    case ColliderType::PLATFORM:
        break;

    case ColliderType::PLAYER_ATTACK:
        
        break;

    case ColliderType::PLAYER_WHIP_ATTACK:
       
        break;
    }
}

void Devil::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLAYER:
        break;
    case ColliderType::PLAYER_WHIP_ATTACK:
    case ColliderType::PLAYER_ATTACK:
        break;
    }
}

void Devil::SetEnabled(bool active) {
    isEnabled = active;
    pbody->body->SetEnabled(active);
    jumpAttackArea->body->SetEnabled(active);
    punchAttackArea->body->SetEnabled(active);
    pbody->body->SetAwake(active);
    jumpAttackArea->body->SetAwake(active);
    punchAttackArea->body->SetAwake(active);
}

bool Devil::CleanUp() {
    if (jumpAttackArea != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(jumpAttackArea);
        jumpAttackArea = nullptr;
    }

    if (punchAttackArea != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(punchAttackArea);
        punchAttackArea = nullptr;
    }

    Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
    return true;
}

void Devil::SetPosition(Vector2D pos) {
    pos.setX(pos.getX() + texW / 2);
    pos.setY(pos.getY() + texH / 2);
    b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
    pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Devil::GetPosition() {
    b2Vec2 bodyPos = pbody->body->GetTransform().p;
    Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
    return pos;
}

void Devil::ResetPath() {
    Vector2D pos = GetPosition();
    Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
    pathfinding->ResetPath(tilePos);
}

void Devil::ResetLives() {
    lives = 10;
    currentAnimation = &idle;
    if (isDying) isDying = false;
    if (isDead) isDead = false;
}

int Devil::GetCurrentFrameId() const {
    if (currentAnimation != nullptr) {
        return currentAnimation->GetCurrentFrameIndex();
    }
    return 0;
}

int Devil::GetTotalFrames() const {
    if (currentAnimation != nullptr) {
        return currentAnimation->totalFrames;
    }
    return 0;
}

void Devil::SetDeathInXML()
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

void Devil::SetAliveInXML()
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

void Devil::SetSavedDeathToDeathInXML()
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

void Devil::SetSavedDeathToAliveInXML()
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

void Devil::ResetPosition() {
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

void Devil::SavePosition(std::string name) {
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
