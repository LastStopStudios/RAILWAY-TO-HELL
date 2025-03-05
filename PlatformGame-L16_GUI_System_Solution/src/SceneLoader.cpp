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

SceneLoader::~SceneLoader() {}

void SceneLoader::LoadScene(int level) {
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
}


void SceneLoader::SetCurrentScene(int level)
{
	currentScene = level;
}

