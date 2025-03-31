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


DialogoM::DialogoM() : Module()
{
	name = "dialogoM";
}

// Destructor
DialogoM::~DialogoM()
{
	CleanUp();
}

// Called before render is available
bool DialogoM::Awake()
{
	return true;
}

// Called before the first frame
bool DialogoM::Start()
{
	return true;
}

// Called each loop iteration
bool DialogoM::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool DialogoM::Update(float dt) {
	UpdateTextAnimation(dt); // Llama a la función que maneja la animación del texto
	return true;
}

// Called each loop iteration
bool DialogoM::PostUpdate()
{
	if (showText && textTexture != nullptr) {
		int width = 1200, height = 600, posx = 40, posy = -100;
			
		SDL_Rect dstRect = { posx, posy, width, height }; // Posicionar y escalar el fondo del texto
		SDL_RenderCopy(Engine::GetInstance().render->renderer, fondo, nullptr, &dstRect);
		Engine::GetInstance().render->DrawTexture(textTexture, 310, 80, nullptr, 1.0f, 0.0, INT_MAX, INT_MAX); // Dibujar el texto
	}
	return true;
}
	

void DialogoM::Texto(const std::string& Dialogo) {
	showText = !showText; // Alternar visibilidad del texto
	if (showText) {
		XMLToVariable(Dialogo); // Cargar el texto correspondiente
		GenerateTextTexture(); // Generar la textura inicial
	}else{
		ResetText(); // Reiniciar el texto
	}
}


void DialogoM::ResetText() {
	textIndex = 0;
	currentText = "";
	displayText = "";//texto entero
	Skip = true;
	if (textTexture != nullptr) {
		SDL_DestroyTexture(textTexture);
		textTexture = nullptr;
	}
	showText = false; // Reiniciar la visibilidad del texto
}

void DialogoM::GenerateTextTexture()//mostrar texto por pantalla
{
	if (textTexture != nullptr)
	{
		SDL_DestroyTexture(textTexture); // Liberar textura anterior
		textTexture = nullptr;
	}

	SDL_Color color = { 255, 255, 255 }; // Decidir el color de la letra
	TTF_Font* font = Engine::GetInstance().render->font;

	if (!font) {
		LOG("Error: Fuente no cargada");
		return;
	}

	SDL_Surface* surface = TTF_RenderText_Blended_Wrapped(font, currentText.c_str(), color, textMaxWidth);// TTF_RenderText_Blended_Wrapped divide el texto en lineas automaticamente
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

	if (textTimer >= textSpeed && currentText.length() != displayText.length())
	{
		textTimer = 0.0f; // Reinicia el temporizador
		textIndex++; // Avanza en el texto
		LOG("TextIndex: %d", textIndex);//timer
		currentText = displayText.substr(0, textIndex); // Obtiene el nuevo fragmento
		LOG("displayText: %s", displayText.c_str());//texto entero
		LOG("currentText: %s", currentText.c_str());//texto que se va actualizando 
		GenerateTextTexture(); // Genera la nueva textura con el fragmento
	}
	if (currentText.length() != displayText.length() && Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_DELETE) == KEY_DOWN) {//Skipear textos Tecla suprimir No pilla el enter ni por Execute.
		currentText = displayText;
		GenerateTextTexture();
	}else if (currentText.length() == displayText.length() && Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_DELETE) == KEY_DOWN && Skip == false) {//Cerrar textos Tecla suprimir No pilla el enter ni por Execute.
		showText = !showText;
		ResetText(); // Reiniciar el texto
	}
	if (currentText.length() == displayText.length()) { Skip = false; }//el texto ya se ha skipeado, sirve de control para que no haga el skip y el cerrar a la vez

	
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
