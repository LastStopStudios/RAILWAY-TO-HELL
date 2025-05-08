#include "Item.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "Physics.h"

Item::Item() : Entity(EntityType::ITEM)
{

}

Item::~Item() {}

bool Item::Awake() {
	return true;
}

bool Item::Start() {

	//initilize textures
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();
	itemType = parameters.attribute("name").as_string();
	ref = parameters.attribute("ref").as_string();
	enemyID = parameters.attribute("name").as_string();


	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	currentAnimation = &idle;
	
	// L08 TODO 4: Add a physics to an item - initialize the physics body
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 2, bodyType::DYNAMIC);

	pbody->listener = this;

	//Assign collider type
	pbody->ctype = ColliderType::ITEM;
	pbody->ID = parameters.attribute("ID").as_string();
	// Set the gravity of the body
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	return true;
}

bool Item::Update(float dt)
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

    if (!IsEnabled()) return true;

    if (pendingDisable) {
        SetEnabled(false);
        pendingDisable = false;
        SetDeathInXML();
    }

	//Add a physics to an item - update the position of the object from the physics.  
	if (pbody == nullptr) {
		LOG("Enemy pbody is null!");
		return false;
	}
	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);
	if (Engine::GetInstance().scene->dibujar == true) {//make the drawing not appear until you enter the gameplay scene 100%.

		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
		currentAnimation->Update();
	}

	return true;
}

void Item::SetDeathInXML()
{
    // Load XML
    pugi::xml_document doc;
    if (!doc.load_file("config.xml")) {
        LOG("Error loading config.xml");
        return;
    }

    pugi::xml_node itemNode;
    int currentScene = Engine::GetInstance().sceneLoader.get()->GetCurrentLevel();

	if (currentScene == 1) {
		itemNode = doc.child("config")
			.child("scene")
			.child("entities")
			.child("items")
			.find_child_by_attribute("item", "name", enemyID.c_str());
	}
	else if (currentScene == 2) {
		itemNode = doc.child("config")
			.child("scene2")
			.child("entities")
			.child("items")
			.find_child_by_attribute("item", "name", enemyID.c_str());
	}
	else if (currentScene == 3) {
		itemNode = doc.child("config")
			.child("scene3")
			.child("entities")
			.child("items")
			.find_child_by_attribute("item", "name", enemyID.c_str());
	}

    if (!itemNode) {
        LOG("Could not find the node for item in the XML");
        return;
    }

    itemNode.attribute("death").set_value(1); // 1 item is picked

    if (!doc.save_file("config.xml")) {
        LOG("Error saving config.xml");
    }
    else {
        LOG("death status updated in the XML for item");
    }
    DeathValue = 1;
}

void Item::SetAliveInXML()
{
    // Load XML file
    pugi::xml_document doc;
    if (!doc.load_file("config.xml")) {
        LOG("Error loading config.xml");
        return;
    }

	pugi::xml_node itemNode;
	int currentScene = Engine::GetInstance().sceneLoader.get()->GetCurrentLevel();

	if (currentScene == 1) {
		itemNode = doc.child("config")
			.child("scene")
			.child("entities")
			.child("items")
			.find_child_by_attribute("item", "name", enemyID.c_str());
	}
	else if (currentScene == 2) {
		itemNode = doc.child("config")
			.child("scene2")
			.child("entities")
			.child("items")
			.find_child_by_attribute("item", "name", enemyID.c_str());
	}
	else if (currentScene == 3) {
		itemNode = doc.child("config")
			.child("scene3")
			.child("entities")
			.child("items")
			.find_child_by_attribute("item", "name", enemyID.c_str());
	}

	if (!itemNode) {
		LOG("Could not find the node for item in the XML");
		return;
	}

    itemNode.attribute("death").set_value(0); // 0 items is not picked

    if (!doc.save_file("config.xml")) {
        LOG("Error saving config.xml");
    }
    else {
        LOG("death status updated in the XML for item");
    }
    DeathValue = 0;
}

