/*
 * pacman.h
 */

#ifndef INC_PACMAN_H_
#define INC_PACMAN_H_

/* Includes */
#include <stdint.h>

/* Constants */
#define MAZE_COLUMN_N      		10
#define MAZE_ROW_N         		14
#define MAZE_CELL_WIDTH    		20

#define MAZE_TOP_BORDER			20
#define MAZE_BOTTOM_BORDER 		220
#define MAZE_LEFT_BORDER		20
#define MAZE_RIGHT_BORDER		220

#define BACKGROUND_COLOR		BLACK
#define PACMAN_COLOR			YELLOW

#define GHOST_COLOR_0			0xf800
#define GHOST_COLOR_1			0xfdbf
#define GHOST_COLOR_2			0x07ff
#define GHOST_COLOR_3			GREEN//0xfdaa

#define PAC_DOTS_COLOR			BRRED
#define WALL_COLOR				BLUE

#define TOTAL_SCORE				139

// 1 uint ~ 50 ms
#define SPEED_GHOST				10

/* Functions */
void game_init(void);
void game_process(void);

#endif /* INC_BUTTON_H_ */
