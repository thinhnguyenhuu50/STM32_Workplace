/*
 * pacman.c
 */

/* Includes ------------------------------------------------------------------*/
#include "pacman.h"
#include "button.h"
#include "lcd.h"
#include "led_7seg.h"

#include <stdlib.h>
#include <time.h>

/* Enums ---------------------------------------------------------------------*/

typedef enum DIRECTION {
	UP, DOWN, LEFT, RIGHT, STOP
} E_DIRECTION;

/* Struct --------------------------------------------------------------------*/
typedef struct CELL {
	uint8_t is_pac_dot;
} S_CELL;

typedef struct MAZE {
	S_CELL cells[MAZE_COLUMN_N][MAZE_ROW_N];
} S_MAZE;

typedef struct GHOST {
	uint8_t i, j;
	uint8_t i_pre, j_pre;
	E_DIRECTION direction;
} S_GHOST;

typedef struct PACMAN {
	uint8_t i, j;
	uint8_t i_pre, j_pre;
	E_DIRECTION direction;
	int score;
} S_PACMAN;

/* Private Objects -----------------------------------------------------------*/
// Pac-Man object
S_PACMAN pacman;
void pacman_draw(uint8_t i, uint8_t j, uint16_t color);
void pacman_direction_process(void);
void pacman_moving_process(void);

// Ghost object
S_GHOST ghost[4];
void ghost_draw(uint8_t i, uint8_t j, uint16_t color);
void ghost_direction_process(void);
void ghost_moving_process(void);

// Maze object
S_MAZE maze;
void pac_dot_draw(uint8_t i, uint8_t j, uint16_t color);

// Game Engine object
void game_draw(void);
uint8_t game_handler(void);

/* Declare Private Support Functions -----------------------------------------*/
uint8_t is_button_up(void);
uint8_t is_button_down(void);
uint8_t is_button_left(void);
uint8_t is_button_right(void);

/* Public Functions ----------------------------------------------------------*/
/**
 * @brief  	Init Pac-Man game
 * @param  	None
 * @note  	Call when you want to init game
 * @retval 	None
 */
void game_init(void) {
	/*
	 * DONE (can be modified)
	 *
	 * 1. Draw a frame for the maze
	 */
	lcd_clear(BACKGROUND_COLOR);
	//lcd_draw_rectangle(MAZE_TOP_BORDER, MAZE_LEFT_BORDER, MAZE_BOTTOM_BORDER, MAZE_RIGHT_BORDER, BLACK);
	/*
	 * TO DO
	 *
	 * 2. When the game starts, all tiles in the 10x10 maze will have one pac dot available, except Pac-man's tile.
	 * - Firstly, you have to assign suitable values to maze.cells[][].is_pac_dot.
	 * - Then, draw all pac dots on the maze.
	 */
	// Pacman starts initially at (0,0)
	// Set 1 to all cells which have available pac dot, otherwise set 0
	for (unsigned char i = 0; i <= MAZE_ROW_N - 1; i++){
		for (unsigned char j = 0; j <= MAZE_COLUMN_N - 1; j++){
			maze.cells[i][j].is_pac_dot = 1;
			pac_dot_draw(1.5*MAZE_CELL_WIDTH + i*20, 1.5*MAZE_CELL_WIDTH + j*20, PAC_DOTS_COLOR);
		}
	}
	maze.cells[0][0].is_pac_dot = 0;
	led_7seg_set_digit(0, 0, 0);
	led_7seg_set_digit(0, 1, 0);
	/*
	 * TO DO
	 *
	 * 3. Init Pac-Man object.
	 * - Firstly, you have to initialize default values for the pacman object.
	 * - Then, draw Pac-Man in the first position.
	 * - Remember that reset maze.cells[][] at pacman's location.
	 */
	pacman.direction = STOP;
	pacman.i = 0; pacman.j = 0;
	pacman.score = 0;
	pacman_draw(1.5*MAZE_CELL_WIDTH, 1.5*MAZE_CELL_WIDTH, PACMAN_COLOR);
	/*
	 * TO DO
	 *
	 * 4. Init Ghost object.
	 * - Firstly, you have to initialize default values for the ghost object.
	 * - Then, draw ghost in the first position.
	 */
	srand(HAL_GetTick());
	for (uint8_t i = 0; i < 4; i++){
	ghost[i].direction = STOP;
	ghost[i].i = 9; ghost[i].j = 9;
	ghost_draw(1.5*MAZE_CELL_WIDTH + 20*ghost[i].i, 1.5*MAZE_CELL_WIDTH + 20*ghost[i].j, GHOST_COLOR);
	}
}

/**
 * @brief  	Process game
 * @param  	None
 * @note  	Call in loop (main)
 * @retval 	None
 */
