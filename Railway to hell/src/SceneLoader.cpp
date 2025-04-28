#include "SceneLoader.h"
#include "Engine.h"
#include "Map.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Player.h" 
#include "Scene.h"  
#include "EntityManager.h"
#include "Terrestre.h"
#include "Pathfinding.h"
#include "Item.h"
#include "Volador.h"
#include "Boss.h"
#include "Doors.h"
#include "Levers.h"
#include "Elevators.h"
#include "Projectiles.h"
#include "Log.h"

SceneLoader::SceneLoader() {
    currentScene = 1;
}

SceneLoader::~SceneLoader() {}

void SceneLoader::LoadScene(int level, int x, int y,bool fade,bool bosscam) {

    if(fade== true)
    { 
        FadeIn(1.0f);// Animation speed (FadeIn)
    }
    //Switch between fixed camera for boss fight to camera following the player
    if (bosscam == true) {//boss fight
        Engine::GetInstance().scene->EntrarBoss();//Boss cam
        Engine::GetInstance().scene->BloquearSensor();//Block scene change sensors to prevent the player from escaping
    }
    else if (bosscam == false) {//camera follows player
        Engine::GetInstance().scene->SalirBoss();//Camera on player
        /*Line to use to unlock scene change sensors*/
        Engine::GetInstance().scene->DesbloquearSensor();//unlocks sensors scene change
    }
      
    /*if (fade == false) {
        
    }*/
    UnLoadEnemiesItems();
    SetCurrentScene(level);
    DrawScene(level, x, y);
	//if (fade == true) {
	//	FadeOut(1.0f, true, level, x, y); // Animation speed (FadeOut)
	//}

}

void SceneLoader::DrawScene(int level, int x, int y) {
    pugi::xml_document loadFile;
    if (!loadFile.load_file("config.xml")) {
        return;
    }

    pugi::xml_node configNode = loadFile.child("config");
    if (!configNode) {
        return;
    }

    Engine::GetInstance().map->CleanUp();

    std::string sceneName = (level == 1) ? "scene" : "scene" + std::to_string(level);
    pugi::xml_node sceneNode = configNode.child(sceneName.c_str());
    if (!sceneNode) {
        return;
    }

    pugi::xml_node mapNode = sceneNode.child("map");
    if (mapNode) {
        Engine::GetInstance().map->Load(mapNode.attribute("path").as_string(),
            mapNode.attribute("name").as_string());
    }

    pugi::xml_node playerNode = sceneNode.child("entities").child("player");
    if (playerNode) {
        Player* player = Engine::GetInstance().scene->GetPlayer();
        player->SetPosition(Vector2D(x, y));
        player->ResetDoorAndLeverStates();
    }
    LoadEnemiesItems(sceneNode);
}

void SceneLoader::LoadEnemiesItems(pugi::xml_node sceneNode) {

    pugi::xml_node enemiesNode = sceneNode.child("entities").child("enemies");
    if (!enemiesNode) {
        return;
    }

    for (pugi::xml_node enemyNode = enemiesNode.child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy")) {
        std::string type = enemyNode.attribute("type").as_string();

        if (type == "rastrero") {
            Terrestre* enemy = (Terrestre*)Engine::GetInstance().entityManager->CreateEntity(EntityType::TERRESTRE);
            enemy->SetParameters(enemyNode);
            Engine::GetInstance().scene->GetEnemyList().push_back(enemy); 
        }

        if (type == "volador") {
            Volador* volador = (Volador*)Engine::GetInstance().entityManager->CreateEntity(EntityType::VOLADOR);
            volador->SetParameters(enemyNode);
            Engine::GetInstance().scene->GetVoladorList().push_back(volador); 
        }

		if (type == "boss") {
			Boss* boss = (Boss*)Engine::GetInstance().entityManager->CreateEntity(EntityType::BOSS);
			boss->SetParameters(enemyNode);
			Engine::GetInstance().scene->GetBossList().push_back(boss);
		}
        
		if (type == "guardian") {
			Caronte* caronte = (Caronte*)Engine::GetInstance().entityManager->CreateEntity(EntityType::CARONTE);
			caronte->SetParameters(enemyNode);
			Engine::GetInstance().scene->GetCaronteList().push_back(caronte);
		}
    }

    pugi::xml_node itemsNode = sceneNode.child("entities").child("items");
    if (itemsNode) {
        for (pugi::xml_node itemNode = itemsNode.child("item"); itemNode; itemNode = itemNode.next_sibling("item"))
        {
            Item* item = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
            item->SetParameters(itemNode);
			Engine::GetInstance().scene->GetItemList().push_back(item);
        }
    }

    pugi::xml_node doorsNode = sceneNode.child("entities").child("doors");
    if (doorsNode) {
        for (pugi::xml_node doorNode = doorsNode.child("door"); doorNode; doorNode = doorNode.next_sibling("door"))
        {
            Doors* door = (Doors*)Engine::GetInstance().entityManager->CreateEntity(EntityType::DOORS);
            door->SetParameters(doorNode);
            Engine::GetInstance().scene->GetDoorsList().push_back(door);
        }
    }

	pugi::xml_node leversNode = sceneNode.child("entities").child("levers");
    if (leversNode) {
        for (pugi::xml_node leverNode = leversNode.child("lever"); leverNode; leverNode = leverNode.next_sibling("lever"))
        {
            Levers* lever = (Levers*)Engine::GetInstance().entityManager->CreateEntity(EntityType::LEVER);
            lever->SetParameters(leverNode);
            Engine::GetInstance().scene->GetLeversList().push_back(lever);
        }
    }

    pugi::xml_node elevatorsNode = sceneNode.child("entities").child("elevators");
    if (elevatorsNode) {
        for (pugi::xml_node elevatorNode = elevatorsNode.child("elevator"); elevatorNode; elevatorNode = elevatorNode.next_sibling("elevator"))
        {
            Elevators* elevator = (Elevators*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ELEVATORS);
            elevator->SetParameters(elevatorNode);
            Engine::GetInstance().scene->GetElevatorsList().push_back(elevator);
        }
    }



    // Initialize enemies
    for (auto enemy : Engine::GetInstance().scene->GetEnemyList()) {
        enemy->Start();
    }
    for (auto enemy : Engine::GetInstance().scene->GetVoladorList()) { 
        enemy->Start();
    }
    for (auto enemy : Engine::GetInstance().scene->GetBossList()) {
        enemy->Start();
    }
	for (auto enemy : Engine::GetInstance().scene->GetCaronteList()) {
		enemy->Start();
	}
    // Initialize items
    for (auto item : Engine::GetInstance().scene->GetItemList()) {
        item->Start();
    }
    // Initialize Doors
	for (auto door : Engine::GetInstance().scene->GetDoorsList()) {
		door->Start();
	}
    // Initialize levers
	for (auto lever : Engine::GetInstance().scene->GetLeversList()) {
		lever->Start();
	}
    // Initialize elevators
    for (auto elevator : Engine::GetInstance().scene->GetElevatorsList()) {
        elevator->Start();
    }

}

