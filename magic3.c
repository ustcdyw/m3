/*
 * Author: dyw
 * Date: Sun Jun 7 CST 2015
 *
 */
#include <curses.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <strings.h>
#include <string.h>

#define TOP 	(LINES/2 - 8)
#define LEFT	(COLS/2 - 14)

#define LEFT_STEP 6
#define TOP_STEP 3

#define	D_LEFT	1
#define	D_RIGHT	2
#define	D_UP	4
#define	D_DOWN	8

static int lines;
static int cols;
static int prenum[4][4];
static int pre_randnum;
static int num[4][4];
static int next_randnum;
static int score;
static int difficulty = 2;

static int dyeffect[4][4];
static int dyundo;
static int dyspeed = 4;
#define DELAYBASE 5000

static int moved;
#ifdef STATISTICS
static int numstat[3] = {0};
static int steps;
static char numstatstr[10];
#endif

static void
getscore()
{
	int i, j;
	int tmp, cbscore;
	score = 0;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++) {
			tmp = num[i][j]/3;
			if (tmp == 0)
				continue;
			cbscore = 1;
			while (tmp != 0) {
				tmp >>= 1;
				cbscore *= 3;
			}
			score += cbscore;
		}
#ifdef DEBUG
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			printf(" %3d ", num[i][j]);
		}
		printf("\n\r");
	}
	printf("\n\r");
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			printf(" %3d ", prenum[i][j]);
		}
		printf("\n\r");
	}
	printf("\n\r");
#endif
}
static char
getkey()
{
	char ch = wgetch(stdscr);
	if (ch == 0x1b) {
		ch = wgetch(stdscr);
		if (ch == '[') {
			ch = wgetch(stdscr);
			switch(ch) {
			case 'A': ch = 'k'; break;
			case 'B': ch = 'j'; break;
			case 'C': ch = 'l'; break;
			case 'D': ch = 'h'; break;
			default: ch = 0;
			}
		} else
			ch = 0;
	}
	return ch;
}
static void
draw_board()
{
	int i;

	for (i = 0; i < 4; i++) {
		mvaddstr(TOP+0+4*i, LEFT, "+------+------+------+------+");
		mvaddstr(TOP+1+4*i, LEFT, "|      |      |      |      |");
		mvaddstr(TOP+2+4*i, LEFT, "|      |      |      |      |");
		mvaddstr(TOP+3+4*i, LEFT, "|      |      |      |      |");
	}
	mvaddstr(TOP+4*4, LEFT, "+------+------+------+------+");
}
static void
dymove()
{

	int i, j, k, movestep = 0, movedir;
	char numstr[10];
#ifdef DEBUG
	move(0, 20);
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			printf(" %3d ", dyeffect[i][j]);
		}
		printf("\n\r");
	}
	printf("\n\r");
	move(0, 0);
#endif
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j ++) {
			if (dyeffect[i][j] == D_LEFT) {
				movestep = LEFT_STEP;
				movedir = D_LEFT;
				goto out;
			} else if (dyeffect[i][j] == D_RIGHT) {
				movestep = LEFT_STEP;
				movedir = D_RIGHT;
				goto out;
			} else if (dyeffect[i][j] == D_UP) {
				movestep = TOP_STEP;
				movedir = D_UP;
				goto out;
			} else if (dyeffect[i][j] == D_DOWN) {
				movestep = TOP_STEP;
				movedir = D_DOWN;
				goto out;
			}
		}
	return;
