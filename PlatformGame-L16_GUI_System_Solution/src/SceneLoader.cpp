#include "SceneLoader.h"
#include "Engine.h"
#include "Map.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Player.h" 
#include "Scene.h"  // Asegúrate de incluir el archivo correcto

SceneLoader::SceneLoader() {
	currentScene = 1;
}

SceneLoader::~SceneLoader()
{

}

void SceneLoader::LoadScene(int level) {
	// Cambia el nombre de la escena según el nivel
	SetCurrentScene(level);  
	/*Engine::GetInstance().audio.get()->StopAllFx();
	Engine::GetInstance().audio.get()->PlayAmbientFx(envoirmentfx);*/
	pugi::xml_document loadFile;
	pugi::xml_parse_result result = loadFile.load_file("config.xml");
	if (!result) {
		std::cout << "Error al cargar config.xml: " << result.description() << std::endl;
		return;
	}
	/*Engine::GetInstance().scene->CleanUpEnemies();*/
	pugi::xml_node configNode = loadFile.child("config");

	/*Engine::GetInstance().entityManager->ReloadEnemies(configNode);*/
	pugi::xml_node sceneNode = configNode.child("scene2"); 

	if (!configNode) {
		std::cout << "No se encontró el nodo <config>\n";
	}
	else {
		std::cout << "Nodo <config> encontrado\n";

		pugi::xml_node playerNode = sceneNode.child("entities").child("player");
		if (!playerNode) {
			std::cout << "No se encontró el nodo <player>\n";
		}
		else {
			std::cout << "Nodo <player> encontrado correctamente\n";
		}
	}

	if (level == 2) {
		Engine::GetInstance().map->CleanUp();
		pugi::xml_node sceneNode = configNode.child("scene2");
		std::string mapPath = sceneNode.child("map").attribute("path").as_string();
		std::string mapName = sceneNode.child("map").attribute("name").as_string();
		Engine::GetInstance().map->Load(mapPath, mapName);
		pugi::xml_node playerNode = sceneNode.child("entities").child("player");
		Vector2D playerPos(playerNode.attribute("x").as_int(), 
			playerNode.attribute("y").as_int()); 
		Player* player = Engine::GetInstance().scene->GetPlayer();
		player->SetPosition(playerPos);
		    
	}
	else {
		Engine::GetInstance().map->CleanUp();
		pugi::xml_node sceneNode = loadFile.child("config").child("scene");
		std::string mapPath = sceneNode.child("map").attribute("path").as_string();
		std::string mapName = sceneNode.child("map").attribute("name").as_string();
		Engine::GetInstance().map->Load(mapPath, mapName);
		Vector2D playerPos(sceneNode.child("entities").child("player").attribute("x").as_int(),
			sceneNode.child("entities").child("player").attribute("y").as_int());
		Player* player = Engine::GetInstance().scene->GetPlayer(); 
		player->SetPosition(playerPos); 
	}
}

void SceneLoader::SetCurrentScene(int level)
{
	currentScene = level;
}

