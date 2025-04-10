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

		//Posicion fondo
		posx = w - 1000;//background position with screen size
		posy = h - 200; //background position with screen size

		//posicion texto
		texty = posy + 80 ;//text position with screen size
		textx = posx + 50 ;//text position with screen size
		
		SDL_Rect dstRect = { posx, posy, width, height }; //Position and scale text background
		SDL_RenderCopy(Engine::GetInstance().render->renderer, fondo, nullptr, &dstRect);
		Engine::GetInstance().render->DrawTexture(textTexture, textx, texty, nullptr, 0.0f, 0.0, INT_MAX, INT_MAX); //Draw the text
	}
	
	return true;
}


void DialogoM::Texto(const std::string& Dialogo) {
	showText = !showText; //Toggle text visibility
	if (showText) {
		Engine::GetInstance().scene->DialogoOn();//stop entities
		XMLToVariable(Dialogo); //Load corresponding text
		GenerateTextTexture(); //Generate the initial texture
	}
	else {
		ResetText(); //Reset text
	}
}


void DialogoM::ResetText() {
	textIndex = 0;
	currentText = "";
	alltext = "";
	displayText = "";//full text
	Skip = true;
	Tim = true;
	Siguiente = true;
	if (textTexture != nullptr) {
		SDL_DestroyTexture(textTexture);
		textTexture = nullptr;
	}
	showText = false; //Reset text visibility 
}

void DialogoM::GenerateTextTexture()//display text on the screen
{
	if (textTexture != nullptr)
	{
		SDL_DestroyTexture(textTexture); // Release previous texture
		textTexture = nullptr;
	}

	SDL_Color color = { 255, 255, 255 }; // Decide on the font color
	TTF_Font* font = Engine::GetInstance().render->font;

	if (!font) {
		LOG("Error: Fuente no cargada");
		return;
	}

	SDL_Surface* surface = TTF_RenderText_Blended_Wrapped(font, currentText.c_str(), color, textMaxWidth);// TTF_RenderText_Blended_Wrapped divides text into lines automatically
	if (surface == nullptr)
	{
		LOG("Error al crear superficie de texto: %s", SDL_GetError());
		return;
	}

	textTexture = SDL_CreateTextureFromSurface(Engine::GetInstance().render->renderer, surface);
	SDL_FreeSurface(surface);

	if (!textTexture)
	{
		LOG("Error al crear textura de texto/sombra: %s", SDL_GetError());
	}
}

void DialogoM::UpdateTextAnimation(float dt)
{
	if (!showText) return;

	textTimer += dt; // Aumenta el temporizador

	if (textTimer >= textSpeed && alltext.length() != displayText.length())
	{
		textTimer = 0.0f; // Reinicia el temporizador
		if (Tim == true) {
			textIndex++;
			LOG("textIndex: %d", textIndex);
		}// Avanza en el texto
		alltext = displayText.substr(0, textIndex); // Obtiene el nuevo fragmento
		if (textIndex < textMaxheigth) {
			LOG("TextIndex: %d", textIndex);//timer
			currentText = displayText.substr(0, textIndex); // Obtiene el nuevo fragmento
			LOG("displayText: %s", displayText.c_str());//texto entero
			LOG("currentText: %s", currentText.c_str());//texto que se va actualizando 
			GenerateTextTexture(); // Genera la nueva textura con el fragmento
		}
		else if (textIndex == textMaxheigth) {
			Tim = false;
			LOG("Entro Tim : % d", Tim);
		}
		if (textIndex >= textMaxheigth && Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_DELETE) == KEY_DOWN && Tim == false) {

			int finaltexto = textIndex - textMaxheigth;
			currentText.clear();
			currentText = displayText.substr(textMaxheigth, finaltexto); // Obtiene el nuevo fragmento
			LOG("displayText: %s", displayText.c_str());//texto entero
			LOG("currentText: %s", currentText.c_str());//texto que se va actualizando 
			GenerateTextTexture(); // Genera la nueva textura con el fragmento
			Siguiente = false;//pasa de cargar la otra parte del texto
			Tim = true;//activar el temporizador del dibujado del texto
		}
		else if (textIndex >= textMaxheigth && Siguiente == false) {
			//Tim = true;
			int finaltexto = textIndex - textMaxheigth;
			currentText = displayText.substr(textMaxheigth - 1, finaltexto); // Obtiene el nuevo fragmento
			GenerateTextTexture(); // Genera la nueva textura con el fragmento
		}


	}
	if (alltext.length() == displayText.length()) { Skip = false; }//el texto ya se ha skipeado, sirve de control para que no haga el skip y el cerrar a la vez
	
	if (alltext.length() == displayText.length() && Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_DELETE) == KEY_DOWN && Skip == false) {//Cerrar textos Tecla suprimir No pilla el enter ni por Execute.
		showText = !showText;
		Engine::GetInstance().scene->DialogoOff();//devolver control al player
		ResetText(); // Reiniciar el texto
	}



}

void DialogoM::XMLToVariable(const std::string& id) {
	if (!showText || !displayText.empty()) return;

	pugi::xml_document loadFile;
	pugi::xml_parse_result result = loadFile.load_file("config.xml");


	if (!result) {
		std::cerr << "Error cargando el archivo XML: " << result.description() << std::endl;
		return;
	}
	int currentLvl = Engine::GetInstance().sceneLoader->GetCurrentLevel(); //sacar la escena actual
	scene = (currentLvl == 1) ? "scene" : "scene" + std::to_string(currentLvl);//pasar la escena de donde sacar los dialogos
	pugi::xml_node dialogueNode = loadFile.child("config").child(scene.c_str()).child("dialogues");//cargar los dialogos de la escena

	for (pugi::xml_node dialog = dialogueNode.child("dialog"); dialog; dialog = dialog.next_sibling("dialog")) {//recorrer todos los dialogos
		if (std::string(dialog.attribute("ID").value()) == id) {//Mira si el id que se le da y el del dialogo es el mismo
			fondo = Engine::GetInstance().textures->Load(dialog.attribute("Img").value()); // Cargar textura para el fondo del texto
			displayText = dialog.attribute("TEXT").value();//darle el texto a la variable 
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
		Engine::GetInstance().textures->UnLoad(fondo);//descargar fondo texto
		fondo = nullptr;
	}
	return true;
}
