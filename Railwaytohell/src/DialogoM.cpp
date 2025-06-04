#include "DialogoM.h"
#include "Engine.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Log.h"
#include "SDL2/SDL_ttf.h"
#include "Player.h"
#include "Mapa.h"
#include "UI.h"


DialogoM::DialogoM() : Module()
{
	name = "dialogoM";
	textTexture = nullptr;
	fondo = nullptr;
	textIndex = 0;
	textTimer = 0.0f;
	textSpeed = 0.05f;
	textMaxheigth = 300;
	textMaxWidth = 380;
	showText = false;
	Tim = true;
	Skip = true;
	Siguiente = true;
	currentText = "";
	alltext = "";
	displayText = "";
	lastDialogID = "";
	bossFightReady = false;
	currentBackgroundPath = "";
}
//Destructor
DialogoM::~DialogoM()
{
	CleanUp();
}

//Called before render is available
bool DialogoM::Awake()
{
	return true;
}

//Called before the first frame
bool DialogoM::Start()
{
	return true;
}

//Called each loop iteration
bool DialogoM::PreUpdate()
{
	
	return true;
}

// Called each loop iteration
bool DialogoM::Update(float dt) {
	UpdateTextAnimation(dt); //Call the function that handles text animation
	
	return true;
}

bool DialogoM::PostUpdate()
{
	if (showText && textTexture != nullptr && fondo != nullptr) {
		// Verificar que la textura de fondo sigue siendo válida
		if (currentBackgroundPath.empty() || backgroundCache.find(currentBackgroundPath) == backgroundCache.end()) {
			LOG("ERROR: Background texture cache corrupted");
			return true;
		}

		Engine::GetInstance().window.get()->GetWindowSize(w, h);
		width = 800;
		height = 200;
		posx = w - 1050;
		posy = h - 200;
		texty = posy - 140;
		textx = posx + 40;

		SDL_Rect dstRect = { posx, posy, width, height };
		SDL_RenderCopy(Engine::GetInstance().render->renderer, fondo, nullptr, &dstRect);
		Engine::GetInstance().render->DrawTexture(textTexture, textx, texty, nullptr, 0.0f, 0.0, INT_MAX, INT_MAX);
	}

	return true;
}

void DialogoM::Texto(const std::string& Dialogo) {

	showText = !showText; // Toggle text visibility
	if (showText) {
		lastDialogID = Dialogo;  // Guardar el ID del diálogo actual
		Engine::GetInstance().scene->DialogoOn(); // stop entities
		Engine::GetInstance().mapa->DialogoOn();//stop map from showing
		XMLToVariable(Dialogo); // Load corresponding text
		GenerateTextTexture(); // Generate the initial texture
	}
	else {
		ResetText(); // Reset text
	}
}


void DialogoM::ResetText() {
	textIndex = 0;
	textTimer = 0.0f;
	currentText = "";
	alltext = "";
	displayText = "";
	Skip = true;
	Tim = true;
	Siguiente = true;

	// Solo destruir la textura de texto, NO el fondo
	if (textTexture != nullptr) {
		SDL_DestroyTexture(textTexture);
		textTexture = nullptr;
	}

	showText = false;
	// NO resetear fondo ni currentBackgroundPath
}

void DialogoM::GenerateTextTexture()//display text on the screen
{
	if (textTexture != nullptr)
	{
		SDL_DestroyTexture(textTexture); // Release previous texture
		//textTexture = nullptr;
	}

	SDL_Color color = { 255, 255, 255 }; // Define font color
	TTF_Font* font = Engine::GetInstance().render->font;

	if (!font) {
		LOG("Error: Font not loaded");
		return;
	}

	SDL_Surface* surface = TTF_RenderText_Blended_Wrapped(font, currentText.c_str(), color, textMaxWidth);// TTF_RenderText_Blended_Wrapped divides text into lines automatically
	if (surface == nullptr)
	{
		LOG("Creating text Render : % s", SDL_GetError());
		return;
	}

	textTexture = SDL_CreateTextureFromSurface(Engine::GetInstance().render->renderer, surface);
	SDL_FreeSurface(surface);

	if (!textTexture)
	{
		LOG("Error while creating text texture: %s", SDL_GetError());
	}
}

