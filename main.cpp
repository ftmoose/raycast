#include <vector>
#include <iostream>

#include "SDL.h"
#include "SDL_ttf.h"

#define mapWidth 24
#define mapHeight 24

#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 384

using namespace std;

int32_t game_running = 0;

int worldMap[mapWidth][mapHeight]=
{
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,3,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,3,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,3,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,3,3,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,0,3,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,0,0,0,3,0,0,3,0,3,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,3,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,0,0,2,0,0,0,3,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,3,0,3,0,0,1},
  {1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,5,0,0,0,3,0,3,0,0,1},
  {1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};



void cancel_main_loop() { game_running = 0; }

void set_main_loop_arg(void (*fcn)(void *), void *const arg, uint32_t const fps, int const infinite_loop) {
  uint32_t const MPF = fps > 0 ? 1000 / fps : 1000 / 60; // milliseconds per frame

  game_running = 1;
  while (game_running) {
    uint32_t const startTime = SDL_GetTicks();
    fcn(arg);
    uint32_t const endTime = SDL_GetTicks();

    uint32_t const timeDiff = endTime - startTime;

    if (timeDiff < MPF) {
      SDL_Delay( MPF - timeDiff); // Sleep for the remainder of the time to maintain FPS
    }
  }
}

struct GameResources {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  SDL_Surface *display;
  TTF_Font *font;
  uint32_t windowID;
};

void FreeGameResources(GameResources *res) {
  TTF_CloseFont(res->font);
  SDL_FreeSurface(res->display);
  SDL_DestroyTexture(res->texture);
  SDL_DestroyRenderer(res->renderer);
  SDL_DestroyWindow(res->window);
}

struct camera {

	// x and y start positions
	double posX;	
	double posY;

	// initial direction vector
	double dirX;
	double dirY;

	// 2d raycaster version of camera plane
	double planeX;
	double planeY;

	// time
	double time;
	double oldTime;

	// speed
	double moveSpeed;
	double rotSpeed;

} cam {	12, 22,			// start positions
		-1, 0,			// direction
		0, 0.66,		// camera plane
		0, 0,			// time
		0, 0 };			// speed

int ProcessEvent(uint32_t windowID) {
  
	static SDL_Event event;

	double oldDirX, oldPlaneX;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {

			case SDL_WINDOWEVENT:
				if (event.window.windowID == windowID) {
					switch (event.window.event) {

						case SDL_WINDOWEVENT_CLOSE:
							SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Window closed\n");
							event.type = SDL_QUIT;
							SDL_PushEvent(&event);
						break;

						default:
							break;
					}
				}
				break;

			case SDL_QUIT:
				SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL_QUIT!\n");
				return 0; // 0 - signal terminate program
			
			case SDL_KEYUP:
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym) {
					case SDLK_UP:
						if (worldMap[int(cam.posX + cam.dirX * cam.moveSpeed)][int(cam.posY)] == false) 
							cam.posX += cam.dirX * cam.moveSpeed;
						if (worldMap[int(cam.posX)][int(cam.posY + cam.dirY * cam.moveSpeed)] == false)
							cam.posY += cam.dirY * cam.moveSpeed;
						break;
					case SDLK_DOWN:
						if (worldMap[int(cam.posX - cam.dirX * cam.moveSpeed)][int(cam.posY)] == false)
							cam.posX -= cam.dirX * cam.moveSpeed;
						if (worldMap[int(cam.posX)][int(cam.posY - cam.dirY * cam.moveSpeed)] == false)
							cam.posY -= cam.dirY * cam.moveSpeed;
						break;
					case SDLK_RIGHT:
						oldDirX = cam.dirX;
						cam.dirX = cam.dirX * cos(-cam.rotSpeed) - cam.dirY * sin(-cam.rotSpeed);
						cam.dirY = oldDirX * sin(-cam.rotSpeed) + cam.dirY * cos(-cam.rotSpeed);
						oldPlaneX = cam.planeX;
						cam.planeX = cam.planeX * cos(-cam.rotSpeed) - cam.planeY * sin(-cam.rotSpeed);
						cam.planeY = oldPlaneX * sin(-cam.rotSpeed) + cam.planeY * cos(-cam.rotSpeed);
						break;
					case SDLK_LEFT:
						oldDirX = cam.dirX;
						cam.dirX = cam.dirX * cos(cam.rotSpeed) - cam.dirY * sin(cam.rotSpeed);
						cam.dirY = oldDirX * sin(cam.rotSpeed) + cam.dirY * cos(cam.rotSpeed);
						oldPlaneX = cam.planeX;
						cam.planeX = cam.planeX * cos(cam.rotSpeed) - cam.planeY * sin(cam.rotSpeed);
						cam.planeY = oldPlaneX * sin(cam.rotSpeed) + cam.planeY * cos(cam.rotSpeed);
						break;
				}
		}
	}
	return 1; // 1 - signal something processed
}

