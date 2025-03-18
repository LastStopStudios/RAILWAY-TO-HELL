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
	return true;
}

void DialogoM::Texto(const std::string& Dialogo) {
	showText = !showText; // Alternar visibilidad del texto
	int x = 200, y = 80; // shadowOffset = 5; !!!!No hay sombra!!!!
	XMLToVariable(Dialogo);//cargar el texto que toque a la variable !!!!Este es el void que tendra que llamar el trigger para los dialogos, la variable es donde va la ID!!!!
	Fondo = Engine::GetInstance().textures->Load("Assets/Textures/Fondo.png");//cargar textura para el fondo del texto
	if (textTexture != nullptr) {
		int with = 200, height = 80;
		Engine::GetInstance().textures->GetSize(Fondo, with, height);//le damos tamaño a la textura
		int windowWidth, windowHeight;
		Engine::GetInstance().window->GetWindowSize(windowWidth, windowHeight);//miramos el tamaño de la pantalla de juego
		SDL_Rect dstRect = { 150,-100, 1000, 600 };//posicionamos el fondo del texto en pantalla
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Fondo, nullptr, &dstRect);
		//Engine::GetInstance().render.get()->DrawTexture(shadowTexture, x + shadowOffset, y + shadowOffset, NULL, 1.0f, 0.0, INT_MAX, INT_MAX); !!!!No funciona las sombras del texto, cambiar por un fondo y ya!!!! //dibuja sombra
		Engine::GetInstance().render.get()->DrawTexture(textTexture, x, y, NULL, 1.0f, 0.0, INT_MAX, INT_MAX);//dibuja la letra renderizada como textura		
	}
}
void DialogoM::GenerateTextTexture()//mostrar texto por pantalla
{
	if (textTexture != nullptr)
	{
		SDL_DestroyTexture(textTexture); // Liberar textura anterior
		textTexture = nullptr;
		CleanUp();
	}

	SDL_Color color = { 255, 255, 255 }; // Decidir el color de la letra
	SDL_Color shadowColor = { 0, 0, 0 }; // Decidir el color de la sombra
	int shadowOffset = 3; // Desplazamiento de la sombra
	//renderizar sombra !!!!No hace la sombra, cambiar por un rectangulo negro de fondo y ya!!!!
	/*SDL_Surface* shadowSurface = TTF_RenderText_Blended_Wrapped(Engine::GetInstance().render.get()->font, currentText.c_str(), shadowColor, textMaxWidth);
	SDL_Texture* shadowTexture = SDL_CreateTextureFromSurface(Engine::GetInstance().render.get()->renderer, shadowSurface);
	SDL_FreeSurface(shadowSurface);*/
	// Renderizar el texto
	SDL_Surface* surface = TTF_RenderText_Blended_Wrapped(Engine::GetInstance().render.get()->font, currentText.c_str(), color, textMaxWidth);// TTF_RenderText_Blended_Wrapped divide el texto en lineas automaticamente
	if (surface == nullptr)
	{
		LOG("Error al crear superficie de texto: %s", SDL_GetError());
		return;
	}
	textTexture = SDL_CreateTextureFromSurface(Engine::GetInstance().render.get()->renderer, surface);
	SDL_FreeSurface(surface);

	if (textTexture == nullptr)// || shadowTexture == nullptr
	{
		LOG("Error al crear textura de texto/sombra: %s", SDL_GetError());
	}
}

void DialogoM::UpdateTextAnimation(float dt)
{
	if (!showText) return;

	textTimer += dt; // Aumenta el temporizador

	if (textTimer >= textSpeed && textIndex < displayText.length())
	{
		textTimer = 0.0f; // Reinicia el temporizador
		textIndex++; // Avanza en el texto
		currentText = displayText.substr(0, textIndex); // Obtiene el nuevo fragmento
		GenerateTextTexture(); // Genera la nueva textura con el fragmento
	}
}

void DialogoM::XMLToVariable(const std::string& id) {
	if (!showText) return;
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
	/*if (shadowTexture != nullptr) !!!!No funciona las sombras del texto, cambiar por un fondo y ya!!!!
	{
		SDL_DestroyTexture(shadowTexture);
		shadowTexture = nullptr;
	}*/
	if (Fondo != nullptr) {
		Engine::GetInstance().textures->UnLoad(Fondo);//descargar fondo texto
		Fondo = nullptr;
	}
	textIndex = 0;// Reinicia la animación 
	currentText = "";// Vacía el texto mostrado
	displayText.clear();
	showText = false;
	return true;
}