void DialogoM::UpdateTextAnimation(float dt)
{
	if (!showText) return;  // If the text is not being shown, exit
	textTimer += dt; // Increase the timer by the delta time
	if (textTimer >= textSpeed && alltext.length() != displayText.length())
	{
		textTimer = 0.0f; // Reset the timer
		if (Tim == true) {
			textIndex++;  // Move to the next character in the text
		}
		alltext = displayText.substr(0, textIndex); // Get the updated fragment of text
		if (textIndex < textMaxheigth) {
			currentText = displayText.substr(0, textIndex);  // Update the current fragment
			GenerateTextTexture();  // Generate the new texture with the updated fragment
		}
		else if (textIndex == textMaxheigth) {
			Tim = false;  // Stop incrementing textIndex when we reach max text height
		}

		// Check both the E key and the Circle button on the controller
		bool advancePressed = Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_E) == KEY_DOWN;

		// Check if a controller is connected and if the Circle button is pressed
		if (Engine::GetInstance().IsControllerConnected()) {
			SDL_GameController* controller = Engine::GetInstance().GetGameController();
			if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B) == KEY_DOWN) {
				advancePressed = true;
			}
		}

		// If the text reaches max height and the advance button is pressed
		if (textIndex >= textMaxheigth && advancePressed && Tim == false) {
			int finaltext = textIndex - textMaxheigth;
			currentText.clear();
			currentText = displayText.substr(textMaxheigth, finaltext); // Update the text fragment
			GenerateTextTexture();  // Generate the new texture with the updated fragment
			Siguiente = false;  // Prevent the next part of the text from being loaded
			Tim = true;  // Restart the text drawing timer
		}
		else if (textIndex >= textMaxheigth && Siguiente == false) {
			int finaltext = textIndex - textMaxheigth;
			currentText = displayText.substr(textMaxheigth - 1, finaltext); // Get the updated fragment
			GenerateTextTexture();  // Generate the new texture with the updated fragment
		}
	}

	// Check if the text has already been fully displayed
	if (alltext.length() == displayText.length()) {
		Skip = false;  // Don't skip and close the dialogue at the same time
	}

	// Check both the E key and the Circle button on the controller for closing the dialogue
	bool closePressed = Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_E) == KEY_DOWN;

	// If a controller is connected, check if the Circle button is pressed
	if (Engine::GetInstance().IsControllerConnected()) {
		SDL_GameController* controller = Engine::GetInstance().GetGameController();
		if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B) == KEY_DOWN) {
			closePressed = true;
		}
	}

	// If the text is fully displayed and the close button is pressed, hide the text and return control to the player
	if (alltext.length() == displayText.length() && closePressed && Skip == false) {
		showText = !showText;  // Hide the text
		Engine::GetInstance().scene->DialogoOff(); // Return control to all entities
		Engine::GetInstance().mapa->DialogoOff(); // Return control to Map

		// If the dialogue was X then...
		if (lastDialogID == "1" && Engine::GetInstance().sceneLoader->GetCurrentLevel() == 3) {//UI Lives Noma
			bossFightReady = true;
			Engine::GetInstance().ui->figth = true;//show boss1 health
		}
		if (lastDialogID == "1" && Engine::GetInstance().sceneLoader->GetCurrentLevel() == 5) {//UI Lives Bufon
			bossFightReady = true;
			Engine::GetInstance().ui->figth2 = true;//show boss2 health
		}
		if (lastDialogID == "1" && Engine::GetInstance().sceneLoader->GetCurrentLevel() == 12) {//UI Lives Devil
			bossFightReady = true;
			Engine::GetInstance().ui->fase1 = true;//Put live boss UI from Phase 2 in screen
			Engine::GetInstance().ui->figth3 = true;//show boss2 health
		}

		ResetText();  // Reset the text system
	}
}

void DialogoM::XMLToVariable(const std::string& id) {
	if (!showText || !displayText.empty()) return;

	pugi::xml_document loadFile;
	pugi::xml_parse_result result = loadFile.load_file("config.xml");

	if (!result) {
		std::cerr << "Error loading XML file: " << result.description() << std::endl;
		return;
	}

	int currentLvl = Engine::GetInstance().sceneLoader->GetCurrentLevel();
	scene = (currentLvl == 1) ? "scene" : "scene" + std::to_string(currentLvl);
	pugi::xml_node dialogueNode = loadFile.child("config").child(scene.c_str()).child("dialogues");

	for (pugi::xml_node dialog = dialogueNode.child("dialog"); dialog; dialog = dialog.next_sibling("dialog")) {
		if (std::string(dialog.attribute("ID").value()) == id) {

			std::string backgroundPath = dialog.attribute("Img").value();

			// Verificar si ya tenemos esta textura en caché
			if (backgroundCache.find(backgroundPath) == backgroundCache.end()) {
				// No está en caché, cargarla
				SDL_Texture* newTexture = Engine::GetInstance().textures->Load(backgroundPath.c_str());
				if (newTexture != nullptr) {
					backgroundCache[backgroundPath] = newTexture;
					LOG("Loaded new background texture: %s", backgroundPath.c_str());
				}
				else {
					LOG("ERROR: Failed to load background texture: %s", backgroundPath.c_str());
					return;
				}
			}

			// Usar la textura del caché
			fondo = backgroundCache[backgroundPath];
			currentBackgroundPath = backgroundPath;
			displayText = dialog.attribute("TEXT").value();

			LOG("Using background texture: %s", backgroundPath.c_str());
			return;
		}
	}

	LOG("WARNING: Dialog with ID '%s' not found", id.c_str());
}

// Called before quitting

bool DialogoM::CleanUp()
{
	LOG("Freeing DialogoM");

	if (textTexture != nullptr)
	{
		SDL_DestroyTexture(textTexture);
		textTexture = nullptr;
	}

	// Limpiar caché de fondos
	for (auto& pair : backgroundCache) {
		if (pair.second != nullptr) {
			Engine::GetInstance().textures->UnLoad(pair.second);
		}
	}
	backgroundCache.clear();

	fondo = nullptr;
	currentBackgroundPath = "";

	return true;
}
