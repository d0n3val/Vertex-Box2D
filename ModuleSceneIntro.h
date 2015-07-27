#pragma once
#include "Module.h"
#include "p2List.h"
#include "p2Point.h"
#include "Globals.h"

#define BOUNCER_TIME 200
#define DATA_BUFFER 8192

enum pivot_positions
{
	top_left,
	top_mid,
	top_right,
	right,
	center,
	bottom_right,
	bottom_mid,
	bottom_left,
	left,
	max
};

class ModuleSceneIntro : public Module
{
public:
	ModuleSceneIntro(Application* app, bool start_enabled = true);
	~ModuleSceneIntro();

	bool Start();
	void Load(const char* filename);
	update_status Update();
	bool CleanUp();
	void ChangePivot(const p2Point<int>& new_pivot);

public:

	SDL_Texture* graphics;
	p2Point<int > pivot;
	p2List<p2Point<int>> points;
	char file[512];
	char file_coords[512];
	char name[64];
	p2List_item<p2Point<int>>* selected;
	p2Point<int> panning_center;
	int pivot_rotation;
};
