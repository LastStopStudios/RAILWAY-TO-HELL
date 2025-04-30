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


DialogoM::DialogoM() : Module()
{
	name = "dialogoM";
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

// Called each loop iteration
bool DialogoM::PostUpdate()
{
	if (showText && textTexture != nullptr) {
	
		Engine::GetInstance().window.get()->GetWindowSize(w, h);//Screen size
		//background size
		width = 800;
		height = 200;

		//Background position
		posx = w - 1050;//background position with screen size
		posy = h - 200; //background position with screen size

		//text position
		texty = posy + 65 ;//text position with background size
		textx = posx + 180 ;//text position with background size
		
		SDL_Rect dstRect = { posx, posy, width, height }; //Position and scale text background
		SDL_RenderCopy(Engine::GetInstance().render->renderer, fondo, nullptr, &dstRect);
		Engine::GetInstance().render->DrawTexture(textTexture, textx, texty, nullptr, 0.0f, 0.0, INT_MAX, INT_MAX); //Draw the text
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
	currentText = "";
	alltext = "";
	displayText = ""; // full text
	Skip = true;
	Tim = true;
	Siguiente = true;
	if (textTexture != nullptr) {
		SDL_DestroyTexture(textTexture);
		textTexture = nullptr;
	}
	showText = false; // Reset text visibility 
	// No resetear lastDialogID aquí
}

void DialogoM::GenerateTextTexture()//display text on the screen
{
	if (textTexture != nullptr)
	{
		SDL_DestroyTexture(textTexture); // Release previous texture
		textTexture = nullptr;
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
		LOG("Error creating text Render: %s", SDL_GetError());
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

		// If the dialogue was the boss fight dialogue (ID "1")
		if (lastDialogID == "1") {
			bossFightReady = true;
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
	int currentLvl = Engine::GetInstance().sceneLoader->GetCurrentLevel(); //bring out the current scene
	scene = (currentLvl == 1) ? "scene" : "scene" + std::to_string(currentLvl);//pass the scene from where to get the dialogues
	pugi::xml_node dialogueNode = loadFile.child("config").child(scene.c_str()).child("dialogues");//load scene dialogs

	for (pugi::xml_node dialog = dialogueNode.child("dialog"); dialog; dialog = dialog.next_sibling("dialog")) {//scroll through all dialogs
		if (std::string(dialog.attribute("ID").value()) == id) {//See if the id given and the one in the dialog are the same
			fondo = Engine::GetInstance().textures->Load(dialog.attribute("Img").value()); //Load texture for text background
			displayText = dialog.attribute("TEXT").value();//give the text to the variable
			return;
		}
	}
}

// Called before quitting

bool DialogoM::CleanUp()
{
	LOG("Freeing scene");

	if (textTexture != nullptr)
	{
		SDL_DestroyTexture(textTexture);
		textTexture = nullptr;
	}

	if (fondo != nullptr) {
		Engine::GetInstance().textures->UnLoad(fondo);//Unload text background
		fondo = nullptr;
	}
	return true;
}