void setup(GameResources* res){
	// VERY IMPORTANT: Ensure SDL2 is initialized
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "could not initialize sdl2: %s\n", SDL_GetError());
    	exit(1);
	}

	// VERY IMPORTANT: if using text in your program, ensure SDL2_ttf library is
	// initialized
	if (TTF_Init() < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "could not initialize SDL2_ttf: %s\n", TTF_GetError());
		exit(1);
	}

	// This creates the actual window in which graphics are displayed
	res->window = SDL_CreateWindow("Raycaster", SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

	if (res->window == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not create window!\n");
		exit(1);
	}

	res->windowID = SDL_GetWindowID(res->window);

	res->renderer = SDL_CreateRenderer(res->window, -1, 0); // don't force hardware or software
                                        				// accel -- let SDL2 choose what's best

	if (res->renderer == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "could not create renderer: %s\n", SDL_GetError());
 		exit(1);
	}

	res->display = SDL_CreateRGBSurfaceWithFormat(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_PIXELFORMAT_RGBA8888);
	if (res->display == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "could not create surface: %s\n", SDL_GetError());
		exit(1);
	}

	res->texture = SDL_CreateTexture(res->renderer, SDL_PIXELFORMAT_RGBA8888, 
									SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	if (res->texture == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "could not create texture: %s\n", SDL_GetError());
		exit(1);
	}

	// Just in case you need text:
	// load iosevka-regular.ttf at a large size into font
	res->font = TTF_OpenFont("iosevka-regular.ttf", 16);
	if (!res->font) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_OpenFont: %s\n", TTF_GetError());
		exit(1);
	}

	SDL_FillRect(res->display, NULL, SDL_MapRGB(res->display->format, 0x00, 0x00, 0x00));

}

void drawVerLine(GameResources* res, int x, int drawStart, int drawEnd, uint32_t color) {

	uint32_t (*pixels)[SCREEN_WIDTH] = (uint32_t(*)[SCREEN_WIDTH]) res->display->pixels;
	
	for (int y = drawStart; y <= drawEnd; ++y){
		pixels[y][x] = color;
	}

}