out:
	for (k = 1; k <= movestep; k++) {
		if (dyundo == 1) {
			k = movestep - k + 1;
		}
		switch (movedir) {
		case D_LEFT:
			for (i = 0; i < 4; i++) {
				for (j = 0; j < 4; j++) {
					if (prenum[i][j] == 0)
						continue;
					sprintf(numstr, "%d", prenum[i][j]);
					if (prenum[i][j] < 3)
						attrset(COLOR_PAIR(prenum[i][j]) | A_BOLD);
					if (prenum[i][j] >= 3)
						attrset(COLOR_PAIR(3) | A_BOLD);
					if (dyeffect[i][j] == 0) {
						mvaddstr(TOP+1+i*4, LEFT+1+j*7, "      ");
						mvaddstr(TOP+2+i*4, LEFT+1+j*7, "      ");
						mvaddstr(TOP+3+i*4, LEFT+1+j*7, "      ");
						mvaddstr(TOP+2+i*4, LEFT+2+j*7, numstr);
						continue;
					}
					mvaddstr(TOP+1+i*4, LEFT+1+j*7 - k, "      ");
					mvaddstr(TOP+2+i*4, LEFT+1+j*7 - k, "      ");
					mvaddstr(TOP+3+i*4, LEFT+1+j*7 - k, "      ");
					mvaddstr(TOP+2+i*4, LEFT+2+j*7 - k, numstr);
					/* background */
					attrset(A_NORMAL);
					if (k <= movestep) {
						if (dyundo == 0) {
							mvaddstr(TOP+1+i*4, LEFT+1+j*7 - k + 6, " ");
							mvaddstr(TOP+2+i*4, LEFT+1+j*7 - k + 6, " ");
							mvaddstr(TOP+3+i*4, LEFT+1+j*7 - k + 6, " ");
						} else {
							mvaddstr(TOP+1+i*4, LEFT+1+j*7 - k - 1, " ");
							mvaddstr(TOP+2+i*4, LEFT+1+j*7 - k - 1, " ");
							mvaddstr(TOP+3+i*4, LEFT+1+j*7 - k - 1, " ");
						}
					}
#if LEFT_STEP==7
					else {
						mvaddstr(TOP+1+i*4, LEFT+1+j*7 - k + 6, "|");
						mvaddstr(TOP+2+i*4, LEFT+1+j*7 - k + 6, "|");
						mvaddstr(TOP+3+i*4, LEFT+1+j*7 - k + 6, "|");
					}
#endif
					if (prenum[i][j] < 3)
						attrset(COLOR_PAIR(prenum[i][j]) | A_BOLD);
					if (prenum[i][j] >= 3)
						attrset(COLOR_PAIR(3) | A_BOLD);
				}
			}
			break;
		case D_RIGHT:
			for (i = 0; i < 4; i++) {
				for (j = 3; j >= 0; j--) {
					if (prenum[i][j] == 0)
						continue;
					sprintf(numstr, "%d", prenum[i][j]);
					if (prenum[i][j] < 3)
						attrset(COLOR_PAIR(prenum[i][j]) | A_BOLD);
					if (prenum[i][j] >= 3)
						attrset(COLOR_PAIR(3) | A_BOLD);
					if (dyeffect[i][j] == 0) {
						mvaddstr(TOP+1+i*4, LEFT+1+j*7, "      ");
						mvaddstr(TOP+2+i*4, LEFT+1+j*7, "      ");
						mvaddstr(TOP+3+i*4, LEFT+1+j*7, "      ");
						mvaddstr(TOP+2+i*4, LEFT+2+j*7, numstr);
						continue;
					}
					mvaddstr(TOP+1+i*4, LEFT+1+j*7 + k, "      ");
					mvaddstr(TOP+2+i*4, LEFT+1+j*7 + k, "      ");
					mvaddstr(TOP+3+i*4, LEFT+1+j*7 + k, "      ");
					mvaddstr(TOP+2+i*4, LEFT+2+j*7 + k, numstr);
					attrset(A_NORMAL);
					if (k <= movestep) {
						if (dyundo == 0) {
							mvaddstr(TOP+1+i*4, LEFT+1+j*7 + k - 1, " ");
							mvaddstr(TOP+2+i*4, LEFT+1+j*7 + k - 1, " ");
							mvaddstr(TOP+3+i*4, LEFT+1+j*7 + k - 1, " ");
						} else {
							mvaddstr(TOP+1+i*4, LEFT+1+j*7 + k + 6, " ");
							mvaddstr(TOP+2+i*4, LEFT+1+j*7 + k + 6, " ");
							mvaddstr(TOP+3+i*4, LEFT+1+j*7 + k + 6, " ");
						}
					}
#if LEFT_STEP==7
					else {
						mvaddstr(TOP+1+i*4, LEFT+1+j*7 + k - 1, "|");
						mvaddstr(TOP+2+i*4, LEFT+1+j*7 + k - 1, "|");
						mvaddstr(TOP+3+i*4, LEFT+1+j*7 + k - 1, "|");
					}
#endif
					if (prenum[i][j] < 3)
						attrset(COLOR_PAIR(prenum[i][j]) | A_BOLD);
					if (prenum[i][j] >= 3)
						attrset(COLOR_PAIR(3) | A_BOLD);
				}
			}
			break;
		case D_UP:
			for (i = 0; i < 4; i++) {
				for (j = 0; j < 4; j++) {
					if (prenum[i][j] == 0)
						continue;
					sprintf(numstr, "%d", prenum[i][j]);
					if (prenum[i][j] < 3)
						attrset(COLOR_PAIR(prenum[i][j]) | A_BOLD);
					if (prenum[i][j] >= 3)
						attrset(COLOR_PAIR(3) | A_BOLD);
					if (dyeffect[i][j] == 0) {
						mvaddstr(TOP+1+i*4, LEFT+1+j*7, "      ");
						mvaddstr(TOP+2+i*4, LEFT+1+j*7, "      ");
						mvaddstr(TOP+3+i*4, LEFT+1+j*7, "      ");
						mvaddstr(TOP+2+i*4, LEFT+2+j*7, numstr);
						continue;
					}
					mvaddstr(TOP+1+i*4 - k, LEFT+1+j*7, "      ");
					mvaddstr(TOP+2+i*4 - k, LEFT+1+j*7, "      ");
					mvaddstr(TOP+3+i*4 - k, LEFT+1+j*7, "      ");
					mvaddstr(TOP+2+i*4 - k, LEFT+2+j*7, numstr);
					attrset(A_NORMAL);
					if (k <= movestep) {
						if (dyundo == 0)
							mvaddstr(TOP+1+i*4 - k + 3, LEFT+1+j*7, "      ");
						else
							mvaddstr(TOP+1+i*4 - k - 1, LEFT+1+j*7, "      ");
					}
#if LEFT_STEP==7
					else {
						mvaddstr(TOP+1+i*4 - k + 3, LEFT+1+j*7, "------");
					}
#endif
					if (prenum[i][j] < 3)
						attrset(COLOR_PAIR(prenum[i][j]) | A_BOLD);
					if (prenum[i][j] >= 3)
						attrset(COLOR_PAIR(3) | A_BOLD);
				}
			}
			break;
		case D_DOWN:
			for (i = 3; i >= 0; i--) {
				for (j = 0; j < 4; j++) {
					if (prenum[i][j] == 0)
						continue;
					sprintf(numstr, "%d", prenum[i][j]);
					if (prenum[i][j] < 3)
						attrset(COLOR_PAIR(prenum[i][j]) | A_BOLD);
					if (prenum[i][j] >= 3)
						attrset(COLOR_PAIR(3) | A_BOLD);
					if (dyeffect[i][j] == 0) {
						mvaddstr(TOP+1+i*4, LEFT+1+j*7, "      ");
						mvaddstr(TOP+2+i*4, LEFT+1+j*7, "      ");
						mvaddstr(TOP+3+i*4, LEFT+1+j*7, "      ");
						mvaddstr(TOP+2+i*4, LEFT+2+j*7, numstr);
						continue;
					}
					mvaddstr(TOP+1+i*4 + k, LEFT+1+j*7, "      ");
					mvaddstr(TOP+2+i*4 + k, LEFT+1+j*7, "      ");
					mvaddstr(TOP+3+i*4 + k, LEFT+1+j*7, "      ");
					mvaddstr(TOP+2+i*4 + k, LEFT+2+j*7, numstr);
					attrset(A_NORMAL);
					if (k <= movestep) {
						if (dyundo == 0)
							mvaddstr(TOP+1+i*4 + k - 1, LEFT+1+j*7, "      ");
						else
							mvaddstr(TOP+1+i*4 + k + 3, LEFT+1+j*7, "      ");
					}
#if LEFT_STEP==7
					else {
						mvaddstr(TOP+1+i*4 + k - 1, LEFT+1+j*7, "------");
					}
#endif
					if (prenum[i][j] < 3)
						attrset(COLOR_PAIR(prenum[i][j]) | A_BOLD);
					if (prenum[i][j] >= 3)
						attrset(COLOR_PAIR(3) | A_BOLD);
				}
			}
			break;
		}
		move(0, 0);
		refresh();
		if (movestep == LEFT_STEP)
			usleep(dyspeed * DELAYBASE);
		else
			usleep(2 * dyspeed * DELAYBASE);
		if (dyundo == 1) {
			k = movestep - k + 1;
			if (k == movestep)
				dyundo = 0;
		}
	}
}
static void
refresh_board()
{
	int i, j;
	char numstr[20] = {0};
	if (lines != LINES || cols != COLS) {
		lines = LINES;
		cols = COLS;
		wclear(stdscr);
	}
	
	draw_board();

	/* dynamic effect */
	if (moved) {
		dymove();
	}
	attrset(A_NORMAL);
	draw_board();
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			if (num[i][j] == 0)
				continue;
			sprintf(numstr, "%d", num[i][j]);
			if (num[i][j] < 3)
				attrset(COLOR_PAIR(num[i][j]) | A_BOLD);
			if (num[i][j] >= 3)
				attrset(COLOR_PAIR(3) | A_BOLD);
			mvaddstr(TOP+1+i*4, LEFT+1+j*7, "      ");
			mvaddstr(TOP+2+i*4, LEFT+1+j*7, "      ");
			mvaddstr(TOP+3+i*4, LEFT+1+j*7, "      ");
			mvaddstr(TOP+2+i*4, LEFT+2+j*7, numstr);
		}
	}
	attrset(A_NORMAL);
	sprintf(numstr, "%d", score);
	mvaddstr(TOP - 9, LEFT + 5, "Score:");
	mvaddstr(TOP - 9, LEFT + 13, "     ");
	mvaddstr(TOP - 9, LEFT + 13, numstr);
	mvaddstr(TOP - 3, LEFT + 5, "Next:");

	mvaddstr(TOP - 6, LEFT + 5, "Mode:     ");
	switch (difficulty) {
	case 1:
		mvaddstr(TOP - 6, LEFT + 13, "Easy    (e/d)");
		break;
	case 2:
		mvaddstr(TOP - 6, LEFT + 13, "Normal  (e/d)");
		break;
	case 3:
		mvaddstr(TOP - 6, LEFT + 13, "Hard    (e/d)");
		break;
	case 4:
		mvaddstr(TOP - 6, LEFT + 13, "Serious (e/d)");
		break;
	}
	mvaddstr(TOP - 7, LEFT + 5, "Speed:    ");
	sprintf(numstr, "%d       (w/s)", 9 - dyspeed);
	mvaddstr(TOP - 7, LEFT + 13, numstr);

	attrset(COLOR_PAIR(next_randnum) | A_BOLD);
	mvaddstr(TOP - 4, LEFT + 13, "      ");
	mvaddstr(TOP - 3, LEFT + 13, "      ");
	mvaddstr(TOP - 2, LEFT + 13, "      ");
	sprintf(numstr, "%d", next_randnum);
	mvaddstr(TOP - 3, LEFT + 15, numstr);
	attrset(A_NORMAL);
	move(0, 0);
	refresh();
	//wrefresh(stdscr);
}
static int
getnextnum()
{
	// adjustment next random number
	int next, i, j;
	int num1 = 0, num2 = 0;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++) {
			if (num[i][j] == 1)
				num1++;
			else if (num[i][j] == 2)
				num2++;
		}
	if (num2 - num1 >= difficulty)
		next = 1;
	else if (num1 - num2 >= difficulty)
		next = 2;
	else
		next = 1 + rand() % 3;
	return next;
}
static void
newrand(char ch)
{
	int k, j, randnum, choose, zeronum = 0;
// statistics of random number
#ifdef STATISTICS
	numstat[next_randnum - 1]++;
#endif
	randnum = next_randnum;
	switch(ch) {
	case 'h': // left
		j = 3;
		goto amove;
	case 'l': // right
		j = 0;
amove:
		for (k = 0; k < 4; k++)
			if (num[k][j] == 0)
				zeronum++;
		assert(zeronum != 0);
		choose = (zeronum == 1) ? 0 : rand() % zeronum;
		for (k = 0; k < 4; k++)
			if (num[k][j] == 0)
				if (choose-- == 0) {
					num[k][j] = randnum;
					break;
				}
		break;
	case 'k':
		j = 3;
		goto vmove;
	case 'j':
		j = 0;
vmove:
		for (k = 0; k < 4; k++)
			if (num[j][k] == 0)
				zeronum++;
		assert(zeronum != 0);
		choose = (zeronum == 1) ? 0 : rand() % zeronum;
		for (k = 0; k < 4; k++)
			if (num[j][k] == 0)
				if (choose-- == 0) {
					num[j][k] = randnum;
					break;
				}
		break;
	}
	// rand adjust
	next_randnum = getnextnum();
}
static void
newmove(char ch)
{
	int i, j, k;
	int curnum, nextnum;

	/* Undo the previous move */
	int tmpnum[4][4];
	int tmp_randnum = next_randnum;
	memcpy(tmpnum, num, sizeof(num));

	bzero(dyeffect, sizeof(dyeffect));

	switch(ch) {
	case 'h': // left
		for (i = 0; i < 4; i++)
			for (j = 0; j < 3; j++) {
				curnum = num[i][j];
				nextnum = num[i][j+1];
				if (curnum == 0) {
					/* dynamic effect */
					for (k = j+1; k < 4; k++) {
						if (num[i][k] != 0) {
							dyeffect[i][k] = D_LEFT;
						}
						/*
						if (num[i][k] == 1)
							dyeffect[i][k] = 0x1;
						else if (num[i][k] == 2)
							dyeffect[i][k] = 0x2;
						else if (num[i][k] >= 3)
							dyeffect[i][k] = 0x4;
							*/
					}
					for (k = j; k < 3; k++) {
						num[i][k] = num[i][k+1];
						if (num[i][k] != 0)
							moved = 1;
					}
					num[i][3] = 0;
					break;
				} else if (curnum == nextnum && curnum > 2) {
					for (k = j+1; k < 4; k++) {
						if (num[i][k] != 0) {
							dyeffect[i][k] = D_LEFT;
						}
						/*
						if (num[i][k] == 1)
							dyeffect[i][k] = 0x1;
						else if (num[i][k] == 2)
							dyeffect[i][k] = 0x2;
						else if (num[i][k] >= 3)
							dyeffect[i][k] = 0x4;
							*/
					}
					num[i][j] <<= 1;
					for (k = j+1; k < 3; k++)
						num[i][k] = num[i][k+1];
					num[i][3] = 0;
					moved = 1;
					break;
				} else if ((curnum == 1 && nextnum == 2 )
					   || (curnum == 2 && nextnum == 1)) {
					for (k = j+1; k < 4; k++) {
						if (num[i][k] != 0) {
							dyeffect[i][k] = D_LEFT;
						}
						/*
						if (num[i][k] == 1)
							dyeffect[i][k] = 0x1;
						else if (num[i][k] == 2)
							dyeffect[i][k] = 0x2;
						else if (num[i][k] >= 3)
							dyeffect[i][k] = 0x4;
							*/
					}
					num[i][j] = 3;
					for (k = j+1; k < 3; k++)
						num[i][k] = num[i][k+1];
					num[i][3] = 0;
					moved = 1;
					break;
				}
			}
		break;
	case 'l': // right
		for (i = 0; i < 4; i++)
			for (j = 3; j > 0; j--) {
				curnum = num[i][j];
				nextnum = num[i][j-1];
				if (curnum == 0) {
					for (k = j-1; k >= 0; k--) {
						if (num[i][k] != 0) {
							dyeffect[i][k] = D_RIGHT;
						}
						/*
						if (num[i][k] == 1)
							dyeffect[i][k] = 0x10;
						else if (num[i][k] == 2)
							dyeffect[i][k] = 0x20;
						else if (num[i][k] >= 3)
							dyeffect[i][k] = 0x40;
							*/
					}
					for (k = j; k > 0; k--) {
						num[i][k] = num[i][k-1];
						if (num[i][k] != 0)
							moved = 1;
					}
					num[i][0] = 0;
					break;
				} else if (curnum == nextnum && curnum > 2) {
					for (k = j-1; k >= 0; k--) {
						if (num[i][k] != 0) {
							dyeffect[i][k] = D_RIGHT;
						}
						/*
						if (num[i][k] == 1)
							dyeffect[i][k] = 0x10;
						else if (num[i][k] == 2)
							dyeffect[i][k] = 0x20;
						else if (num[i][k] >= 3)
							dyeffect[i][k] = 0x40;
							*/
					}
					num[i][j] <<= 1;
					for (k = j-1; k > 0; k--)
						num[i][k] = num[i][k-1];
					num[i][0] = 0;
					moved = 1;
					break;
				} else if ((curnum == 1 && nextnum == 2 )
					   || (curnum == 2 && nextnum == 1)) {
					for (k = j-1; k >= 0; k--) {
						if (num[i][k] != 0) {
							dyeffect[i][k] = D_RIGHT;
						}
						/*
						if (num[i][k] == 1)
							dyeffect[i][k] = 0x10;
						else if (num[i][k] == 2)
							dyeffect[i][k] = 0x20;
						else if (num[i][k] >= 3)
							dyeffect[i][k] = 0x40;
							*/
					}
					num[i][j] = 3;
					for (k = j-1; k > 0; k--)
						num[i][k] = num[i][k-1];
					num[i][0] = 0;
					moved = 1;
					break;
				}
			}
		break;
	case 'k': // up
		for (j = 0; j < 4; j++)
			for (i = 0; i < 3; i++) {
				curnum = num[i][j];
				nextnum = num[i+1][j];
				if (curnum == 0) {
					for (k = i+1; k < 4; k++) {
						if (num[k][j] != 0) {
							dyeffect[k][j] = D_UP;
						}
						/*
						if (num[k][j] == 1)
							dyeffect[k][j] = 0x100;
						else if (num[k][j] == 2)
							dyeffect[k][j] = 0x200;
						else if (num[k][j] >= 3)
							dyeffect[k][j] = 0x400;
							*/
					}
					for (k = i; k < 3; k++) {
						num[k][j] = num[k+1][j];
						if (num[k][j] != 0)
							moved = 1;
					}
					num[3][j] = 0;
					break;
				} else if (curnum == nextnum && curnum > 2) {
					for (k = i+1; k < 4; k++) {
						if (num[k][j] != 0) {
							dyeffect[k][j] = D_UP;
						}
						/*
						if (num[k][j] == 1)
							dyeffect[k][j] = 0x100;
						else if (num[k][j] == 2)
							dyeffect[k][j] = 0x200;
						else if (num[k][j] >= 3)
							dyeffect[k][j] = 0x400;
							*/
					}
					num[i][j] <<= 1;
					for (k = i+1; k < 3; k++)
						num[k][j] = num[k+1][j];
					num[3][j] = 0;
					moved = 1;
					break;
				} else if ((curnum == 1 && nextnum == 2 )
					   || (curnum == 2 && nextnum ==1)) {
					for (k = i+1; k < 4; k++) {
						if (num[k][j] != 0) {
							dyeffect[k][j] = D_UP;
						}
						/*
						if (num[k][j] == 1)
							dyeffect[k][j] = 0x100;
						else if (num[k][j] == 2)
							dyeffect[k][j] = 0x200;
						else if (num[k][j] >= 3)
							dyeffect[k][j] = 0x400;
							*/
					}
					num[i][j] = 3;
					for (k = i+1; k < 3; k++)
						num[k][j] = num[k+1][j];
					num[3][j] = 0;
					moved = 1;
					break;
				}
			}
		break;
	case 'j': //down
		for (j = 0; j < 4; j++)
			for (i = 3; i > 0; i--) {
				curnum = num[i][j];
				nextnum = num[i-1][j];
				if (curnum == 0) {
					for (k = i-1; k >= 0; k--) {
						if (num[k][j] != 0) {
							dyeffect[k][j] = D_DOWN;
							move(0, 0);
						}
						/*
						if (num[k][j] == 1)
							dyeffect[k][j] = 0x1000;
						else if (num[k][j] == 2)
							dyeffect[k][j] = 0x2000;
						else if (num[k][j] >= 3)
							dyeffect[k][j] = 0x4000;
							*/
					}
					for (k = i; k > 0; k--) {
						num[k][j] = num[k-1][j];
						if (num[k][j] != 0)
							moved = 1;
					}
					num[0][j] = 0;
					break;
				} else if (curnum == nextnum && curnum > 2) {
					for (k = i-1; k >= 0; k--) {
						if (num[k][j] != 0) {
							dyeffect[k][j] = D_DOWN;
						}
						/*
						if (num[k][j] == 1)
							dyeffect[k][j] = 0x1000;
						else if (num[k][j] == 2)
							dyeffect[k][j] = 0x2000;
						else if (num[k][j] >= 3)
							dyeffect[k][j] = 0x4000;
							*/
					}
					num[i][j] <<= 1;
					for (k = i-1; k > 0; k--)
						num[k][j] = num[k-1][j];
					num[k][j] = 0;
					moved = 1;
					break;
				} else if ((curnum == 1 && nextnum == 2 )
					   || (curnum == 2 && nextnum ==1)) {
					for (k = i-1; k >= 0; k--) {
						if (num[k][j] != 0) {
							dyeffect[k][j] = D_DOWN;
						}
						/*
						if (num[k][j] == 1)
							dyeffect[k][j] = 0x1000;
						else if (num[k][j] == 2)
							dyeffect[k][j] = 0x2000;
						else if (num[k][j] >= 3)
							dyeffect[k][j] = 0x4000;
							*/
					}
					num[i][j] = 3;
					for (k = i-1; k > 0; k--)
						num[k][j] = num[k-1][j];
					num[k][j] = 0;
					moved = 1;
					break;
				}
			}
		break;
	}
	if (moved) {
#ifdef STATISTICS
		steps++;
#endif
		/* Record it when it really moves. */
		memcpy(prenum, tmpnum, sizeof(prenum));
		pre_randnum = tmp_randnum;

		newrand(ch);
		getscore();
	}
}
static int
checkadj(int i, int j)
{
	int found = 0;
	if (i == 0) {
		if (j > 0 && j < 3) {
			if (num[i][j] == 1) {
				if (num[i][j-1] == 2
				    || num[i+1][j] == 2
				    || num[i][j+1] == 2)
					found = 1;
			} else if (num[i][j] == 2) {
				if (num[i][j-1] == 1
				    || num[i+1][j] == 1
				    || num[i][j+1] == 1)
					found = 1;
			} else {
				if (num[i][j-1] == num[i][j]
				    || num[i+1][j] == num[i][j]
				    || num[i][j+1] == num[i][j])
					found = 1;
			}
		}
	} else if (i == 3) {
		if (j > 0 && j < 3) {
			if (num[i][j] == 1) {
				if (num[i][j-1] == 2
				    || num[i-1][j] == 2
				    || num[i][j+1] == 2)
					found = 1;
			} else if (num[i][j] == 2) {
				if (num[i][j-1] == 1
				    || num[i-1][j] == 1
				    || num[i][j+1] == 1)
					found = 1;
			} else {
				if (num[i][j-1] == num[i][j]
				    || num[i-1][j] == num[i][j]
				    || num[i][j+1] == num[i][j])
					found = 1;
			}
		}
	} else {
		if (j > 0 && j < 3) {
			if (num[i][j] == 1) {
				if (num[i][j-1] == 2
				    || num[i-1][j] == 2
				    || num[i][j+1] == 2
				    || num[i+1][j] == 2)
					found = 1;
			} else if (num[i][j] == 2) {
				if (num[i][j-1] == 1
				    || num[i-1][j] == 1
				    || num[i][j+1] == 1
				    || num[i+1][j] == 1)
					found = 1;
			} else {
				if (num[i][j-1] == num[i][j]
				    || num[i-1][j] == num[i][j]
				    || num[i][j+1] == num[i][j]
				    || num[i+1][j] == num[i][j])
					found = 1;
			}
		} else {
			if (num[i][j] + num[i-1][j] == 3
			    || num[i][j] + num[i+1][j] == 3
			    || (num[i][j] == num[i-1][j] && num[i][j] > 2)
			    || (num[i][j] == num[i+1][j] && num[i][j] > 2))
				found = 1;
		}
	}
	return found;
}
static int
gameover()
{
	int found;
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			if (num[i][j] == 0)
				return 0;

	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++) {
			found = checkadj(i, j);
			if (found)
				return 0;
		}
	return 1;
}
int
main(int argc, char *argv[])
{
	char ch;
	if (initscr() == 0) {
		fprintf(stderr, "initscr()\n");
		return 1;
	}
	srand(time(0) ^ getpid());
	next_randnum = 1 + rand() % 3;
	noecho();
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLUE);
	init_pair(2, COLOR_WHITE, COLOR_RED);
	init_pair(3, COLOR_BLACK, COLOR_WHITE);
	lines = LINES;
	cols = COLS;
