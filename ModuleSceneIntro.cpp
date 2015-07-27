#include "Globals.h"
#include "Application.h"
#include "ModuleSceneIntro.h"

void toClipboard(const char* str, uint size)
{
	OpenClipboard(0);
	EmptyClipboard();
	HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, size);
	if(!hg)
	{
		CloseClipboard();
		return;
	}
	memcpy(GlobalLock(hg), str, size);
	GlobalUnlock(hg);
	SetClipboardData(CF_TEXT, hg);
	CloseClipboard();
	GlobalFree(hg);
}


ModuleSceneIntro::ModuleSceneIntro(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	graphics = NULL;
	selected = NULL;
	strcpy_s(name, "points");
	strcpy_s(file_coords, "./unknows_coords.txt");
}

ModuleSceneIntro::~ModuleSceneIntro()
{}

// Load assets
bool ModuleSceneIntro::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;

	App->renderer->camera.x = 50;
	App->renderer->camera.y = 50;

	if(App->GetArgument(1) != NULL)
		Load(App->GetArgument(1));

	return ret;
}

void ModuleSceneIntro::Load(const char* filename)
{
	CleanUp();

	// Graphics
	strcpy_s(file, filename);
	uint len = strlen(file);
	for(uint i = 0; i < len; ++i)
	{
		if(file[i] == '\\')
		{
			file[i] = '/';
		}
	}

	graphics = App->textures->Load(file);

	// Analyze file path
	int to_slash, to_dot;

	for(int i = len; i >= 0; --i)
	{
		if(file[i] == '.')
			to_dot = i;

		if(file[i] == '/')
		{
			to_slash = i + 1;
			break;
		}
	}

	// set obj name
	strcpy_s(name, &file[to_slash]);
	name[strlen(name) - (len - to_dot)] = '\0';

	// set coord name
	strcpy_s(file_coords, file);
	file_coords[to_dot] = '\0';
	strcat_s(file_coords, "_coords.txt");

	char buf[DATA_BUFFER];
	FILE* f;
	if(fopen_s(&f, file_coords, "r") == 0)
	{
		fread_s(buf, DATA_BUFFER, 1, DATA_BUFFER, f);
		fclose(f);

		// Parse file and create points
		int i = 0;
		int l = strlen(buf);

		while(buf[i] != '\n' && i < l)
			++i;

		int readed = EOF;
		p2Point<int> p;
		float x, y;
		do
		{
			readed = sscanf_s(&buf[i++], " %ff, %ff", &x, &y);
			if(readed == EOF)
				break;

			p.x = (int)x;
			p.y = (int)y;
			points.add(p);

			while(buf[i] != '\n' && i < l)
				++i;
		} while(1);
	}
}

// Load assets
bool ModuleSceneIntro::CleanUp()
{
	LOG("Unloading Intro scene");

	if(graphics != NULL)
		App->textures->Unload(graphics);

	if(points.count() > 0)
	{
		char buf[DATA_BUFFER];
		sprintf_s(buf, "float %s[%d] = {\n", name, points.count() * 2);

		char line[255] = "";

		p2List_item<p2Point<int>>* item = points.getFirst();

		while(item != NULL)
		{
			App->renderer->DrawQuad({item->data.x, item->data.y, 5, 5}, 255, 255, 255, 255, false);
			sprintf_s(line, 255, "\t%d.0f, %d.0f,\n", item->data.x, item->data.y);
			strcat_s(buf, line);

			item = item->next;
		}

		buf[strlen(buf) - 2] = 0;
		strcat_s(buf, "\n};\n");

		FILE* f;
		if(fopen_s(&f, file_coords, "w") == 0)
		{
			fputs(buf, f);
			fclose(f);
		}

		// also copy to the clipboard
		toClipboard(buf, strlen(buf));

		points.clear();

		selected = NULL;
	}

	return true;
}

// Update: draw background
update_status ModuleSceneIntro::Update()
{
	if(graphics != NULL)
		App->renderer->Blit(graphics, 0, 0);

	char str[256];
	p2Point<int> mouse;
	mouse.x = App->input->GetMouseX() / App->renderer->zoom - App->renderer->camera.x / App->renderer->zoom;
	mouse.y = App->input->GetMouseY() / App->renderer->zoom - App->renderer->camera.y / App->renderer->zoom;

	if(graphics != NULL)
		sprintf_s(str, "Vertex-Box2D [%s.png] %d, %d", name, mouse.x, mouse.y);
	else
		sprintf_s(str, "Vertex-Box2D [UNKNOWN] %d, %d", mouse.x, mouse.y);

	App->window->SetTitle(str);
	
	App->renderer->zoom += App->input->GetMouseZ() * 0.1f;

	if(App->input->GetKey(SDL_SCANCODE_KP_PLUS) == KEY_DOWN)
	{
		App->renderer->zoom += 0.25f;
	}

	if(App->input->GetKey(SDL_SCANCODE_KP_MINUS) == KEY_DOWN)
	{
		if(App->renderer->zoom >= 0.5f)
			App->renderer->zoom -= 0.25f;
	}

	if(selected != NULL && (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_UP || App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_IDLE))
	{
		selected = NULL;
	}

	App->renderer->DrawLine(-App->renderer->camera.x / App->renderer->zoom, 0, App->renderer->camera.w / App->renderer->zoom - App->renderer->camera.x / App->renderer->zoom, 0, 100, 100, 100, 100, true);
	App->renderer->DrawLine(0, -App->renderer->camera.y / App->renderer->zoom, 0, App->renderer->camera.h / App->renderer->zoom - App->renderer->camera.y / App->renderer->zoom, 100, 100, 100, 100, true);

	p2List_item<p2Point<int>>* item = points.getFirst();
	p2Point<int> prev;

	while(item != NULL)
	{
		int quad_size = (int) (5.0f / App->renderer->zoom);

		bool filled = false;
		if(selected == item || (selected == NULL && mouse.DistanceTo(item->data) < 5))
		{
			selected = item;
			filled = true;
		}

		App->renderer->DrawQuad({item->data.x - quad_size, item->data.y - quad_size, quad_size + quad_size, quad_size + quad_size}, 255, (item->next) ? 255 : 0, 100, 255, filled, true);

		if(item != points.getFirst())
		{
			App->renderer->DrawLine(prev.x, prev.y, item->data.x, item->data.y, 255, 255, 255, 255, true);
		}

		prev = item->data;
		item = item->next;
	}

	if(App->input->GetKey(SDL_SCANCODE_DELETE) == KEY_DOWN)
	{
		if(selected != NULL)
		{
			points.del(selected);
			selected = NULL;
		}
	}

	if(App->input->GetKey(SDL_SCANCODE_BACKSPACE) == KEY_DOWN)
	{
		points.del(points.getLast());
	}

	if(App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_REPEAT)
	{
		if(selected != NULL)
		{
			selected->data = mouse;
		}
	}

	if(App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		if(selected == NULL)
		{
			selected = points.add(mouse);
		}
	}

	if(App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_DOWN)
	{
		panning_center = mouse;
	}

	if(App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_REPEAT)
	{
		App->renderer->camera.x += mouse.x - panning_center.x;
		App->renderer->camera.y += mouse.y - panning_center.y;
	}

	if(App->input->FileWasDropped() == true)
	{
		Load(App->input->GetDroppedFile());
	}

	return UPDATE_CONTINUE;
}
