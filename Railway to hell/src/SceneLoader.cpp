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

SceneLoader::SceneLoader() {
    currentScene = 1;
}

SceneLoader::~SceneLoader() {}

void SceneLoader::LoadScene(int level, int x, int y,bool fade,bool bosscam) {

    if(fade== true)
    { 
        FadeIn(1.0f);// Animation speed (FadeIn)
    }

    if (bosscam == true) {
        Engine::GetInstance().scene->EntrarBoss();
    }
    else if (bosscam == false) {
        Engine::GetInstance().scene->SalirBoss();
    }
       
    UnLoadEnemiesItems();
    SetCurrentScene(level);

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
        Engine::GetInstance().scene->GetPlayer()->SetPosition(
            Vector2D(x, y));
    }
    LoadEnemiesItems(sceneNode);

   // Engine::GetInstance().sceneLoader->FadeOut(1.0f);    // Animation speed(FadeOut)


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
    }

    pugi::xml_node itemsNode = sceneNode.child("entities").child("items");
    if (itemsNode) {
        for (pugi::xml_node itemNode = itemsNode.child("item"); itemNode; itemNode = itemNode.next_sibling("item"))
        {
            Item* item = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
            item->SetParameters(itemNode);
            itemsList.push_back(item);  // Now using the member variable
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
    // Initialize items
    for (auto item : itemsList) {
        item->Start();
    } 
}

void SceneLoader::UnLoadEnemiesItems() {
    // Get the entity manager
    auto entityManager = Engine::GetInstance().entityManager.get();

    // Make a copy of entities to safely iterate
    std::vector<Entity*> entitiesToRemove;

    // Find all enemies and items (skip the player)
    for (auto entity : entityManager->entities) {
        if (entity->type == EntityType::TERRESTRE || entity->type == EntityType::ITEM || entity->type == EntityType::VOLADOR || entity->type == EntityType::BOSS) {
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
    itemsList.clear();  
}
void SceneLoader::SetCurrentScene(int level)
{
	currentScene = level;
}

// Fill the screen with a black color that gradually becomes more opaque
void SceneLoader::FadeIn(float speed) {
    float alpha = 0.0f;
    SDL_SetRenderDrawBlendMode(Engine::GetInstance().render->renderer, SDL_BLENDMODE_BLEND);

    while (alpha < 90.0f) {
        SDL_SetRenderDrawColor(Engine::GetInstance().render->renderer, 0, 0, 0, static_cast<Uint8>(alpha));
        SDL_RenderFillRect(Engine::GetInstance().render->renderer, NULL);
        SDL_RenderPresent(Engine::GetInstance().render->renderer);

        alpha += speed;  //  Increase opacity based on speed
        SDL_Delay(10);   
    }
    
    // Check if the background is completely black
    SDL_SetRenderDrawColor(Engine::GetInstance().render->renderer, 0, 0, 0, 90);
    SDL_RenderFillRect(Engine::GetInstance().render->renderer, NULL);
    SDL_RenderPresent(Engine::GetInstance().render->renderer);
}

//Makes the black screen fade out
void SceneLoader::FadeOut(float speed) {
    float alpha = 90.0;
    SDL_SetRenderDrawBlendMode(Engine::GetInstance().render->renderer, SDL_BLENDMODE_BLEND);

    
    while (alpha > 0.0f) {
        SDL_SetRenderDrawColor(Engine::GetInstance().render->renderer, 0, 0, 0, static_cast<Uint8>(alpha));
        SDL_RenderFillRect(Engine::GetInstance().render->renderer, NULL);
        SDL_RenderPresent(Engine::GetInstance().render->renderer);

        alpha -= speed;  

        SDL_Delay(10);   
    }

    SDL_SetRenderDrawColor(Engine::GetInstance().render->renderer, 0, 0, 0, 50);
    SDL_RenderFillRect(Engine::GetInstance().render->renderer, NULL);
    SDL_RenderPresent(Engine::GetInstance().render->renderer);
}
