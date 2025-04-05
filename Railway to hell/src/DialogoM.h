#pragma once
#include "Module.h"
#include <vector>
#include <string>
#include "SDL2/SDL_ttf.h"


struct SDL_Texture;

class DialogoM : public Module
{	
public:

	DialogoM();

	// Destructor
	virtual ~DialogoM();
	
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

	void ResetText();

private:

	SDL_Texture* textTexture = nullptr;//textura letras
	SDL_Texture* fondo = nullptr;//textura fondo letras

	//voids texto
	void GenerateTextTexture(); // mostrar texto por pantalla
	void UpdateTextAnimation(float dt);// Llama a la función que maneja la animación del texto
	void XMLToVariable(const std::string& id);//sacar datos del xml

	// Variables para manejar el texto
	bool showText = false;
	//boleanos de control de errores
	bool Tim = true;//pasa de cargar la otra parte del texto
	bool Siguiente = true;///activar el temporizador del dibujado del texto
	std::string scene = "";//pasar escena de la que sacar los dialogos
	//Control del dialogo
	std::string displayText = ""; // variable que recibe el texto entero del xml
	std::string alltext = "";//variable de control de texto
	std::string currentText = ""; // Texto que se muestra y se va actualizando
	//timer y control de escritura
	int textIndex = 0;           // Índice del último carácter mostrado
	float textTimer = 0.0f;      // Temporizador para controlar la velocidad
	float textSpeed = 5.0f;     // Velocidad de escritura (segundos entre letras)
	float Skip = true; //Skipear textos | Ahora solo cierra los textos
	//Margenes y posicion dialogos y fondo
	int textMaxWidth = 900; // Máximo ancho antes de saltar de línea
	int textMaxheigth = 405; //Maximo largo antes del salto de dialogo
	int h, w;
	int width, height;
	int posx, posy;
	int texty, textx;


};

