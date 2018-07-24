#include <iostream>
#include <fstream>
#include <SDL.h>
#include <map>
#include <ctime>

// Ported by @valt to SDL from https://www.youtube.com/user/FamTrinli minesweeper tutorial in SFML
// uncovering multiple fields with no mines at one time - first attempt recursive
// todo all mines marked with flags...

const int FigureSize = 32;
const int PuzzleSize = 10;
enum {MINE = 9, COVER, FLAG};
// grids, one item reserved from each side for numbers checking conditions
int backGrid[PuzzleSize+2][PuzzleSize+2];
int showGrid[PuzzleSize+2][PuzzleSize+2];
const int sizeX = PuzzleSize*FigureSize, sizeY = PuzzleSize*FigureSize;

void uncover(int x, int y){	// 0 - empty field
//	std::cout << x << " " << y << std::endl;
	if (backGrid[y - 1][x] < MINE && showGrid[y - 1][x] == COVER){
		showGrid[y - 1][x] = backGrid[y - 1][x];
		if(backGrid[y - 1][x] == 0) uncover(x, y - 1);
	}
	if (backGrid[y + 1][x] < MINE && showGrid[y + 1][x] == COVER) {
		showGrid[y + 1][x] = backGrid[y + 1][x];
		if (backGrid[y + 1][x] == 0) uncover(x, y + 1);
	}
	if (backGrid[y][x - 1] < MINE && showGrid[y][x - 1] == COVER) {
		showGrid[y][x - 1] = backGrid[y][x - 1];
		if(backGrid[y][x - 1] == 0) uncover(x - 1, y);
	}
	if (backGrid[y][x + 1] < MINE && showGrid[y][x + 1] == COVER) {
		showGrid[y][x + 1] = backGrid[y][x + 1];
		if (backGrid[y][x + 1] == 0) uncover(x + 1, y);
	}
}

int main(int argc, char ** argv) {
	srand(time(0));
	// Error checks
	std::cout << "SDL_Init\n";
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *win = SDL_CreateWindow("Minesweeper", 100, 100, sizeX, sizeY, 0);
	if (win == nullptr) {
		std::cout << "SDL_CreateWindow error\n";
		return 1;
	}
	SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == nullptr) {
		std::cout << "SDL_CreateRenderer error\n";
		return 1;
	}
	// Load bitmap
	SDL_Surface * bitmapSurface = SDL_LoadBMP("img/tiles.bmp");
	// Create textures from bitmap 
	SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, bitmapSurface);
	if (texture == nullptr) 
		std::cout << " SDL_CreateTextureFromSurface error\n";
	else
		std::cout << " SDL_CreateTextureFromSurface OK\n";
	SDL_FreeSurface(bitmapSurface);

	std::map<int, SDL_Rect> Figures;
	// Load 12 figures for mines: 0..8 numbers, 9 mine, 10 empty, 11 flag
	for (int i = 0; i < 12; i++)
		Figures[i] = SDL_Rect{ i*FigureSize, 0, FigureSize, FigureSize };
	// Init show grid & place mines on backgrid
	for (int i = 1; i <= PuzzleSize; i++)
		for (int j = 1; j <= PuzzleSize; j++) {
			showGrid[j][i] = COVER;
			// place mine
			if (rand() % 5 == 0)  
				backGrid[i][j] = MINE;
			else 
				backGrid[i][j] = 0;
		}
	// init numbers 
	for (int i = 1; i <= PuzzleSize; i++)
		for (int j = 1; j <= PuzzleSize; j++) {
			int n = 0;
			if (backGrid[i][j] == MINE) continue;
			if (backGrid[i + 1][j] == MINE) n++;
			if (backGrid[i][j + 1] == MINE) n++;
			if (backGrid[i - 1][j] == MINE) n++;
			if (backGrid[i][j - 1] == MINE) n++;
			if (backGrid[i + 1][j + 1] == MINE) n++;
			if (backGrid[i - 1][j - 1] == MINE) n++;
			if (backGrid[i - 1][j + 1] == MINE) n++;
			if (backGrid[i + 1][j - 1] == MINE) n++;
			backGrid[i][j] = n;
		}
	while (1) {
		SDL_Event e;
		if (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				break;
			}
			/* Mouse button down */
			if (e.type == SDL_MOUSEBUTTONDOWN) {
				SDL_Point mousePos;
				mousePos.x = e.motion.x / FigureSize + 1;
				mousePos.y = e.motion.y / FigureSize + 1;
				std::cout << mousePos.x << "," << mousePos.y << "->" << backGrid[mousePos.y][mousePos.x] << std::endl;
				if (e.button.button == SDL_BUTTON_LEFT) {
					if (backGrid[mousePos.y][mousePos.x] != MINE) {
						showGrid[mousePos.y][mousePos.x] = backGrid[mousePos.y][mousePos.x];
						uncover(mousePos.x ,mousePos.y);
					}
					else{
						SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Game over", "You found a mine.", nullptr);
						// uncover all fields
						for (int i = 1; i <= PuzzleSize; i++)
							for (int j = 1; j <= PuzzleSize; j++) 
								showGrid[j][i] = backGrid[j][i];
						//break;
					}
				}
				if (e.button.button == SDL_BUTTON_RIGHT) 
					showGrid[mousePos.y][mousePos.x] = FLAG;
			}
		}
		SDL_RenderClear(renderer);
		// figures texture
		for (int i = 1; i <= PuzzleSize; i++)
			for (int j = 1; j <= PuzzleSize; j++) {
				auto it = Figures.find(showGrid[j][i]);
				if (it != Figures.end()) {
					SDL_Rect DestR;
					DestR.x = (i-1)*FigureSize;
					DestR.y = (j-1)*FigureSize;
					DestR.h = FigureSize;
					DestR.w = FigureSize;
					SDL_RenderCopy(renderer, texture, &it->second, &DestR);
				}
			}
		SDL_RenderPresent(renderer);
	}
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(win);
	SDL_Quit();
	std::cout << "SDL_Quit\n";
	return 0;
}