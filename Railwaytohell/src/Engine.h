#pragma once

#include <memory>
#include <list>
#include "Module.h"
#include "Timer.h"
#include "PerfTimer.h"
#include "pugixml.hpp"
#include "SDL2/SDL.h"

// Modules
class Window;
class Input;
class Render;
class Textures;
class Audio;
class Scene;
class EntityManager;
class SceneLoader;
class Map;
class Physics;
class DialogoM;
class Mapa;
class GuiManager;
class UI;
class Ffmpeg;

class Engine
{
public:

	// Public method to get the instance of the Singleton
	static Engine& GetInstance();

	//	
	void AddModule(std::shared_ptr<Module> module);

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool Update();

	// Called before quitting
	bool CleanUp();

	float GetDt() const {
		return dt;
	}

	// Game Controller methods
	void InitGameController();
	bool IsControllerConnected() const { return gameController != nullptr; }
	SDL_GameController* GetGameController() const { return gameController; }

private:

	// Private constructor to prevent instantiation
	// Constructor
	Engine();

	// Delete copy constructor and assignment operator to prevent copying
	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;

	// Call modules before each loop iteration
	void PrepareUpdate();

	// Call modules before each loop iteration
	void FinishUpdate();

	// Call modules before each loop iteration
	bool PreUpdate();

	// Call modules on each loop iteration
	bool DoUpdate();

	// Call modules after each loop iteration
	bool PostUpdate();

	// Load config file
	bool LoadConfig();

	std::list<std::shared_ptr<Module>> moduleList;

public:

	enum EngineState
	{
		CREATE = 1,
		AWAKE,
		START,
		LOOP,
		CLEAN,
		FAIL,
		EXIT
	};

	// Modules
	std::shared_ptr<Window> window;
	std::shared_ptr<Input> input;
	std::shared_ptr<Render> render;
	std::shared_ptr<Textures> textures;
	std::shared_ptr<Audio> audio;
	std::shared_ptr<Scene> scene;
	std::shared_ptr<SceneLoader> sceneLoader;
	std::shared_ptr<EntityManager> entityManager;
	std::shared_ptr<Map> map;
	std::shared_ptr<Physics> physics;
	std::shared_ptr<DialogoM> dialogoM;
	std::shared_ptr<Mapa> mapa;
	std::shared_ptr<GuiManager> guiManager;
	std::shared_ptr<UI> ui;
	std::shared_ptr<Ffmpeg> ffmpeg;

private:
	// Game Controller
	SDL_GameController* gameController;

	// Delta time
	float dt;
	//Frames since startup
	int frames;

	// Calculate timing measures
	// required variables are provided:
	Timer startupTime;
	PerfTimer frameTime;
	PerfTimer lastSecFrameTime;

	int frameCount = 0;
	int framesPerSecond = 0;
	int lastSecFrameCount = 0;

	float averageFps = 0.0f;
	int secondsSinceStartup = 0;

	//Maximun frame duration in miliseconds.
	int maxFrameDuration = 16;

	std::string gameTitle = "Platformer Game";

	//L05 TODO 2: Declare a xml_document to load the config file
	pugi::xml_document configFile;
};