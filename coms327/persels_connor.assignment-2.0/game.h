#ifndef GAME_H
# define GAME_H

typedef struct tile {
	int adjacent_bombs;
	int is_bomb;
	int revealed;
} tile_t;

#endif