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
	sceneForThisCheckpoint = parameters.attribute("scene").as_int();

	checkpointFX = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/checkpoint.ogg");

	//Engine::GetInstance().scene->AddToMusic(checkpointFX);
	Engine::GetInstance().audio.get()->SetFxVolume(checkpointFX, 4);

	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	activated.LoadAnimations(parameters.child("animations").child("activated"));
	if (!activatedXML) {
		currentAnimation = &idle;
	}

    // Add a physics to an item - initialize the physics body
	pbody = Engine::GetInstance().physics.get()->CreateRectangleSensor((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texW, texH, bodyType::STATIC);

	// Assign collider type
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

	setToIdleAnim();
	
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
	else if (scene == 4) {
		checkpointNode = doc.child("config")
			.child("scene4")
			.child("entities")
			.child("checkpoints")
			.find_child_by_attribute("checkpoint", "name", enemyID.c_str());
	}
	else if (scene == 5) {
		checkpointNode = doc.child("config")
			.child("scene5")
			.child("checkpoints")
			.find_child_by_attribute("checkpoint", "name", enemyID.c_str());
	}
	else if (scene == 6) {
		checkpointNode = doc.child("config")
			.child("scene6")
			.child("entities")
			.child("checkpoints")
			.find_child_by_attribute("checkpoint", "name", enemyID.c_str());
	}
	else if (scene == 7) {
		checkpointNode = doc.child("config")
			.child("scene7")
			.child("checkpoints")
			.find_child_by_attribute("checkpoint", "name", enemyID.c_str());
	}
	else if (scene == 8) {
		checkpointNode = doc.child("config")
			.child("scene8")
			.child("entities")
			.child("checkpoints")
			.find_child_by_attribute("checkpoint", "name", enemyID.c_str());
	}
	else if (scene == 9) {
		checkpointNode = doc.child("config")
			.child("scene9")
			.child("checkpoints")
			.find_child_by_attribute("checkpoint", "name", enemyID.c_str());
	}
	else if (scene == 10) {
		checkpointNode = doc.child("config")
			.child("scene10")
			.child("entities")
			.child("checkpoints")
			.find_child_by_attribute("checkpoint", "name", enemyID.c_str());
	}
	else if (scene == 11) {
		checkpointNode = doc.child("config")
			.child("scene11")
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

void Checkpoints::setActivatedToFalse() {
	// Load XML file
	pugi::xml_document doc;
	if (!doc.load_file("config.xml")) {
		LOG("Error loading config.xml");
		return;
	}

	pugi::xml_node checkpointNode;
	if (sceneForThisCheckpoint == 1) {
		checkpointNode = doc.child("config")
			.child("scene")
			.child("entities")
			.child("checkpoints")
			.find_child_by_attribute("checkpoint", "name", enemyID.c_str());
	}
	else if (sceneForThisCheckpoint == 2) {
		checkpointNode = doc.child("config")
			.child("scene2")
			.child("entities")
			.child("checkpoints")
			.find_child_by_attribute("checkpoint", "name", enemyID.c_str());
	}
	else if (sceneForThisCheckpoint == 3) {
		checkpointNode = doc.child("config")
			.child("scene3")
			.child("checkpoints")
			.find_child_by_attribute("checkpoint", "name", enemyID.c_str());
	}
	else if (sceneForThisCheckpoint == 4) {
		checkpointNode = doc.child("config")
			.child("scene4")
			.child("entities")
			.child("checkpoints")
			.find_child_by_attribute("checkpoint", "name", enemyID.c_str());
	}
	else if (sceneForThisCheckpoint == 5) {
		checkpointNode = doc.child("config")
			.child("scene5")
			.child("checkpoints")
			.find_child_by_attribute("checkpoint", "name", enemyID.c_str());
	}
	else if (sceneForThisCheckpoint == 6) {
		checkpointNode = doc.child("config")
			.child("scene6")
			.child("entities")
			.child("checkpoints")
			.find_child_by_attribute("checkpoint", "name", enemyID.c_str());
	}
	else if (sceneForThisCheckpoint == 7) {
		checkpointNode = doc.child("config")
			.child("scene7")
			.child("checkpoints")
			.find_child_by_attribute("checkpoint", "name", enemyID.c_str());
	}
	else if (sceneForThisCheckpoint == 8) {
		checkpointNode = doc.child("config")
			.child("scene8")
			.child("entities")
			.child("checkpoints")
			.find_child_by_attribute("checkpoint", "name", enemyID.c_str());
	}
	else if (sceneForThisCheckpoint == 9) {
		checkpointNode = doc.child("config")
			.child("scene9")
			.child("checkpoints")
			.find_child_by_attribute("checkpoint", "name", enemyID.c_str());
	}
	else if (sceneForThisCheckpoint == 10) {
		checkpointNode = doc.child("config")
			.child("scene10")
			.child("entities")
			.child("checkpoints")
			.find_child_by_attribute("checkpoint", "name", enemyID.c_str());
	}
	else if (sceneForThisCheckpoint == 11) {
		checkpointNode = doc.child("config")
			.child("scene11")
			.child("checkpoints")
			.find_child_by_attribute("checkpoint", "name", enemyID.c_str());
	}

	if (!checkpointNode) {
		LOG("Could not find the node for checkpoint in the XML");
		return;
	}

	checkpointNode.attribute("activated").set_value(false); // set to false

	doc.save_file("config.xml");

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

void Checkpoints::setToIdleAnim() {

	if (pendingToChangeAnim) {
		pendingToChangeAnim = false;
		isActivated = false;
		currentAnimation = &idle;
		setActivatedToFalse();
	}

}

void Checkpoints::ResetOthersCheckpoints() {

	pugi::xml_document loadFile;

	pugi::xml_parse_result result = loadFile.load_file("config.xml");

	if (result == NULL)
	{
		LOG("Could not load file. Pugi error: %s", result.description());
		return;
	}

	pugi::xml_node sceneNode;
	
	int maxScenes = 12;

	for (int i = 0; i < maxScenes; ++i) {
		if (i == 0) sceneNode = loadFile.child("config").child("scene");
		else if (i == 1) sceneNode = loadFile.child("config").child("scene2");
		else if (i == 2) sceneNode = loadFile.child("config").child("scene3");
		else if (i == 3) sceneNode = loadFile.child("config").child("scene4");
		else if (i == 4) sceneNode = loadFile.child("config").child("scene5");
		else if (i == 5) sceneNode = loadFile.child("config").child("scene6");
		else if (i == 6) sceneNode = loadFile.child("config").child("scene7");
		else if (i == 7) sceneNode = loadFile.child("config").child("scene8");
		else if (i == 8) sceneNode = loadFile.child("config").child("scene9");
		else if (i == 9) sceneNode = loadFile.child("config").child("scene10");
		else if (i == 10) sceneNode = loadFile.child("config").child("scene11");
		//checkpoints
		pugi::xml_node checkpointsNode = sceneNode.child("entities").child("checkpoints");

		for (pugi::xml_node checkpointNode : checkpointsNode.children("checkpoint")) {
			for (int i = 0; i < Engine::GetInstance().scene.get()->checkpointList.size(); ++i) {

				if (enemyID != Engine::GetInstance().scene.get()->checkpointList[i]->GetCheckpointType()) {
					Engine::GetInstance().scene.get()->checkpointList[i]->pendingToChangeAnim = true;
				}
			}
		}
	}
}



bool Checkpoints::CleanUp()
{
	//if (texture != nullptr) {
	//	SDL_DestroyTexture(texture);
	//	texture = nullptr;
	//}

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

		setToIdleAnim();

		if (!isActivated && Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_E) == KEY_DOWN) {
			ResetOthersCheckpoints();
			Engine::GetInstance().scene.get()->SaveState();
			int currentscene = Engine::GetInstance().sceneLoader.get()->GetCurrentLevel();
			setActivatedToTrue(currentscene);
			Engine::GetInstance().audio.get()->StopAllFx();
			Engine::GetInstance().audio.get()->PlayFx(checkpointFX);
			isActivated = true;
			currentAnimation = &activated;
		}

		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_E) == KEY_DOWN) {
			Engine::GetInstance().scene.get()->GetPlayer()->ResetLives();
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