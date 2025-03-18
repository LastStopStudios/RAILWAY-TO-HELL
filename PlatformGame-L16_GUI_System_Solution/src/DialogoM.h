#pragma once
#include "Module.h"
#include <vector>
#include "SDL2/SDL_ttf.h"
#include <string>

struct SDL_Texture;

class DialogoM
{
public:
	// Called before DialogueManager is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	void Texto(const std::string& Dialogo);
private:

	SDL_Texture* textTexture = nullptr;//textura letras
	SDL_Texture* Fondo = nullptr;//textura fondo letras
	SDL_Texture* shadowTexture = nullptr;//textura sombra !!!!No va!!!!

	//voids texto
	void GenerateTextTexture();
	void UpdateTextAnimation(float dt);
	void XMLToVariable(const std::string& id);
	// Variables para manejar el texto
	bool showText = false;
	std::string scene = "";
	std::string displayText = ""; // variable que recibe el texto del xml
	std::string currentText = ""; // Texto que se muestra
	//std::string hermana1 = "01";//Variable con la id del texto a imprimir !!!!Esta variable es la que tendra que dar el trigger que llame al void!!!!
	int textIndex = 0;           // Índice del último carácter mostrado
	float textTimer = 0.0f;      // Temporizador para controlar la velocidad
	float textSpeed = 5.0f;     // Velocidad de escritura (segundos entre letras)
	int textMaxWidth = 900; // Máximo ancho antes de saltar de línea

};

