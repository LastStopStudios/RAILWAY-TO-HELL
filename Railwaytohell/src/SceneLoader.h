#pragma once
#include "Module.h"
#include <vector>
#include "pugixml.hpp"
#include "Item.h"
#include "Doors.h"
#include "Levers.h"
#include "MosaicLevers.h"
#include "MosaicPiece.h"
#include "MosaicPuzzle.h"
#include "Elevators.h"
#include "Entity.h"
#include "Engine.h"
#include "EntityManager.h" 
#include "Ffmpeg.h"
#include <thread>        
#include <atomic>        

class Terrestre;
class Scene;
class Engine;
class Entity;
class Pathfinding;
class EntityManager;
class Item;
class Caronte;
class Bufon;
class Volador;
class MosaicPiece;
class MosaicLever;
class MosaicPuzzle;

class SceneLoader
{
public:
    SceneLoader();
    ~SceneLoader();
    void LoadScene(int level, int x, int y, bool fade, bool bosscam);
    void LoadSceneWithVideo(int level, int x, int y, bool fade, bool bosscam);
    void DrawScene(int level, int x, int y);
    void SetCurrentScene(int level);
    int GetCurrentLevel() const { return currentScene; }
    void FadeIn(float speed);
    void FadeOut(float speed, bool loadscene, int level = -1, int x = -1, int y = -1);
    int currentScene;

private:
    void LoadEnemiesItems(pugi::xml_node sceneNode, int scene = -1);
    void UnLoadEnemiesItems();
    void VisibilityScene(int level);
    void SetupMosaicPuzzle();

    // Métodos para carga con video
    void LoadSceneInBackground(int level, int x, int y, bool fade, bool bosscam);
    void PlayLoadingVideo();
    bool IsSceneLoadingComplete() const { return sceneLoadingComplete.load(); }
    void ShowSimpleLoadingScreen();

    // Variables para carga asíncrona
    std::atomic<bool> sceneLoadingComplete;
    std::thread loadingThread;
    Ffmpeg* videoPlayer;

    // Ruta del video de carga único
    static const char* LOADING_VIDEO_PATH;

};