void SceneLoader::UnLoadEnemiesItems() {
    // Get the entity manager
    auto entityManager = Engine::GetInstance().entityManager.get();

    // Make a copy of entities to safely iterate
    std::vector<Entity*> entitiesToRemove;

    // Find all enemies and items (skip the player)
    for (auto entity : entityManager->entities) {
        if (entity->type == EntityType::TERRESTRE || entity->type == EntityType::ITEM || entity->type == EntityType::VOLADOR || entity->type == EntityType::BOSS || entity->type == EntityType::CARONTE || entity->type == EntityType::DOORS || entity->type == EntityType::LEVER || entity->type == EntityType::ELEVATORS) {
            entitiesToRemove.push_back(entity);
        }
    }

    // Destroy each entity separately
    for (auto entity : entitiesToRemove) {
        entityManager->DestroyEntity(entity);
    }

    // Clear your local tracking list
    Engine::GetInstance().scene->GetEnemyList().clear(); 
    Engine::GetInstance().scene->GetVoladorList().clear(); 
    Engine::GetInstance().scene->GetBossList().clear();
	Engine::GetInstance().scene->GetCaronteList().clear();
	Engine::GetInstance().scene->GetDoorsList().clear();
	Engine::GetInstance().scene->GetLeversList().clear();
    Engine::GetInstance().scene->GetElevatorsList().clear();
	Engine::GetInstance().scene->GetItemList().clear();
}
void SceneLoader::SetCurrentScene(int level)
{
	currentScene = level;
}

// Fill the screen with a black color that gradually becomes more opaque
void SceneLoader::FadeIn(float speed) {
    float alpha = 0.0f;
    SDL_SetRenderDrawBlendMode(Engine::GetInstance().render->renderer, SDL_BLENDMODE_BLEND);

    while (alpha < 255.0f) {

		// Fill the screen with black color
        SDL_SetRenderDrawColor(Engine::GetInstance().render->renderer, 0, 0, 0, static_cast<Uint8>(alpha));
        SDL_RenderFillRect(Engine::GetInstance().render->renderer, NULL);

        // Update screen
        SDL_RenderPresent(Engine::GetInstance().render->renderer);

        alpha += speed;  //  Increase opacity based on speed
        SDL_Delay(10);   
    }
}

//Makes the black screen fade out
void SceneLoader::FadeOut(float speed, bool loadscene, int level, int x, int y) {
    float alpha = 255.0;
    SDL_SetRenderDrawBlendMode(Engine::GetInstance().render->renderer, SDL_BLENDMODE_BLEND);

    if (loadscene) {
       SetCurrentScene(level);

    }
    
    while (alpha > 0.0f) {
        if (loadscene) {
            SDL_RenderClear(Engine::GetInstance().render->renderer);
            UnLoadEnemiesItems();
            DrawScene(level, x, y);
        }
		else {
			Engine::GetInstance().scene->DrawCurrentScene();
		}
        
        SDL_SetRenderDrawColor(Engine::GetInstance().render->renderer, 0, 0, 0 , static_cast<Uint8>(alpha));
        SDL_RenderFillRect(Engine::GetInstance().render->renderer, NULL);

		// Update screen
        SDL_RenderPresent(Engine::GetInstance().render->renderer);

        alpha -= speed;  

        SDL_Delay(10);   
    }
}
