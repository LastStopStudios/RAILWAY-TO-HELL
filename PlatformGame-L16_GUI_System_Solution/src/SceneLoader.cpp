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
#include "Enemy.h"
#include "Pathfinding.h"
#include "Item.h"

SceneLoader::SceneLoader() {
    currentScene = 1;
}

SceneLoader::~SceneLoader() {}

void SceneLoader::LoadScene(int level) {

    
    Engine::GetInstance().sceneLoader->FadeIn(1.0f);   // Animation speed (FadeIn)
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
            Vector2D(playerNode.attribute("x").as_int(),
                playerNode.attribute("y").as_int()));
    }
    LoadEnemiesItems(sceneNode);

    Engine::GetInstance().sceneLoader->FadeOut(1.0f);    // Animation speed(FadeOut)


}

void SceneLoader::LoadEnemiesItems(pugi::xml_node sceneNode) {

    pugi::xml_node enemiesNode = sceneNode.child("entities").child("enemies");
    if (!enemiesNode) {
        return;
    }

    // Create enemies
    for (pugi::xml_node enemyNode = enemiesNode.child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy")) {
        int deathValue = enemyNode.attribute("death").as_int();
		int deathXMLValue = enemyNode.attribute("savedDeath").as_int();
        if (deathValue == 0 && deathXMLValue == 0 || deathValue == 1 && deathXMLValue == 0) {
            Enemy* enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ENEMY);
            enemy->SetParameters(enemyNode);
            Engine::GetInstance().scene->enemyList.push_back(enemy);
        }
    }

    pugi::xml_node itemsNode = sceneNode.child("entities").child("items");
    if (itemsNode) {
        for (pugi::xml_node itemNode = itemsNode.child("item"); itemNode; itemNode = itemNode.next_sibling("item"))
        {
            int deathValue = itemNode.attribute("death").as_int();
            int deathXMLValue = itemNode.attribute("savedDeath").as_int();
            if (deathValue == 0 && deathXMLValue == 0 || deathValue == 1 && deathXMLValue == 0) {
                Item* item = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
                item->SetParameters(itemNode);
                Engine::GetInstance().scene->itemList.push_back(item);  // Now using the member variable
            }
        }
    }

    // Initialize enemies
    for (auto enemy : Engine::GetInstance().scene->enemyList) {
        enemy->Start();
    }

    // Initialize items
    for (auto item : Engine::GetInstance().scene->itemList) {
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
        if (entity->type == EntityType::ENEMY || entity->type == EntityType::ITEM) {
            entitiesToRemove.push_back(entity);
        }
    }

    // Destroy each entity separately
    for (auto entity : entitiesToRemove) {
        entityManager->DestroyEntity(entity);
    }

    // Clear your local tracking list
    Engine::GetInstance().scene->enemyList.clear();
    Engine::GetInstance().scene->itemList.clear(); 
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
