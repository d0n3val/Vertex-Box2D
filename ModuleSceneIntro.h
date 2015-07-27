#pragma once
#include "Module.h"
#include "p2List.h"
#include "p2Point.h"
#include "Globals.h"

#define BOUNCER_TIME 200
#define DATA_BUFFER 1024

class ModuleSceneIntro : public Module
{
public:
	ModuleSceneIntro(Application* app, bool start_enabled = true);
	~ModuleSceneIntro();

	bool Start();
	void Load(const char* filename);
	update_status Update();
	bool CleanUp();


public:

	SDL_Texture* graphics;
	p2List<p2Point<int>> points;
	char file[512];
	char file_coords[512];
	char name[64];
	p2List_item<p2Point<int>>* selected;
	p2Point<int> panning_center;
};