void Item::SetSavedDeathToDeathInXML()
{
    // Load XML
    pugi::xml_document doc;
    if (!doc.load_file("config.xml")) {
        LOG("Error loading config.xml");
        return;
    }

	pugi::xml_node itemNode;
	int currentScene = Engine::GetInstance().sceneLoader.get()->GetCurrentLevel();

	if (currentScene == 1) {
		itemNode = doc.child("config")
			.child("scene")
			.child("entities")
			.child("items")
			.find_child_by_attribute("item", "name", enemyID.c_str());
	}
	else if (currentScene == 2) {
		itemNode = doc.child("config")
			.child("scene2")
			.child("entities")
			.child("items")
			.find_child_by_attribute("item", "name", enemyID.c_str());
	}
	else if (currentScene == 3) {
		itemNode = doc.child("config")
			.child("scene3")
			.child("entities")
			.child("items")
			.find_child_by_attribute("item", "name", enemyID.c_str());
	}

	if (!itemNode) {
		LOG("Could not find the node for item in the XML");
		return;
	}

    itemNode.attribute("savedDeath").set_value(1); // 1 item is picked

    if (!doc.save_file("config.xml")) {
        LOG("Error saving config.xml");
    }
    else {
        LOG("death status updated in the XML for item");
    }

    SavedDeathValue = 1;
}

void Item::SetSavedDeathToAliveInXML()
{
    // Load XML file
    pugi::xml_document doc;
    if (!doc.load_file("config.xml")) {
        LOG("Error loading config.xml");
        return;
    }

	pugi::xml_node itemNode;
	int currentScene = Engine::GetInstance().sceneLoader.get()->GetCurrentLevel();

	if (currentScene == 1) {
		itemNode = doc.child("config")
			.child("scene")
			.child("entities")
			.child("items")
			.find_child_by_attribute("item", "name", enemyID.c_str());
	}
	else if (currentScene == 2) {
		itemNode = doc.child("config")
			.child("scene2")
			.child("entities")
			.child("items")
			.find_child_by_attribute("item", "name", enemyID.c_str());
	}
	else if (currentScene == 3) {
		itemNode = doc.child("config")
			.child("scene3")
			.child("entities")
			.child("items")
			.find_child_by_attribute("item", "name", enemyID.c_str());
	}

	if (!itemNode) {
		LOG("Could not find the node for item in the XML");
		return;
	}

    itemNode.attribute("savedDeath").set_value(0); // 0 item is not picked

    if (!doc.save_file("config.xml")) {
        LOG("Error saving config.xml");
    }
    else {
        LOG("death status updated in the XML for item");
    }

    SavedDeathValue = 0;
}

void Item::SetCreatedTrueInXML() {
	// Load XML file
	pugi::xml_document doc;
	if (!doc.load_file("config.xml")) {
		LOG("Error loading config.xml");
		return;
	}

	pugi::xml_node itemNode;
	int currentScene = Engine::GetInstance().sceneLoader.get()->GetCurrentLevel();

	if (currentScene == 1) {
		itemNode = doc.child("config")
			.child("scene")
			.child("entities")
			.child("items")
			.find_child_by_attribute("item", "name", enemyID.c_str());
	}
	else if (currentScene == 2) {
		itemNode = doc.child("config")
			.child("scene2")
			.child("entities")
			.child("items")
			.find_child_by_attribute("item", "name", enemyID.c_str());
	}
	else if (currentScene == 3) {
		itemNode = doc.child("config")
			.child("scene3")
			.child("entities")
			.find_child_by_attribute("item", "name", enemyID.c_str());
	}

	if (!itemNode) {
		LOG("Could not find the node for item in the XML");
		return;
	}

	itemNode.attribute("created").set_value(true); // true item is created

	if (!doc.save_file("config.xml")) {
		LOG("Error saving config.xml");
	}
	else {
		LOG("death status updated in the XML for item");
	}

}