void game_process(void) {
	// timer
	static uint8_t time_ghost_move = 0;
	time_ghost_move = (time_ghost_move + 1) % SPEED_GHOST;
	if (game_handler() == 1){
		pacman_direction_process();
		if (time_ghost_move == 0){
			ghost_direction_process();
			ghost_moving_process();
			pacman_moving_process();
			}
		game_draw();
	}
}

/* Private Functions ---------------------------------------------------------*/
void game_draw(void) {
	/*
	 * TO DO
	 *
	 * Draw Pac-Man, Ghost, and Pac Dots.
	 *
	 * Hint: Remember to delete the object in the previous position, before drawing the new one.
	 */
	// PACMAN
	if (pacman.direction != STOP){
		pacman_draw(1.5*MAZE_CELL_WIDTH + pacman.i_pre*MAZE_CELL_WIDTH, 1.5*MAZE_CELL_WIDTH + pacman.j_pre*MAZE_CELL_WIDTH, BACKGROUND_COLOR);
		pacman_draw(1.5*MAZE_CELL_WIDTH + pacman.i*MAZE_CELL_WIDTH, 1.5*MAZE_CELL_WIDTH + pacman.j*MAZE_CELL_WIDTH, PACMAN_COLOR);
	}
	// GHOST
	for (uint8_t i = 0; i < 4; i++){
	if (ghost[i].direction != STOP){
		if (!(pacman.i == ghost[i].i_pre && pacman.j == ghost[i].j_pre)){
			ghost_draw(1.5*MAZE_CELL_WIDTH + ghost[i].i_pre*MAZE_CELL_WIDTH, 1.5*MAZE_CELL_WIDTH + ghost[i].j_pre*MAZE_CELL_WIDTH, BACKGROUND_COLOR);
		}
		if (maze.cells[ghost[i].i_pre][ghost[i].j_pre].is_pac_dot == 1){
			pac_dot_draw(1.5*MAZE_CELL_WIDTH + ghost[i].i_pre*MAZE_CELL_WIDTH, 1.5*MAZE_CELL_WIDTH + ghost[i].j_pre*MAZE_CELL_WIDTH, PAC_DOTS_COLOR);
		}
		ghost_draw(1.5*MAZE_CELL_WIDTH + ghost[i].i*MAZE_CELL_WIDTH, 1.5*MAZE_CELL_WIDTH + ghost[i].j*MAZE_CELL_WIDTH, GHOST_COLOR);
	}
	}
	led_7seg_set_digit(pacman.score % 10, 3, 0);
	led_7seg_set_digit(pacman.score/10, 2, 0);
}

uint8_t game_handler(void) {
	/*
	 * TO DO
	 *
	 * 1. Check the loss condition, show something, and restart the game.
	 * 2. Check the win condition, show something, and restart the game.
	 * 3. Check if Pac-Man has won any dots or not, then update the score.
	 */
	// timer
	static uint8_t time_led = 0;
	static uint8_t time_show = 0;

	static uint8_t update_led = 0;
	time_led = (time_led + 1) % 20;
	if (maze.cells[pacman.i][pacman.j].is_pac_dot == 1){
			maze.cells[pacman.i][pacman.j].is_pac_dot = 0;
			pacman.score++;
	}
	if (button_count[15] == 60){
		game_init();
	}
	// Win
	else if (pacman.score == 99){
		if (time_show == 0){
			lcd_clear(BACKGROUND_COLOR);
			lcd_show_string(50, 100, "YOU WIN", BLUE, WHITE, 32, 1);
			time_show = time_show + 1;
		}
		if (time_show == 21){
			game_init();
			time_show = 0;
			return 1;
		}
		time_show = (time_show + 1) % 22;
		return 0;
	}
	// Lose
	else for (uint8_t i = 0; i < 4; i++){
		if ((pacman.i == ghost[i].i && pacman.j == ghost[i].j) ||
			(pacman.i == ghost[i].i_pre && pacman.j == ghost[i].j_pre && ghost[i].i == pacman.i_pre && ghost[i].j_pre == pacman.j_pre)){
			if (time_show == 0){
				lcd_clear(BACKGROUND_COLOR);
				lcd_show_string(50, 100, "YOU LOSE", BLUE, WHITE, 32, 1);
				time_show = time_show + 1;
			}

		if (time_show == 21){
			game_init();
			time_show = (time_show + 1) % 22;
			return 1;
		}
		time_show = (time_show + 1) % 22;
		return 0;
		}
	}
	if (pacman.score < 0.3*TOTAL_SCORE){
		if (time_led == 0){
		HAL_GPIO_TogglePin(OUTPUT_Y0_GPIO_Port, OUTPUT_Y0_Pin);
		}
	}
	else if (pacman.score < 0.75*TOTAL_SCORE){
		if (update_led == 0){
			update_led = 1;
			time_led = 0;
		}
		if (time_led % 10 == 0){
			HAL_GPIO_TogglePin(OUTPUT_Y1_GPIO_Port, OUTPUT_Y1_Pin);
		}
	}
	else{
		if (update_led== 1){
			update_led = 0;
			time_led = 0;
		}
		if (time_led % 5 == 0){
			HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
		}
	}
	return 1;
}

