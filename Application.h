#pragma once

#include "p2List.h"
#include "Globals.h"
#include "Module.h"
#include "Dummy.h"
#include "ModuleWindow.h"
#include "ModuleRender.h"
#include "ModuleTextures.h"
#include "ModuleInput.h"
#include "ModuleSceneIntro.h"

class Application
{
public:
	ModuleRender* renderer;
	ModuleWindow* window;
	ModuleTextures* textures;
	ModuleInput* input;
	ModuleSceneIntro* scene_intro;

private:

	p2List<Module*> list_modules;
	int argc;
	char ** argv;

public:

	Application(int argc, char** argv);
	~Application();

	bool Init();
	update_status Update();
	bool CleanUp();
	const char* GetArgument(int pos) const;

private:

	void AddModule(Module* mod);
};