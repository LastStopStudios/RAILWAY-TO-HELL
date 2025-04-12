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
#include "Log.h"

SceneLoader::SceneLoader() {
    currentScene = 1;
}

SceneLoader::~SceneLoader() {}

void SceneLoader::LoadScene(int level, int x, int y,bool fade,bool bosscam) {
    LOG("Cargando escena %d...", level);

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
       
    UnLoadEnemiesItems();
    SetCurrentScene(level);

    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");
    if (!loadFile.load_file("config.xml")) {
        return;
    }
    if (!result) {
        LOG("Error al cargar config.xml: %s", result.description());
        return;
    }
    LOG("config.xml cargado correctamente");

    // Cargar las plantillas antes de cargar la escena

    LoadEnemyTemplates(loadFile);

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

void SceneLoader::LoadEnemyTemplates(pugi::xml_document& doc) {
    pugi::xml_node configNode = doc.child("config");
    if (!configNode) {
        LOG("Error: Node 'config' not found.");
        return;
    }

    pugi::xml_node templatesNode = configNode.child("templates").child("enemy_templates");
    if (!templatesNode) {
        LOG("Error: Node 'enemy_templates' not found.");
        return;
    }

    // Limpiar plantillas previas
    enemyTemplates.clear();
    LOG("Clearing existing enemy templates.");

    // Cargar todas las plantillas
    int count = 0;
    for (pugi::xml_node templateNode = templatesNode.child("template");
        templateNode;
        templateNode = templateNode.next_sibling("template")) {

        std::string id = templateNode.attribute("id").as_string();
        if (id.empty()) {
            LOG("Error: Enemy template without id. Skipping...");
            continue;
        }

        enemyTemplates[id] = templateNode;
        count++;
        LOG("Loaded template: %s", id.c_str());
    }

    LOG("Loaded %d enemy templates.", count);
}

void SceneLoader::LoadEnemiesItems(pugi::xml_node sceneNode) {
    LOG("Cargando enemigos e items...");

    pugi::xml_node enemiesNode = sceneNode.child("entities").child("enemies");
    if (!enemiesNode) {
        LOG("Error: No se encontró nodo 'enemies'");
        return;
    }

    int enemyCount = 0;
    for (pugi::xml_node enemyNode = enemiesNode.child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy")) {
        std::string templateId = enemyNode.attribute("template").as_string();
        LOG("Procesando enemigo con template: %s", templateId.c_str());

        if (enemyTemplates.find(templateId) == enemyTemplates.end()) {
            LOG("Error: No se encontró template %s", templateId.c_str());
            continue;
        }

        pugi::xml_node templateNode = enemyTemplates[templateId];
        std::string type = templateNode.attribute("type").as_string();
        LOG("Creando enemigo de tipo: %s", type.c_str());

            // Crear el enemigo según su tipo
        if (type == "rastrero") {
            Terrestre* enemy = (Terrestre*)Engine::GetInstance().entityManager->CreateEntity(EntityType::TERRESTRE);
            if (!enemy) {
                LOG("Error: No se pudo crear entidad Terrestre");
                continue;
            }

            enemy->SetParameters(templateNode);
            enemy->SetSpecificParameters(enemyNode);

            if (!enemy->Start()) {
                LOG("Error: Falló Start() para enemigo Terrestre");
                continue;
            }

            Engine::GetInstance().scene->GetEnemyList().push_back(enemy);
            enemyCount++;
            LOG("Enemigo Terrestre creado exitosamente");
        }
            else if (type == "volador") {
                Volador* volador = (Volador*)Engine::GetInstance().entityManager->CreateEntity(EntityType::VOLADOR);
                volador->SetParameters(templateNode);
                volador->SetSpecificParameters(enemyNode);
                Engine::GetInstance().scene->GetVoladorList().push_back(volador);
            }
            else if (type == "boss") {
                Boss* boss = (Boss*)Engine::GetInstance().entityManager->CreateEntity(EntityType::BOSS);
                boss->SetParameters(templateNode);
                boss->SetSpecificParameters(enemyNode);
                Engine::GetInstance().scene->GetBossList().push_back(boss);
            }
            else if (type == "guardian") {
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
    if (doorsNode) {
        for (pugi::xml_node leverNode = leversNode.child("lever"); leverNode; leverNode = leverNode.next_sibling("lever"))
        {
            Levers* lever = (Levers*)Engine::GetInstance().entityManager->CreateEntity(EntityType::LEVER);
            lever->SetParameters(leverNode);
            Engine::GetInstance().scene->GetLeversList().push_back(lever);
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

	for (auto door : Engine::GetInstance().scene->GetDoorsList()) {
		door->Start();
	}
	for (auto lever : Engine::GetInstance().scene->GetLeversList()) {
		lever->Start();
	}
    LOG("Total enemigos cargados: %d", enemyCount);
}

void SceneLoader::UnLoadEnemiesItems() {
    // Get the entity manager
    auto entityManager = Engine::GetInstance().entityManager.get();

    // Make a copy of entities to safely iterate
    std::vector<Entity*> entitiesToRemove;

    // Find all enemies and items (skip the player)
    for (auto entity : entityManager->entities) {
        if (entity->type == EntityType::TERRESTRE || entity->type == EntityType::ITEM || entity->type == EntityType::VOLADOR || entity->type == EntityType::BOSS || entity->type == EntityType::CARONTE || entity->type == EntityType::DOORS || entity->type == EntityType::LEVER) {
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