void Item::SetCreatedFalseInXML() {
	// Load XML file
	pugi::xml_document doc;
	if (!doc.load_file("config.xml")) {
		LOG("Error loading config.xml");
		return;
	}

	pugi::xml_node itemNode;
	int currentScene = Engine::GetInstance().sceneLoader.get()->GetCurrentLevel();

	if (currentScene == 1) {
		itemNode = doc.child("config")
			.child("scene")
			.child("entities")
			.child("items")
			.find_child_by_attribute("item", "name", enemyID.c_str());
	}
	else if (currentScene == 2) {
		itemNode = doc.child("config")
			.child("scene2")
			.child("entities")
			.child("items")
			.find_child_by_attribute("item", "name", enemyID.c_str());
	}
	else if (currentScene == 3) {
		itemNode = doc.child("config")
			.child("scene3")
			.child("entities")
			.find_child_by_attribute("item", "name", enemyID.c_str());
	}

	if (!itemNode) {
		LOG("Could not find the node for item in the XML");
		return;
	}

	itemNode.attribute("created").set_value(false); // false item is not created

	if (!doc.save_file("config.xml")) {
		LOG("Error saving config.xml");
	}
	else {
		LOG("death status updated in the XML for item");
	}
}

void Item::SetEnabled(bool active) {
    isEnabled = active;
    pbody->body->SetEnabled(active);
    pbody->body->SetAwake(active);
}

void Item::OnCollision(PhysBody* physA, PhysBody* physB) {
	if (physA->ctype == ColliderType::PLAYER_ATTACK && physB->ctype == ColliderType::ENEMY) {
		// Additional enemy hit logic can go here
		return;
	}
	switch (physB->ctype) {
	case ColliderType::PLATFORM:
		break;
	case ColliderType::PLAYER: {
		if (GetItemType() == "Dash ability") {
			pendingDisable = true;
			//Engine::GetInstance().entityManager.get()->DestroyEntity(this);
		}
		if (GetItemType() == "Double jump") {
            pendingDisable = true;
			//Engine::GetInstance().entityManager.get()->DestroyEntity(this);
		}
		if (GetItemType() == "Whip") {
            pendingDisable = true;
			//Engine::GetInstance().entityManager.get()->DestroyEntity(this);
		}
		if (GetItemType() == "Door key") {
            pendingDisable = true;
			//Engine::GetInstance().entityManager.get()->DestroyEntity(this);
		}
		if (GetItemType() == "Remember1") {
            pendingDisable = true;
			//Engine::GetInstance().entityManager.get()->DestroyEntity(this);
		}
		if (GetItemType() == "Ball") {
            pendingDisable = true;
			//Engine::GetInstance().entityManager.get()->DestroyEntity(this);
		}
		break;
	}
	case ColliderType::UNKNOWN:
		break;
	}
}

void Item::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {

}

void Item::SetPosition(Vector2D pos) {
	pos.setX(pos.getX() + texW / 2);
	pos.setY(pos.getY() + texH / 2);
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Item::GetPosition() {
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
	return pos;
}

void Item::SavePosition(std::string name) {

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

	//Save info to XML 

	//items
	pugi::xml_node itemsNode = sceneNode.child("entities").child("items");
	if (!Engine::GetInstance().scene.get()->itemList.empty()) {
		int i = 0;
		for (pugi::xml_node itemNode : itemsNode.children("item")) {
			if (i < Engine::GetInstance().scene.get()->itemList.size()) {
				std::string enemyID = itemNode.attribute("name").as_string();
				if (enemyID == name) {
					itemNode.attribute("x").set_value(Engine::GetInstance().scene.get()->itemList[i]->GetPosition().getX());
					itemNode.attribute("y").set_value(Engine::GetInstance().scene.get()->itemList[i]->GetPosition().getY());
				}
				i++;
			}

		}
	}

	//Saves the modifications to the XML 
	loadFile.save_file("config.xml");
}

bool Item::CleanUp()
{


	Engine::GetInstance().physics->DeletePhysBody(pbody);

	return true;
}