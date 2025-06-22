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
#include <mutex>
#include "Ffmpeg.h"

const char* SceneLoader::LOADING_VIDEO_PATH = "Assets/Videos/loading.mp4";
SceneLoader::SceneLoader() {
    currentScene = 1;
    sceneLoadingComplete.store(false);
    videoPlayer = nullptr;
}

SceneLoader::~SceneLoader() {
    if (loadingThread.joinable()) {
        loadingThread.join();
    }
}

void SceneLoader::LoadSceneWithVideo(int level, int x, int y, bool fade, bool bosscam) {
    LOG("Starting scene loading with video for level %d", level);

    // Resetear el flag de carga completada
    sceneLoadingComplete.store(false);

    // Iniciar la carga de la escena en un hilo separado
    loadingThread = std::thread([this, level, x, y, fade, bosscam]() {
        LoadSceneInBackground(level, x, y, fade, bosscam);
        });

    // Mientras tanto, reproducir el video de carga en el hilo principal
    PlayLoadingVideo();

    // Esperar a que termine la carga de la escena
    if (loadingThread.joinable()) {
        loadingThread.join();
    }

    if (fade) {
        FadeOut(1.0f, false);
    }

    LOG("Scene loading with video completed for level %d", level);
}

void SceneLoader::LoadSceneInBackground(int level, int x, int y, bool fade, bool bosscam) {
    LOG("Starting scene loading for level %d", level);

    Mix_PauseMusic();

    // Configurar cámara
    if (bosscam == true) {
        Engine::GetInstance().scene->EntrarBoss();
        Engine::GetInstance().scene->BloquearSensor();
    }
    else if (bosscam == false) {
        Engine::GetInstance().scene->SalirBoss();
        Engine::GetInstance().scene->DesbloquearSensor();
    }

    // Descargar entidades anteriores
    UnLoadEnemiesItems();

    // Configurar nueva escena
    SetCurrentScene(level);
    VisibilityScene(level);

    // Cargar nueva escena
    DrawScene(level, x, y);

    Mix_ResumeMusic();

    LOG("Scene loading completed for level %d", level);

    // Marcar como completada
    sceneLoadingComplete.store(true);
}

void SceneLoader::PlayLoadingVideo() {
    // Obtener instancia del reproductor de video
    videoPlayer = Engine::GetInstance().ffmpeg.get();

    if (!videoPlayer) {
        LOG("Warning: No video player available, using simple loading screen");
        ShowSimpleLoadingScreen();
        return;
    }

    // Cargar el video de carga
    if (videoPlayer->LoadVideo(LOADING_VIDEO_PATH)) {
        LOG("Warning: Could not load loading video, using simple loading screen");
        ShowSimpleLoadingScreen();
        return;
    }

    LOG("Playing loading video while scene loads...");

    // Reproducir el video hasta que termine la carga de la escena
    while (!IsSceneLoadingComplete()) {
        // Reproducir el siguiente frame del video
        bool videoEnded = videoPlayer->ConvertPixels(LOADING_VIDEO_PATH);

        // Si el video terminó pero la carga aún no está completa, reiniciar el video
        if (videoEnded) {
            LOG("Loading video ended, checking if scene loading is complete...");

            // Verificar si la carga ya terminó
            if (IsSceneLoadingComplete()) {
                LOG("Scene loading completed when video ended - stopping playback");
                break;
            }

            LOG("Scene still loading, restarting video...");

            // Reiniciar el video (NO cerrar el video player)
            if (videoPlayer->LoadVideo(LOADING_VIDEO_PATH)) {
                // No se pudo reiniciar el video, usar pantalla simple
                LOG("Could not restart video, switching to simple loading screen");
                ShowSimpleLoadingScreen();
                break;
            }
            // Video reiniciado correctamente, continuar en el bucle
            continue;
        }

        // Pequeña pausa para controlar el framerate y permitir que el hilo de carga trabaje
        SDL_Delay(16); // ~60 FPS
    }

    // Limpiar el reproductor de video cuando la carga esté completa
    if (videoPlayer) {
        // Detener el renderizado del video si existe el método

            videoPlayer->StopRendering();
        

        // Pequeña pausa para asegurar que cualquier renderizado en progreso termine
        SDL_Delay(50);

        videoPlayer->CloseCurrentVideo();
        LOG("Video player closed after scene loading completion");
    }

    LOG("Loading video stopped - scene loading completed");
}
void SceneLoader::ShowSimpleLoadingScreen() {
    SDL_Renderer* renderer = Engine::GetInstance().render->renderer;
    LOG("Showing simple loading screen");

    while (!IsSceneLoadingComplete()) {
        // Procesar eventos para mantener la ventana responsiva
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        static int dots = 0;
        static Uint32 lastTime = 0;
        Uint32 currentTime = SDL_GetTicks();

        if (currentTime - lastTime > 500) {
            dots = (dots + 1) % 4;
            lastTime = currentTime;
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}

void SceneLoader::LoadScene(int level, int x, int y,bool fade,bool bosscam) {
    LoadSceneWithVideo(level, x, y, fade, bosscam);

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

        if (deathValue == 0 && deathXMLValue == 0 ) {
            if (type == "boss" && ref == "bufon") {
                Bufon* bufon = (Bufon*)Engine::GetInstance().entityManager->CreateEntity(EntityType::BUFON);
                bufon->SetParameters(enemyNode);
                bufon->SetAliveInXML();
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
        if (deathValue == 0 && deathXMLValue == 0) {
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

    pugi::xml_node estatuaNode = sceneNode.child("entities").child("estatuas");
    if (estatuaNode) {
        for (pugi::xml_node estatuNode = estatuaNode.child("estatua"); estatuNode; estatuNode = estatuNode.next_sibling("estatua"))
        {
            Estatua* estatua = (Estatua*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ESTATUA);
            estatua->SetParameters(estatuNode);
            Engine::GetInstance().scene->GetEstatuaList().push_back(estatua);
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
    // Initialize statues
    for (auto estatua : Engine::GetInstance().scene->GetEstatuaList()) {
        estatua->Start();
    }
}

void SceneLoader::UnLoadEnemiesItems() {
    // Get the entity manager
    auto entityManager = Engine::GetInstance().entityManager.get();

    // Make a copy of entities to safely iterate
    std::vector<Entity*> entitiesToRemove;

    // Find all enemies and items (skip the player)
    for (auto entity : entityManager->entities) {
        if (entity->type == EntityType::TERRESTRE || entity->type == EntityType::EXPLOSIVO || entity->type == EntityType::ITEM || entity->type == EntityType::VOLADOR || entity->type == EntityType::DEVIL || entity->type == EntityType::BOSS || entity->type == EntityType::CARONTE || entity->type == EntityType::DOORS || entity->type == EntityType::LEVER || entity->type == EntityType::ELEVATORS || entity->type == EntityType::ESTATUA || entity->type == EntityType::BUFON || entity->type == EntityType::CHECKPOINT || entity->type == EntityType::MOSAIC_LEVER || entity->type == EntityType::MOSAIC_PIECE || entity->type == EntityType::MOSAIC_PUZZLE) {
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
    Engine::GetInstance().scene->GetEstatuaList().clear();
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
