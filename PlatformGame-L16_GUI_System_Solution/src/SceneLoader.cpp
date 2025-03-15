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

    Engine::GetInstance().sceneLoader->FadeOut(50.0f);    UnLoadEnemiesItems();
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
    Engine::GetInstance().sceneLoader->FadeIn(50.0f);
}

void SceneLoader::LoadEnemiesItems(pugi::xml_node sceneNode) {

    pugi::xml_node enemiesNode = sceneNode.child("entities").child("enemies");
    if (!enemiesNode) {
        return;
    }

    // Create enemies
    for (pugi::xml_node enemyNode = enemiesNode.child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy")) {
        Enemy* enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ENEMY);
        enemy->SetParameters(enemyNode);
        enemysList.push_back(enemy);
    }

    // Create a vector to track items
    std::vector<Item*> itemsList;

    pugi::xml_node itemsNode = sceneNode.child("entities").child("items");
    if (itemsNode) {  // Add this check to avoid null pointer issues
        for (pugi::xml_node itemNode = itemsNode.child("item"); itemNode; itemNode = itemNode.next_sibling("item"))
        {
            Item* item = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
            item->SetParameters(itemNode);
            itemsList.push_back(item);  // Add the item to the list
        }
    }

    // Initialize enemies
    for (auto enemy : enemysList) {
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
        if (entity->type == EntityType::ENEMY || entity->type == EntityType::ITEM) {
            entitiesToRemove.push_back(entity);
        }
    }

    // Destroy each entity separately
    for (auto entity : entitiesToRemove) {
        entityManager->DestroyEntity(entity);
    }

    // Clear your local tracking list
    enemysList.clear();
}
void SceneLoader::SetCurrentScene(int level)
{
	currentScene = level;
}

void SceneLoader::FadeOut(float speed) {
    float alpha = 0.0f;
    SDL_SetRenderDrawBlendMode(Engine::GetInstance().render->renderer, SDL_BLENDMODE_BLEND);

    // Llenar la pantalla con un color negro que gradualmente se va volviendo más opaco
    while (alpha < 255.0f) {
        SDL_SetRenderDrawColor(Engine::GetInstance().render->renderer, 0, 0, 0, static_cast<Uint8>(alpha));
        SDL_RenderFillRect(Engine::GetInstance().render->renderer, NULL);
        SDL_RenderPresent(Engine::GetInstance().render->renderer);

        alpha += speed;  // Aumentar la opacidad en función de la velocidad
        SDL_Delay(10);   // Esperar para lograr el efecto
    }

    // Asegurarse de que el fade esté completamente negro al final
    SDL_SetRenderDrawColor(Engine::GetInstance().render->renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(Engine::GetInstance().render->renderer, NULL);
    SDL_RenderPresent(Engine::GetInstance().render->renderer);
}
void SceneLoader::FadeIn(float speed) {
    float alpha = 255.0f;
    SDL_SetRenderDrawBlendMode(Engine::GetInstance().render->renderer, SDL_BLENDMODE_BLEND);

    // Llenar la pantalla con un color negro que gradualmente va desapareciendo
    while (alpha > 0.0f) {
        SDL_SetRenderDrawColor(Engine::GetInstance().render->renderer, 0, 0, 0, static_cast<Uint8>(alpha));
        SDL_RenderFillRect(Engine::GetInstance().render->renderer, NULL);
        SDL_RenderPresent(Engine::GetInstance().render->renderer);

        alpha -= speed;  // Reducir la opacidad en función de la velocidad
        SDL_Delay(10);   // Esperar para lograr el efecto
    }

    // Asegurarse de que la pantalla sea completamente visible al final
    SDL_SetRenderDrawColor(Engine::GetInstance().render->renderer, 0, 0, 0, 0);
    SDL_RenderFillRect(Engine::GetInstance().render->renderer, NULL);
    SDL_RenderPresent(Engine::GetInstance().render->renderer);
}

