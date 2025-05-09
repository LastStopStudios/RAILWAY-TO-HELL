#include "Checkpoints.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"

Checkpoints::Checkpoints() : Entity(EntityType::CHECKPOINT)
{
}

Checkpoints::~Checkpoints() {}

bool Checkpoints::Awake() {
	return true;
}

bool Checkpoints::Start() {

	//initilize textures
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();
	enemyID = parameters.attribute("name").as_string();

	checkpointFX = Engine::GetInstance().audio.get()->LoadFx("Assets/SFX/Uso/checkpoint_activated_2.wav");
	Engine::GetInstance().scene->AddToMusic(checkpointFX);

	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	activating.LoadAnimations(parameters.child("animations").child("activating"));
	activated.LoadAnimations(parameters.child("animations").child("activated"));
	currentAnimation = &idle;

	// L08 TODO 4: Add a physics to an item - initialize the physics body
	pbody = Engine::GetInstance().physics.get()->CreateRectangleSensor((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texW - 32, texH, bodyType::STATIC);

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::CHECKPOINT;

	pbody->listener = this;

	// Set the gravity of the body
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	return true;
}

bool Checkpoints::Update(float dt)
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

	// L08 TODO 4: Add a physics to an item - update the position of the object from the physics.  
	if (activating.HasFinished()) currentAnimation = &activated;

	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + 1, (int)position.getY(), &currentAnimation->GetCurrentFrame());
	currentAnimation->Update();

	return true;
}

void Checkpoints::setActivatedToTrue() {
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

bool Checkpoints::CleanUp()
{
	return true;
}

void Checkpoints::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:

		if (!isActivated) {
			Engine::GetInstance().audio.get()->StopAllFx();
			Engine::GetInstance().audio.get()->PlayFx(checkpointFX);
			isActivated = true;
			currentAnimation = &activating;
		}

		break;
	case ColliderType::UNKNOWN:

		break;
	default:
		break;
	}
}

void Checkpoints::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:

		break;
	case ColliderType::UNKNOWN:

		break;
	default:
		break;
	}
}