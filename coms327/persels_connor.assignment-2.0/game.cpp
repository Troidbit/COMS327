#include <ncurses.h>
#include <cstdlib>
#include <time.h>
#include <stdlib.h>
#include "game.h"
#include <iostream>
#include <string>
using namespace std;


int BOMB_CHANCE = 5;
tile_t map[10][10];

void terminal_setup() {
	initscr();
	cbreak();
}

void print_map() {
	printw("\n");
	int i, j;

	printw("     ");
	for (i = 0; i < 10; i++) {
		printw("%d ", i + 1);
	}
	printw("\n    ____________________\n");

	for (i = 0; i < 10; i++) {
		printw("%2d |", i+1);
		for (j = 0; j < 10; j++) {
			//if (map[i][j].revealed == 1) {
				if (map[i][j].adjacent_bombs > 0) {
					printw("%2d", map[i][j].adjacent_bombs);
				}
				else {
					printw("  ");
				}
			//}
		}
		printw("\n");
		refresh();
	}
}

void draw_map() {
	int i, j, n;
	int dX[8] = { 0, 0,-1, 1, 1, 1,-1,-1 };
	int dY[8] = { 1,-1, 0, 0, 1,-1, 1,-1 };

	//place bombs
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 10; j++) {
			map[i][j].adjacent_bombs = 0;
			if ((rand() % 100) <= BOMB_CHANCE) {
				map[i][j].is_bomb = 1;
			}
			else {
				map[i][j].is_bomb = 0;
			}
		}
	}

	//calculate adjacencies

	for (i = 0; i < 10; i++) {
		for (j = 0; j < 10; j++) {

			if (map[i][j].is_bomb != 1) {
				for (n = 0; n < 8; n++) {
					if (j + dY[n] >= 0 && j + dY[n] < 10 &&
						i + dX[n] >= 0 && i + dX[n] < 10) {

						if (map[i + dX[n]][j + dY[n]].is_bomb == 1) {
							map[i][j].adjacent_bombs += 1;
						}
					}
				}
			}
		}
	}
}

void reveal_neighbors(int x, int y,int rmap[10][10]) {
	printw("\nrevealing %d%d", x, y);
	refresh();
	getchar();
	if (map[x][y].revealed == 1 || map[x][y].is_bomb == 1 || map[x][y].adjacent_bombs > 0) {
		return;
	}
	map[x][y].revealed = true;
	int n;
	int dX[8] = { 0, 0,-1, 1, 1, 1,-1,-1 };
	int dY[8] = { 1,-1, 0, 0, 1,-1, 1,-1 };

	for (n = 0; n < 4; n++) {
		if (y + dY[n] >= 0 && y + dY[n] < 10 && x + dX[n] >= 0 && x + dX[n] < 10) {
			if (map[x + dX[n]][y + dY[n]].is_bomb == 0) {

				reveal_neighbors(x + dX[n], y + dY[n],rmap);
			}
		}
	}
}

void game_loop() {
	print_map();
	printw("\nPlease give x: ");
	refresh();
	int x = getchar()-'0';
	printw("\nPlease give y: ");
	refresh();
	int y = getchar()-'0';

	int i, j;
	if (x >= 1 && x < 11 && y >= 1 && y < 11) {
		if (map[x][y].is_bomb == 1) {
			clear();
			printw("You lose.");
			getchar();
		}
		else {
			if (map[x-1][y-1].revealed == 0) {
				map[x-1][y-1].revealed = 1;

				if (map[x - 1][y - 1].adjacent_bombs == 0) {

					int rmap[10][10];
					for (i = 0; i < 10; i++) {
						for (j = 0; j < 10; j++) {
							rmap[i][j] = 0;
						}
					}
					reveal_neighbors(x - 1, y - 1, rmap);
				}
			}
		}

	}
	else {
		printw("\n[[please try different coordinates]]");
		game_loop();
	}
}

int main(int argc, char* argv[]) {
	terminal_setup();
	srand(time(NULL));

	draw_map();

	print_map();

	//game_loop();

	getchar();
	endwin();
	return 0;
}