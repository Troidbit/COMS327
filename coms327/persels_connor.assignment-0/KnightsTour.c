#include <stdio.h>

int row[]={2, 1, -1, -2, -2, -1, 1, 2};
int col[]={1, 2, 2, 1, -1, -2, -2, -1};
int c=0;

int kT(int x, int y);
int rKT(int x, int y, int bd[5][5], int moves,int sol[25]);
int p(int sol[25]);
int v(int x, int y);

int main(int argc, char *argv[])
{
	int i,j;
	for (i=0; i<5; i++) {
		for (j=0; j<5; j++) {
			kT(i,j);
		}
	}
	printf("SOLUTIONS: %d\n",c);
	return 0;
}

int kT(int x, int y) {
	int bd[5][5]; 
	int sol[25];
	int i,j;

	for(i=0; i<5; i++) {
		for (j=0; j<5; j++) {
			bd[i][j] = 0;
		}
	}
	bd[x][y]=1;
	rKT(x,y,bd,1,sol);
	return 0;
}

int rKT(int x, int y, int bd[5][5], int moves, int sol[25]) {
	int k, newX, newY;

	sol[moves-1]=((y*5)+x);
	if (moves == 25) {
		c++;
		p(sol);
		return 0;
	}

	for(k=0; k<8; k++) {
		newX = x + row[k];
		newY = y + col[k];

		if (v(newX,newY)==0 && bd[newX][newY]==0) {
			bd[newX][newY]=1;

			rKT(newX,newY,bd,moves+1,sol);

			bd[newX][newY] = 0;
		}
	}
	return 1;
}

int p(int sol[25]) {
	int i;
	for (i=0; i<25; i++) {
		printf("%d, ",sol[i]);
	}
	printf("\n");
	return 0;
}

int v(int x, int y) {
	if (x>=0 && x<5 && y>=0 && y<5) {
		return 0;
	}
	return 1;
}
		