void pacman_direction_process(void) {
	/*
	 * TO DO
	 *
	 * Let user use button to control Pac-Man.
	 */
	//pacman.direction = STOP;
	if (is_button_up()){
		pacman.direction = UP;
		return;
	}
	if (is_button_left()){
		pacman.direction = LEFT;
		return;
	}
	if (is_button_right()){
		pacman.direction = RIGHT;
		return;
	}
	if (is_button_down()){
		pacman.direction = DOWN;
		return;
	}
}

void pacman_moving_process(void) {
	/*
	 * TO DO
	 *
	 * Update Pac-Man's current and previous position based on current direction.
	 *
	 */
	pacman.i_pre = pacman.i;
	pacman.j_pre = pacman.j;
	switch (pacman.direction) {
	case UP:
		(pacman.i - 1 < 0) ? (pacman.i = 9) : (pacman.i--);
		break;

	case DOWN:
		(pacman.i + 1 > 9) ? (pacman.i = 0) : (pacman.i++);
		break;

	case LEFT:
		(pacman.j - 1 < 0) ? (pacman.j = 9) : (pacman.j--);
		break;

	case RIGHT:
		(pacman.j + 1 > 9) ? (pacman.j = 0) : (pacman.j++);
		break;

	case STOP:

		break;

	default:
		pacman.direction = STOP;
		break;
	}
}

void ghost_direction_process(void) {
	/*
	 * TO DO
	 *
	 * Make Ghost move randomly.
	 * Hint: Change direction randomly.
	 */
	static E_DIRECTION direction;
	if (pacman.direction != STOP){
		for (uint8_t i = 0; i < 4; i++){
			direction = rand() % 5;
			ghost[i].direction = direction;
		}
	}
}

void ghost_moving_process(void) {
	/*
	 * TO DO
	 *
	 * Update Ghost's current and previous position based on current direction.
	 */
	for (uint8_t i = 0; i < 4; i++){
		ghost[i].i_pre = ghost[i].i; ghost[i].j_pre = ghost[i].j;
		switch (ghost[i].direction) {
		case UP:
			(ghost[i].i - 1 < 0) ? (ghost[i].i = 9) : (ghost[i].i--);
			break;

		case DOWN:
			(ghost[i].i + 1 > 9) ? (ghost[i].i = 0) : (ghost[i].i++);
			break;

		case LEFT:
			(ghost[i].j - 1 < 0) ? (ghost[i].j = 9) : (ghost[i].j--);
			break;

		case RIGHT:
			(ghost[i].j + 1 > 9) ? (ghost[i].j = 0) : (ghost[i].j++);
			break;
		case STOP:

			break;

		default:
			ghost[i].direction = STOP;
			break;
		}
	}
}

void pac_dot_draw(uint8_t i, uint8_t j, uint16_t color) {
	/*
	 * TO DO
	 *
	 * Draw whatever you like
	 */
	lcd_draw_circle(j, i, color, MAZE_CELL_WIDTH / 6, 1);
}

void pacman_draw(uint8_t i, uint8_t j, uint16_t color) {
	/*
	 * TO DO
	 *
	 * Draw whatever you like
	 */
	lcd_draw_circle(j, i, color, MAZE_CELL_WIDTH / 3, 1);
}

void ghost_draw(uint8_t i, uint8_t j, uint16_t color) {
	/*
	 * TO DO
	 *
	 * Draw whatever you like
	 */
	lcd_draw_circle(j, i, color, MAZE_CELL_WIDTH / 3, 1);
}

// button has index 1
uint8_t is_button_up(void) {
	/*
	 * TO DO
	 */
	if (button_count[1] == 1 || button_count[1] % 10 == 1){
		return 1;
	}
	return 0;
}

// button has index 9
uint8_t is_button_down(void) {
	/*
	 * TO DO
	 */
	if (button_count[9] == 1 || button_count[9] % 10 == 1){
			return 1;
		}
	return 0;
}

// button has index 4
uint8_t is_button_left(void) {
	/*
	 * TO DO
	 */
	if (button_count[4] == 1 || button_count[4] % 10 == 1){
			return 1;
		}
	return 0;
}

// button has index 6
uint8_t is_button_right(void) {
	/*
	 * TO DO
	 */
	if (button_count[6] == 1 || button_count[6] % 10 == 1){
			return 1;
		}
	return 0;
}