void GameLoop(void *const arg){

	GameResources *const res = (GameResources *const)arg;

	if (ProcessEvent(res->windowID) == 0) {
		FreeGameResources(res);
		cancel_main_loop();
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Exiting!\n");
		SDL_Quit();
		exit(0);
	}
	
	int w = SCREEN_WIDTH;
	int h = SCREEN_HEIGHT;

	// Raycasting loop
	for (int x = 0; x < w; x++){
		// calculate ray position and direction
		double cameraX = 2 * x / double(w) - 1;		// x-coordinate in camera space
		double rayDirX = cam.dirX + cam.planeX * cameraX;
		double rayDirY = cam.dirY + cam.planeY * cameraX;

		// which box of the map we're in
		int mapX = int(cam.posX);
		int mapY = int(cam.posY);

		// length of ray from current position to next x or y side
		double sideDistX, sideDistY;

		// length of ray from one x or y side to next x or y side
		double deltaDistX = abs(1/rayDirX);		// or sqrt(1 + rayDirY^2 / rayDirX^2)
		double deltaDistY = abs(1/rayDirY);		// or sqrt(1 + rayDirx^2 / rayDirY^2)
		double perpWallDist;

		// what direction to step in x or y direction (either +1 or -1)
		int stepX;
		int stepY;

		int hit = 0; 		// was wall hit?
		int side;			// was a NS or EW wall hit?

		// calculate step and initial sideDist
		if (rayDirX < 0) {
			stepX = -1;
			sideDistX = ( cam.posX - mapX ) * deltaDistX;
		}	
		else {
			stepX = 1;
			sideDistX = ( mapX + 1.0 - cam.posX ) * deltaDistX;
		}
		if (rayDirY < 0) {
			stepY = -1;
			sideDistY = ( cam.posY - mapY ) * deltaDistY;
		}
		else {
			stepY = 1;
			sideDistY = ( mapY + 1.0 - cam.posY ) * deltaDistY;
		}
		
		// DDA line algorithm
		while (hit == 0) {
			if (sideDistX < sideDistY) {
				sideDistX += deltaDistX;
				mapX += stepX;
				side = 0;
			}
			else {
				sideDistY += deltaDistY;
				mapY += stepY;
				side = 1;
			}
			if (worldMap[mapX][mapY] > 0) hit = 1;
		}

		// calculate distance projected on camera direction (Euclidean distance will give fisheye effect)
		if (side == 0) 	perpWallDist = ( mapX - cam.posX + (1 - stepX) / 2 ) / rayDirX;
		else 			perpWallDist = ( mapY - cam.posY + (1 - stepY) / 2 ) / rayDirY;


		int lineHeight = (int)( h / perpWallDist );
		
		// calculate lowest and highest pixel to fill in current stripe (center of wall is at center of screen)
		int drawStart = -lineHeight / 2 + h / 2;
		if (drawStart < 0) drawStart = 0;
		int drawEnd = lineHeight / 2 + h / 2;
		if (drawEnd >= h) drawEnd = h - 1;

		uint32_t color;
		switch (worldMap[mapX][mapY]){
			case 1:		color = 0xff0000ff;	break; // red
			case 2: 	color = 0x00ff00ff; break; // green
			case 3: 	color = 0x0000ffff; break; // blue
			case 4: 	color = 0xff00ffff; break; // white
			default:	color = 0xffff00ff; break; // yellow
		}

		// give x and y sides different brightness
		if ( side == 1 ) color = color - color/4;
		
		// draw the pixels of the stripe as a vertical line
		drawVerLine(res, x, drawStart, drawEnd, color);

	}
	
	cam.oldTime = cam.time;
	cam.time = SDL_GetTicks();
	double frameTime = (cam.time - cam.oldTime) / 1000.0; 	// frameTime is the time this frame has taken, in seconds
	
	static char frameTimeString[128];
	static SDL_Color const color = {255, 255, 0, 255};
	SDL_snprintf(frameTimeString, sizeof(frameTimeString), "Frame time: %d", (int)(frameTime*1000));
	SDL_Surface *fpsGauge = TTF_RenderText_Blended(res->font, frameTimeString, color);
    SDL_BlitSurface(fpsGauge, NULL, res->display, NULL);
    SDL_FreeSurface(fpsGauge);

	cam.moveSpeed = frameTime * 5.0;		// the constant value is in squares/second
	cam.rotSpeed = frameTime * 3.0;			// the constant value is in radians/second




	

	SDL_UpdateTexture(res->texture, NULL, res->display->pixels, res->display->pitch);
	SDL_RenderClear(res->renderer);
	SDL_RenderCopy(res->renderer, res->texture, NULL, NULL);
	SDL_RenderPresent(res->renderer);

	SDL_FillRect(res->display, NULL, SDL_MapRGB(res->display->format, 0x00, 0x00, 0x00));
}









int main()
{
	GameResources res;
	setup(&res);

	set_main_loop_arg(GameLoop, &res, 0, 1);

	return 0;
}
