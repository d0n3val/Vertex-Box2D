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
	pivot.x = pivot.y = 0;
	pivot_rotation = pivot_positions::top_left;
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

		pivot.x = -1; 
		pivot.y = -1;


		sscanf_s(&buf[i], "// Pivot %d, %d", &pivot.x, &pivot.y);
		
		while(buf[i] != '\n' && i < l)
			++i;

		int readed = EOF;
		p2Point<int> p;
		++i;

		do
		{	
			while(buf[i] != '\n' && buf[i] != ';'&& i < l)
				++i;

			readed = sscanf_s(&buf[i++], " %d, %d", &p.x, &p.y);
			if(readed == EOF || buf[i] == '}')
				break;

			points.add(p);

		} while(1);

		App->renderer->camera.x -= pivot.x;
		App->renderer->camera.y -= pivot.y;
	}
}

// Load assets
bool ModuleSceneIntro::CleanUp()
{
	LOG("Unloading Intro scene");

	if(points.count() > 0)
	{
		char buf[DATA_BUFFER];

		// Print absolute coordinates ---
		sprintf_s(buf, "// Pivot %d, %d\nint %s[%d] = {\n", pivot.x, pivot.y, name, points.count() * 2);
		char line[255] = "";

		p2List_item<p2Point<int>>* item = points.getFirst();

		while(item != NULL)
		{
			App->renderer->DrawQuad({item->data.x, item->data.y, 5, 5}, 255, 255, 255, 255, false);
			sprintf_s(line, 255, "\t%d, %d,\n", item->data.x, item->data.y);
			strcat_s(buf, line);

			item = item->next;
		}

		buf[strlen(buf) - 2] = 0;
		strcat_s(buf, "\n};\n");

		// Print relative coordinates ---
		if(graphics != NULL)
		{
			int width, height;

			SDL_QueryTexture(graphics, NULL, NULL, &width, &height);

			sprintf_s(line, "\n// Pivot %f, %f\nfloat %s[%d] = {\n", (float)pivot.x / width, (float)pivot.y / height, name, points.count() * 2);
			strcat_s(buf, line);
			char line[255] = "";

			p2List_item<p2Point<int>>* item = points.getFirst();

			while(item != NULL)
			{
				App->renderer->DrawQuad({item->data.x, item->data.y, 5, 5}, 255, 255, 255, 255, false);
				sprintf_s(line, 255, "\t%ff, %ff,\n", (float)item->data.x / width, (float)item->data.y / height);
				strcat_s(buf, line);

				item = item->next;
			}

			buf[strlen(buf) - 2] = 0;
			strcat_s(buf, "\n};\n");

		}

		// Write to file ---
		FILE* f;
		if(fopen_s(&f, file_coords, "w") == 0)
		{
			fputs(buf, f);
			fclose(f);
		}

		// also copy to the clipboard
		toClipboard(buf, strlen(buf));

		points.clear();

		if(graphics != NULL)
			App->textures->Unload(graphics);

		selected = NULL;
	}

	return true;
}

// Update: draw background
update_status ModuleSceneIntro::Update()
{
	if(graphics != NULL)
		App->renderer->Blit(graphics, pivot.x, pivot.y);

	char str[256];
	p2Point<int> mouse;
	mouse.x = App->input->GetMouseX() / App->renderer->zoom - App->renderer->camera.x / App->renderer->zoom;
	mouse.y = App->input->GetMouseY() / App->renderer->zoom - App->renderer->camera.y / App->renderer->zoom;

	if(graphics != NULL)
		sprintf_s(str, "Vertex-Box2D [%s.png] (%d vertices) %d, %d", name, points.count(), mouse.x, mouse.y);
	else
		sprintf_s(str, "Vertex-Box2D [UNKNOWN] (%d vertices) %d, %d", points.count(), mouse.x, mouse.y);

	App->window->SetTitle(str);
	
	App->renderer->zoom += App->input->GetMouseZ() * 0.1f;

	if(App->input->GetKey(SDL_SCANCODE_KP_PLUS) == KEY_DOWN)
	{
		App->renderer->zoom += 0.25f / App->renderer->zoom;
	}

	if(App->input->GetKey(SDL_SCANCODE_KP_MINUS) == KEY_DOWN)
	{
		if(App->renderer->zoom >= 0.5f)
			App->renderer->zoom -= 0.25f / App->renderer->zoom;
	}

	if(selected != NULL && (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_UP || App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_IDLE))
	{
		selected = NULL;
	}

	App->renderer->DrawLine(-App->renderer->camera.x / App->renderer->zoom, 0, App->renderer->camera.w / App->renderer->zoom - App->renderer->camera.x / App->renderer->zoom, 0, 200, 200, 200, 150, true);
	App->renderer->DrawLine(0, -App->renderer->camera.y / App->renderer->zoom, 0, App->renderer->camera.h / App->renderer->zoom - App->renderer->camera.y / App->renderer->zoom, 200, 200, 200, 150, true);

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
		if(App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RSHIFT) == KEY_REPEAT)
		{
			ChangePivot(mouse);
		}
		else if(selected != NULL)
		{
			selected->data = mouse;
		}
	}

	if(App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		if(App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RSHIFT) == KEY_REPEAT)
		{
			ChangePivot(mouse);
		}
		else if(selected == NULL)
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

	if(App->input->GetKey(SDL_SCANCODE_P) == KEY_DOWN)
	{
		if(graphics != NULL)
		{
			int width, height;
			p2Point<int> new_pivot = pivot;
			SDL_QueryTexture(graphics, NULL, NULL, &width, &height);

			// rotate pivot to predefined positions
			++pivot_rotation;
			if(pivot_rotation == pivot_positions::max)
				pivot_rotation = pivot_positions::top_left;

			switch(pivot_rotation)
			{
				case pivot_positions::top_mid:
				new_pivot.x += width / 2;
				break;
				case pivot_positions::top_right:
				new_pivot.x += width;
				break;
				case pivot_positions::right:
				new_pivot.x += width;
				new_pivot.y += height / 2;
				break;
				case pivot_positions::center:
				new_pivot.x += width / 2;
				new_pivot.y += height / 2;
				break;
				case pivot_positions::left:
				new_pivot.y += height / 2;
				break;
				case pivot_positions::bottom_right:
				new_pivot.y += height;
				new_pivot.x += width;
				break;
				case pivot_positions::bottom_mid:
				new_pivot.y += height;
				new_pivot.x += width / 2;
				break;
				case pivot_positions::bottom_left:
				new_pivot.y += height;
				break;
			}

			ChangePivot(new_pivot);
		}
	}

	if(App->input->FileWasDropped() == true)
	{
		Load(App->input->GetDroppedFile());
	}

	return UPDATE_CONTINUE;
}

void ModuleSceneIntro::ChangePivot(const p2Point<int>& new_pivot)
{
	pivot -= new_pivot;

	// move center of coordinates to mouse
	p2List_item<p2Point<int>>* item = points.getFirst();

	while(item != NULL)
	{
		item->data -= new_pivot;
		item = item->next;
	}

	App->renderer->camera.x += new_pivot.x;
	App->renderer->camera.y += new_pivot.y;
}