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

Checkpoints::~Checkpoints() {
	CleanUp();
}

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
	activatedXML = parameters.attribute("activated").as_bool();

	checkpointFX = Engine::GetInstance().audio.get()->LoadFx("Assets/SFX/Uso/checkpoint_activated_2.wav");
	Engine::GetInstance().scene->AddToMusic(checkpointFX);

	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	activated.LoadAnimations(parameters.child("animations").child("activated"));
	if (!activatedXML) {
		currentAnimation = &idle;
	}

	// L08 TODO 4: Add a physics to an item - initialize the physics body
	pbody = Engine::GetInstance().physics.get()->CreateRectangleSensor((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texW, texH, bodyType::STATIC);

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


	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + 1, (int)position.getY(), &currentAnimation->GetCurrentFrame());
	currentAnimation->Update();

	return true;
}

void Checkpoints::setActivatedToTrue(int scene) {
	// Load XML file
	pugi::xml_document doc;
	if (!doc.load_file("config.xml")) {
		LOG("Error loading config.xml");
		return;
	}

	pugi::xml_node checkpointNode;

	if (scene == 1) {
		checkpointNode = doc.child("config")
			.child("scene")
			.child("entities")
			.child("checkpoints")
			.find_child_by_attribute("checkpoint", "name", enemyID.c_str());
	}
	else if (scene == 2) {
		checkpointNode = doc.child("config")
			.child("scene2")
			.child("entities")
			.child("checkpoints")
			.find_child_by_attribute("checkpoint", "name", enemyID.c_str());
	}
	else if (scene == 3) {
		checkpointNode = doc.child("config")
			.child("scene3")
			.child("checkpoints")
			.find_child_by_attribute("checkpoint", "name", enemyID.c_str());
	}

	if (!checkpointNode) {
		LOG("Could not find the node for checkpoint in the XML");
		return;
	}

	checkpointNode.attribute("activated").set_value(true); // set to true

	if (!doc.save_file("config.xml")) {
		LOG("Error saving config.xml");
	}
	else {
		LOG("death status updated in the XML for checkpoint");
	}

}

void Checkpoints::setToActivatedAnim() {
	activatedXML = parameters.attribute("activated").as_bool();

	if (activatedXML) {
		isActivated = true;
		currentAnimation = &activated;
	}
}

bool Checkpoints::CleanUp()
{
	if (texture != nullptr) {
		Engine::GetInstance().textures.get()->UnLoad(texture);
		texture = nullptr;
	}

	if (pbody != nullptr) {
		Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
		pbody = nullptr;
	}
	return true;
}

void Checkpoints::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:

		if (!isActivated && Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_E) == KEY_DOWN) {
			Engine::GetInstance().scene.get()->SaveState();
			int currentscene = Engine::GetInstance().sceneLoader.get()->GetCurrentLevel();
			setActivatedToTrue(currentscene);
			Engine::GetInstance().audio.get()->StopAllFx();
			Engine::GetInstance().audio.get()->PlayFx(checkpointFX);
			isActivated = true;
			currentAnimation = &activated;
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