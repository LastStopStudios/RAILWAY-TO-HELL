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

	// Called after all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	void Texto(const std::string& Dialogo);

	void ResetText();

private:

	SDL_Texture* textTexture = nullptr;//texture letters
	SDL_Texture* fondo = nullptr;//background texture

	//text voids
	void GenerateTextTexture(); //display text on the screen
	void UpdateTextAnimation(float dt);//Call the function that handles text animation
	void XMLToVariable(const std::string& id);//extract data from xml

	//Variables to handle text
	bool showText = false;
	//error control booleans
	bool Tim = true;//go on to load the other part of the text
	bool Siguiente = true;//activate the text drawing timer
	std::string scene = "";//pass scene from which to take the dialogues
	//Dialog control
	std::string displayText = ""; //variable that receives the entire text of the xml
	std::string alltext = "";//control variable
	std::string currentText = ""; //Text displayed and updated
	//timer and write control
	int textIndex = 0;           // Index of the last character displayed
	float textTimer = 0.0f;      // Timer for speed control
	float textSpeed = 5.0f;     // Writing speed (seconds between letters)
	float Skip = true; //Skip texts | Now only closes texts
	//Margins and position of dialogs and backgrounds
	int textMaxWidth = 580; // Maximum width before jumping out of line
	int textMaxheigth = 352; //Maximum length before the dialog break
	int h, w;
	int width, height;
	int posx, posy;
	int texty, textx;


};

