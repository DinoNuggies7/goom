#include "input.h"
#include "draw.h"
#include "map.h"
#include "object.h"
#include "player.h"

#include <SDL2/SDL.h>

SDL_Window* window;
SDL_Renderer* renderer;

int main(int argc, char** argv) {
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_PNG);

	SDL_DisplayMode display;
	SDL_GetCurrentDisplayMode(0, &display);

	window = SDL_CreateWindow("doom'd", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, display.w, display.h, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN_DESKTOP);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_Rect view = {0, 0, 640, 336};
	SDL_RenderSetScale(renderer, (float)display.w / view.w, (float)display.h / (view.h + 24));

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);

	SDL_SetRelativeMouseMode(true);
	// SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	SDL_Surface* texture[3];
	texture[0] = IMG_Load("res/brick.png");
	texture[1] = IMG_Load("res/stone.png");
	texture[2] = IMG_Load("res/darkbrickbig.png");

	float ticks = SDL_GetTicks();
	float lastTicks;
	float dt;

	ItemFunction itemFunc;
	SetItemFunctions(&itemFunc);

	Player player;
	Player__INIT(&player, &itemFunc);

	ObjectFunction objFunc;
	SetObjectFunctions(&objFunc);

	int objects = 2;
	Object object[objects];
	object[0] = CreateObject(OBJECT_DERK, &objFunc);
	object[1] = CreateObject(OBJECT_CHAIR, &objFunc);

	object[0].pos.x = 4;
	object[0].pos.y = 3.5;
	object[1].pos.x = 7;
	object[1].pos.y = 6;

	Map map;
	parseMap(&map, player.map);

	for (int j = 0; j < map.h; j++) {		// Print map data
		for (int i = 0; i < map.w; i++) {
			if (getTile(map, i, j) == TILE_SPAWN) {
				printf("@ ");
				player.pos.x = i;	// Setting player spawn point,
				player.pos.y = j;	//  should move to player.c (maybe)
			}
			else if (getTile(map, i, j) > TILE_COLLISION_START && getTile(map, i, j) < TILE_COLLISION_END)
				printf("%d ", getTile(map, i, j));
			else
				printf(". ");
		}
		printf("\n");
	}

	while (!player.quit) {
		SDL_Event event;
		handleInput(&event, &player);

		lastTicks = ticks;
		ticks = SDL_GetTicks();
		dt = (ticks - lastTicks) / 1000.f;

		Player__UPDATE(&player, &map, dt);

		for (int i = 0; i < objects; i++) {
			// Update Objects
			if (!object[i].destroy) {
				if (object[i].type > OBJECT_NONE && object[i].type < OBJECT_TYPES)
					object[i].update(&object[i], dt, &player, map, view);
				ObjectGlobalUPDATE(&object[i], player, map, dt);
			}
			else // Remove Objects
				ObjectGlobalINIT(&object[i]);			
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		drawBackground(renderer, view);
		drawForeground(renderer, view, texture, objects, object, player, map);
		drawHUD(renderer, view, player);

		SDL_RenderPresent(renderer);
	}
	
	save(&player);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
	return 0;
}