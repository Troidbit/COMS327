#include  <stdio.h>

void draw(char *map[80][24]) {
	int j,i;

	for (j=0; j<24; j++) {
		for (i=0; i<80; i++) {
		       printf("%s",map[i][j]);
		       if (i == 79) {     
			      printf("\n");
		       }
		}
	}	
}

typedef enum Terrain {
	DEBUG,
	char ROCK = "%",
	char PATH = "#",
	char GRASS = ".",
	char POKEMON_MART = "M",
	char POKEMON_CENTER = "C",
} Terrain;

void generate() {
	int map[80][24];
	int j,i;

	for (j=0; j<24; j++) {
		for (i=0; i<80; i++) {
			if (j==0 || j==20 || (i==0 && j<=20) || (i==79 && j<=20)) {
				map[i][j] = type_t.ROCK;
			} else if (j<=20) {
				map[i][j] = type_t.GRASS;
			} else {
				map[i][j] = " ";
			}
			
		}
	}

	draw(map);
}

int main() {

	generate();
	return 0;
}
