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
	Fondo = Engine::GetInstance().textures->Load("Assets/Textures/Fondo.png"); // Cargar textura para el fondo del texto
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
		int width = 200, height = 80;
		Engine::GetInstance().textures->GetSize(Fondo, width, height); // Obtener tamaño de la textura
		int windowWidth, windowHeight;
		Engine::GetInstance().window->GetWindowSize(windowWidth, windowHeight); // Obtener tamaño de la ventana
		SDL_Rect dstRect = { 150, -100, 1000, 600 }; // Posicionar el fondo del texto
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Fondo, nullptr, &dstRect);
		Engine::GetInstance().render->DrawTexture(textTexture, 200, 80, nullptr, 1.0f, 0.0, INT_MAX, INT_MAX); // Dibujar el texto
	}
	return true;
}
	

void DialogoM::Texto(const std::string& Dialogo) {

	showText = !showText; // Alternar visibilidad del texto
	if (showText) {
		XMLToVariable(Dialogo); // Cargar el texto correspondiente
		textIndex = 0; // Reiniciar el índice del texto
		currentText = ""; // Reiniciar el texto actual
		GenerateTextTexture(); // Generar la textura inicial
	}
	else {
		ResetText(); // Reiniciar el texto
	}
	/*showText = !showText; // Alternar visibilidad del texto
	int x = 200, y = 80;
	XMLToVariable(Dialogo);//cargar el texto que toque a la variable !!!!Este es el void que tendra que llamar el trigger para los dialogos, la variable es donde va la ID!!!!
	Fondo = Engine::GetInstance().textures->Load("Assets/Textures/Fondo.png");//cargar textura para el fondo del texto
	if (textTexture != nullptr) {
		int with = 200, height = 80;
		Engine::GetInstance().textures->GetSize(Fondo, with, height);//le damos tamaño a la textura
		int windowWidth, windowHeight;
		Engine::GetInstance().window->GetWindowSize(windowWidth, windowHeight);//miramos el tamaño de la pantalla de juego
		SDL_Rect dstRect = { 150,-100, 1000, 600 };//posicionamos el fondo del texto en pantalla
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Fondo, nullptr, &dstRect);
		Engine::GetInstance().render.get()->DrawTexture(textTexture, x, y, NULL, 1.0f, 0.0, INT_MAX, INT_MAX);//dibuja la letra renderizada como textura		
	}*/
}

void DialogoM::ResetText() {
	textIndex = 0;
	currentText = "";
	displayText = "";
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
		//CleanUp();
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

	if (!textTexture)// || shadowTexture == nullptr
	{
		LOG("Error al crear textura de texto/sombra: %s", SDL_GetError());
	}
}

void DialogoM::UpdateTextAnimation(float dt)
{
	if (!showText || textIndex >= displayText.length()) return;

	textTimer += dt; // Aumenta el temporizador

	if (textTimer >= textSpeed )
	{
		textTimer = 0.0f; // Reinicia el temporizador
		textIndex++; // Avanza en el texto
		currentText = displayText.substr(0, textIndex); // Obtiene el nuevo fragmento
		GenerateTextTexture(); // Genera la nueva textura con el fragmento
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

	switch (currentLvl) {//añadir apartado por cada escena
	case 1:
		scene = "scene";//pasar la primera escena
		break;
	case 2:
		scene = "scene2";//pasar la segunda escena
		break;
	default:
		scene = "scene";
		break;
	}
	pugi::xml_node dialogueNode = loadFile.child("config").child(scene.c_str()).child("dialogues");//cargar los dialogos de la escena
	for (pugi::xml_node dialog = dialogueNode.child("dialog"); dialog; dialog = dialog.next_sibling("dialog")) {//recorrer todos los dialogos
		if (std::string(dialog.attribute("ID").value()) == id) {//Mira si el id que se le da y el del dialogo es el mismo
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
	
	if (Fondo != nullptr) {
		Engine::GetInstance().textures->UnLoad(Fondo);//descargar fondo texto
		Fondo = nullptr;
	}
	return true;
}
