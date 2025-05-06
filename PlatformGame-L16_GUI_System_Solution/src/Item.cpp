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
	name = "item";
}

Item::~Item() {}

bool Item::Awake() {
	return true;
}

bool Item::Start() {

	//initilize textures
	enemyID = parameters.attribute("name").as_string();
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();
	DeathValue = parameters.attribute("death").as_int();
	type = parameters.attribute("type").as_int();

	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	currentAnimation = &idle;
	
	// L08 TODO 4: Add a physics to an item - initialize the physics body
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 2, bodyType::DYNAMIC);

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::ITEM;

	// Set the gravity of the body
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	return true;
}

bool Item::Update(float dt)
{
	if (!IsEnabled()) return true;
	if (pendingDisable) {
		SetEnabled(false);
		pendingDisable = false;
		SetDeathInXML();
	}
	// L08 TODO 4: Add a physics to an item - update the position of the object from the physics.  
	if (pbody == nullptr) {
		LOG("Enemy pbody is null!");
		return false;
	}
	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
	currentAnimation->Update();

	return true;
}

bool Item::CleanUp()
{
	// Explicitly destroy the physics body if it exists
	//if (pbody != nullptr)
	//{
	//	// Make sure to tell the physics system to destroy this body
	//	Engine::GetInstance().physics->DeletePhysBody(pbody);
	//	pbody = nullptr;
	//}
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
		LOG("Could not find the node for enemy in the XML");
		return;
	}

	itemNode.attribute("death").set_value(1); // 1 enemy is death

	if (!doc.save_file("config.xml")) {
		LOG("Error saving config.xml");
	}
	else {
		LOG("death status updated in the XML for enemy");
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
		LOG("Could not find the node for enemy in the XML");
		return;
	}

	itemNode.attribute("death").set_value(0); // 0 enemy is alive

	if (!doc.save_file("config.xml")) {
		LOG("Error saving config.xml");
	}
	else {
		LOG("death status updated in the XML for");
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
			.find_child_by_attribute("items", "name", enemyID.c_str());
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
		LOG("Could not find the node for enemy in the XML");
		return;
	}

	itemNode.attribute("savedDeath").set_value(1); // 1 enemy is death

	if (!doc.save_file("config.xml")) {
		LOG("Error saving config.xml");
	}
	else {
		LOG("death status updated in the XML for enemy");
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
		LOG("Could not find the node for enemy in the XML");
		return;
	}

	itemNode.attribute("savedDeath").set_value(0); // 0 enemy is alive

	if (!doc.save_file("config.xml")) {
		LOG("Error saving config.xml");
	}
	else {
		LOG("death status updated in the XML for");
	}

	SavedDeathValue = 0;
}

void Item::SetEnabled(bool active) {
	isEnabled = active;
	pbody->body->SetEnabled(active);
	pbody->body->SetAwake(active);
}

void Item::SetPosition(Vector2D pos) {
	pos.setX(pos.getX() + texH / 2);
	pos.setY(pos.getY() + texH / 2);
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos, 0);
}

void Item::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		LOG("Collided with player - DESTROY");
		pendingDisable = true;
		break;
	}
}

Vector2D Item::GetPosition() {
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
	pos.setX(pos.getX() - texH / 2);
	pos.setY(pos.getY() - texH / 2);
	return pos;
}

void Item::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		LOG("Collision player");
		break;
	}
}