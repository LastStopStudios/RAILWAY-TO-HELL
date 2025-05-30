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
#include "Checkpoints.h"
#include "Log.h"
#include "Bufon.h"
#include "Devil.h"

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
    VisibilityScene(level);
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
        player->RestoreFullStats();
    }
    LoadEnemiesItems(sceneNode, level);
}

void SceneLoader::LoadEnemiesItems(pugi::xml_node sceneNode, int scene) {

    pugi::xml_node enemiesNode = sceneNode.child("entities").child("enemies");

    for (pugi::xml_node enemyNode = enemiesNode.child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy")) {
        std::string type = enemyNode.attribute("type").as_string();
		std::string ref = enemyNode.attribute("ref").as_string();
        int deathValue = enemyNode.attribute("death").as_int();
        int deathXMLValue = enemyNode.attribute("savedDeath").as_int();

        if (type == "rastrero") {
            Terrestre* enemy = (Terrestre*)Engine::GetInstance().entityManager->CreateEntity(EntityType::TERRESTRE);
            enemy->SetParameters(enemyNode);
            Engine::GetInstance().scene->GetEnemyList().push_back(enemy); 
        }
        if (type == "amego") {
            Explosivo* explo = (Explosivo*)Engine::GetInstance().entityManager->CreateEntity(EntityType::EXPLOSIVO);
            explo->SetParameters(enemyNode);
            Engine::GetInstance().scene->GetExploList().push_back(explo);
        }

        if (type == "volador") {
            Volador* volador = (Volador*)Engine::GetInstance().entityManager->CreateEntity(EntityType::VOLADOR);
            volador->SetParameters(enemyNode);
            Engine::GetInstance().scene->GetVoladorList().push_back(volador); 
        }

        if (deathValue == 0 && deathXMLValue == 0 || deathValue == 1 && deathXMLValue == 0) {
            if (type == "boss" && ref == "bufon") {
                Bufon* bufon = (Bufon*)Engine::GetInstance().entityManager->CreateEntity(EntityType::BUFON);
                bufon->SetParameters(enemyNode);
                Engine::GetInstance().scene->GetBufonList().push_back(bufon);
            }
        }
        if (deathValue == 0 && deathXMLValue == 0 || deathValue == 1 && deathXMLValue == 0) {
            if (type == "boss" && ref == "devil") {
                Devil* devil = (Devil*)Engine::GetInstance().entityManager->CreateEntity(EntityType::DEVIL);
                devil->SetParameters(enemyNode);
                Engine::GetInstance().scene->GetDevilList().push_back(devil);
            }
        }
        if (deathValue == 0 && deathXMLValue == 0 || deathValue == 1 && deathXMLValue == 0) {
            if (type == "boss" && ref == "noma") {
                Boss* boss = (Boss*)Engine::GetInstance().entityManager->CreateEntity(EntityType::BOSS);
                boss->SetParameters(enemyNode);
                boss->SetAliveInXML();
                Engine::GetInstance().scene->GetBossList().push_back(boss);
            }
        }
        
        if (type == "guardian") {
			Caronte* caronte = (Caronte*)Engine::GetInstance().entityManager->CreateEntity(EntityType::CARONTE);
			caronte->SetParameters(enemyNode);
			Engine::GetInstance().scene->GetCaronteList().push_back(caronte);
		}
    }

    pugi::xml_node checkpointsNode = sceneNode.child("entities").child("checkpoints");
    if (checkpointsNode) {
        for (pugi::xml_node checkpointNode = checkpointsNode.child("checkpoint"); checkpointNode; checkpointNode = checkpointNode.next_sibling("checkpoint"))
        {
            bool activatedXMLValue = checkpointNode.attribute("activated").as_bool();
			if (activatedXMLValue) { // if activated then create the checkpoint activated
                Checkpoints* checkpoint = (Checkpoints*)Engine::GetInstance().entityManager->CreateEntity(EntityType::CHECKPOINT);
                checkpoint->SetParameters(checkpointNode);
				checkpoint->setActivatedToTrue(scene);
                checkpoint->setToActivatedAnim();
                Engine::GetInstance().scene->GetCheckpointsList().push_back(checkpoint);
            }
			else { // else create the checkpoint not activated
				Checkpoints* checkpoint = (Checkpoints*)Engine::GetInstance().entityManager->CreateEntity(EntityType::CHECKPOINT);
				checkpoint->SetParameters(checkpointNode);
				Engine::GetInstance().scene->GetCheckpointsList().push_back(checkpoint);
			}

        }
    }

    pugi::xml_node itemsNode = sceneNode.child("entities").child("items");
    if (itemsNode) {
        for (pugi::xml_node itemNode = itemsNode.child("item"); itemNode; itemNode = itemNode.next_sibling("item"))
        {
            int deathValue = itemNode.attribute("death").as_int();
            int deathXMLValue = itemNode.attribute("savedDeath").as_int();
			bool createdXMLValue = itemNode.attribute("created").as_bool();
			std::string name = itemNode.attribute("name").as_string();
            if (name == "Whip") {
                if (createdXMLValue) {
                    if (deathValue == 0 && deathXMLValue == 0 || deathValue == 1 && deathXMLValue == 0) {
                        Item* item = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
                        item->SetParameters(itemNode);
                        item->SetAliveInXML();
                        Engine::GetInstance().scene->GetItemList().push_back(item);
                    }
                }
            }
            else {
                if (deathValue == 0 && deathXMLValue == 0 || deathValue == 1 && deathXMLValue == 0) {
                    Item* item = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
                    item->SetParameters(itemNode);
                    item->SetAliveInXML();
                    Engine::GetInstance().scene->GetItemList().push_back(item);
                }
            }
        }
    }
    // MOSAIC
    pugi::xml_node mosaicPiecesNode = sceneNode.child("entities").child("mosaicPieces");
    if (mosaicPiecesNode) {
        for (pugi::xml_node mosaicPieceNode = mosaicPiecesNode.child("mosaicPiece"); mosaicPieceNode; mosaicPieceNode = mosaicPieceNode.next_sibling("mosaicPiece"))
        {
            MosaicPiece* piece = (MosaicPiece*)Engine::GetInstance().entityManager->CreateEntity(EntityType::MOSAIC_PIECE);
            piece->SetParameters(mosaicPieceNode);
            Engine::GetInstance().scene->GetMosaicPiecesList().push_back(piece);
        }
    }
    pugi::xml_node mosaicLeversNode = sceneNode.child("entities").child("mosaicLevers");
    if (mosaicLeversNode) {
        for (pugi::xml_node mosaicLeverNode = mosaicLeversNode.child("mosaicLever"); mosaicLeverNode; mosaicLeverNode = mosaicLeverNode.next_sibling("mosaicLever"))
        {
            MosaicLever* lever = (MosaicLever*)Engine::GetInstance().entityManager->CreateEntity(EntityType::MOSAIC_LEVER);
            lever->SetParameters(mosaicLeverNode);
            Engine::GetInstance().scene->GetMosaicLeversList().push_back(lever);
        }
    }
    pugi::xml_node mosaicPuzzleNode = sceneNode.child("entities").child("mosaicPuzzle");
    if (mosaicPuzzleNode) {
        // Create the mosaic puzzle
        MosaicPuzzle* puzzle = new MosaicPuzzle();

        // Set sound effect IDs if provided in the XML
        if (mosaicPuzzleNode.attribute("solve_fx_id")) {
            puzzle->solveFxId = mosaicPuzzleNode.attribute("solve_fx_id").as_int();
        }
        if (mosaicPuzzleNode.attribute("rotate_fx_id")) {
            puzzle->rotateFxId = mosaicPuzzleNode.attribute("rotate_fx_id").as_int();
        }

        // Initialize the puzzle
        puzzle->Initialize();

        // Add puzzle to the scene's puzzle list
        Engine::GetInstance().scene->GetMosaicPuzzleList().push_back(puzzle);
    }
    SetupMosaicPuzzle();
    //
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
    for (auto enemy : Engine::GetInstance().scene->GetExploList()) {
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
    // Initialize mosaic
    for (auto pieces : Engine::GetInstance().scene->GetMosaicPiecesList()) {
        pieces->Start();
    }
    for (auto levers : Engine::GetInstance().scene->GetMosaicLeversList()) {
        levers->Start();
    }

    // Initialize levers
	for (auto lever : Engine::GetInstance().scene->GetLeversList()) {
		lever->Start();
	}
    for (auto checkpoint : Engine::GetInstance().scene->GetCheckpointsList()) {
        checkpoint->Start();
    }
    // Initialize elevators
    for (auto elevator : Engine::GetInstance().scene->GetElevatorsList()) {
        elevator->Start();
    }
    for (auto bufon : Engine::GetInstance().scene->GetBufonList()) {
        bufon->Start();
    }
    for (auto devil : Engine::GetInstance().scene->GetDevilList()) {
        devil->Start();
    }
}

void SceneLoader::UnLoadEnemiesItems() {
    // Get the entity manager
    auto entityManager = Engine::GetInstance().entityManager.get();

    // Make a copy of entities to safely iterate
    std::vector<Entity*> entitiesToRemove;

    // Find all enemies and items (skip the player)
    for (auto entity : entityManager->entities) {
        if (entity->type == EntityType::TERRESTRE || entity->type == EntityType::EXPLOSIVO || entity->type == EntityType::ITEM || entity->type == EntityType::VOLADOR || entity->type == EntityType::DEVIL || entity->type == EntityType::BOSS || entity->type == EntityType::CARONTE || entity->type == EntityType::DOORS || entity->type == EntityType::LEVER || entity->type == EntityType::ELEVATORS || entity->type == EntityType::BUFON || entity->type == EntityType::CHECKPOINT || entity->type == EntityType::MOSAIC_LEVER || entity->type == EntityType::MOSAIC_PIECE || entity->type == EntityType::MOSAIC_PUZZLE) {
            entitiesToRemove.push_back(entity);
        }
    }

    // Destroy each entity separately
    for (auto entity : entitiesToRemove) {
        entityManager->DestroyEntity(entity);
    }

    // Clear your local tracking list
    Engine::GetInstance().scene->GetEnemyList().clear();
    Engine::GetInstance().scene->GetExploList().clear();
    Engine::GetInstance().scene->GetVoladorList().clear(); 
    Engine::GetInstance().scene->GetBossList().clear();
	Engine::GetInstance().scene->GetCaronteList().clear();
	Engine::GetInstance().scene->GetDoorsList().clear();
	Engine::GetInstance().scene->GetLeversList().clear();
    Engine::GetInstance().scene->GetElevatorsList().clear();
	Engine::GetInstance().scene->GetItemList().clear();
    Engine::GetInstance().scene->GetBufonList().clear();
    Engine::GetInstance().scene->GetDevilList().clear();
    Engine::GetInstance().scene->GetMosaicPiecesList().clear();
    Engine::GetInstance().scene->GetMosaicLeversList().clear();
    Engine::GetInstance().scene->GetMosaicPuzzleList().clear();
    Engine::GetInstance().scene->GetCheckpointsList().clear();
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

void SceneLoader::SetupMosaicPuzzle() {
    // Get the puzzle from the scene
    auto& puzzleList = Engine::GetInstance().scene->GetMosaicPuzzleList();
    if (puzzleList.empty()) {
        LOG("Error: No MosaicPuzzle found when setting up the puzzle!");
        return;
    }

    MosaicPuzzle* puzzle = puzzleList[0];

    // Add all pieces to the puzzle
    for (auto piece : Engine::GetInstance().scene->GetMosaicPiecesList()) {
        puzzle->AddPiece(piece);
    }

    // Set the puzzle reference for all levers
    for (auto lever : Engine::GetInstance().scene->GetMosaicLeversList()) {
        lever->SetPuzzle(puzzle);
    }
}

void SceneLoader::VisibilityScene(int level) {//player goes to another scene
    pugi::xml_document loadFile;
    if (!loadFile.load_file("config.xml")) {
        return;
    }
    pugi::xml_node configNode = loadFile.child("config");
    if (!configNode) {
        return;
    }
    std::string sceneName = (level == 1) ? "scene" : "scene" + std::to_string(level);
    pugi::xml_node sceneNode = configNode.child(sceneName.c_str());
    if (!sceneNode) {
        return;
    }
    pugi::xml_node visible = sceneNode.child("visibility"); //search if the scene have been visited
    if (!visible) {
        return;
    }
    if (visible.attribute("accessed").as_bool()) {
        visible.attribute("accessed").set_value(true); //scene have been visited       
    }
    visible.attribute("accessed").set_value(true); //scene have been visited 
    loadFile.save_file("config.xml");
}