newgame:
	bzero(num, sizeof(num));
	num[rand()%4][rand()%4] = 3;
//	num[1][1] = 1;
	memcpy(prenum, num, sizeof(num));
#ifdef STATISTICS
	steps = 0;
	numstat[0] = numstat[1] = numstat[2] = 0;
#endif
	wclear(stdscr);
	refresh_board();
	ch = ' ';
	while (ch != 'q' && ch != 'Q') {
		ch = getkey();
		moved = 0;
		switch(ch) {
		case 'h':
			newmove('h');
			break;
		case 'j':
			newmove('j');
			break;
		case 'k':
			newmove('k');
			break;
		case 'l':
			newmove('l');
			break;
		case 'u':
			wclear(stdscr);
			memcpy(num, prenum, sizeof(num));
			next_randnum = pre_randnum;
			dyundo = 1;
			break;
		case 'e': // increase difficulty
			if (++difficulty > 4)
				difficulty = 4;
			break;
		case 'd': // decrease difficulty
			if (--difficulty < 1)
				difficulty = 1;
			break;
		case 's': // decrease speed
			if (++dyspeed > 8)
				dyspeed = 8;
			break;
		case 'w':
			if (--dyspeed < 1)
				dyspeed = 1;
			break;
		case 'n':
		case 'N':
			goto newgame;
		}
		refresh_board();
		if (gameover()) {
			mvaddstr(TOP+18, LEFT+5, "Game over!");
			mvaddstr(TOP+19, LEFT+5, "New Game: 'N/n'");
			mvaddstr(TOP+20, LEFT+5, "Quit: 'Q/q'");
			mvaddstr(TOP+21, LEFT+5, "Undo: 'u'");
#ifdef STATISTICS
			//mvaddstr(TOP+22, LEFT+5, "Statistics:");
			mvaddstr(TOP+23, LEFT+5, "Number of 1:");
			sprintf(numstatstr, "%d", numstat[0]);
			mvaddstr(TOP+23, LEFT+18, numstatstr);
			mvaddstr(TOP+24, LEFT+5, "Number of 2:");
			sprintf(numstatstr, "%d", numstat[1]);
			mvaddstr(TOP+24, LEFT+18, numstatstr);
			mvaddstr(TOP+25, LEFT+5, "Number of 3:");
			sprintf(numstatstr, "%d", numstat[2]);
			mvaddstr(TOP+25, LEFT+18, numstatstr);
			mvaddstr(TOP+26, LEFT+5, "Steps:");
			sprintf(numstatstr, "%d", steps);
			mvaddstr(TOP+26, LEFT+18, numstatstr);
#endif
		}
	}
	endwin();
	return 0;
}
