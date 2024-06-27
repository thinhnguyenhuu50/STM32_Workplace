/*
 * pacman.c
 */
//this is version 1
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
	uint16_t color;
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
void pacman_draw(uint16_t i, uint8_t j, uint16_t color);
void pacman_direction_process(void);
void pacman_moving_process(void);

// Ghost object
S_GHOST ghost[4];
void ghost_draw(uint16_t i, uint8_t j, uint16_t color);
void ghost_direction_process(void);
void ghost_moving_process(void);

// Maze object
uint32_t maze_bit[2*MAZE_ROW_N + 1];

void pac_dot_draw(uint16_t i, uint8_t j, uint16_t color);
void wall_draw(uint8_t i, uint8_t j);

// Game Engine object
void game_draw(void);
uint8_t game_handler(void);

/* Declare Private Support Functions -----------------------------------------*/
uint8_t is_button_up(void);
uint8_t is_button_down(void);
uint8_t is_button_left(void);
uint8_t is_button_right(void);

uint8_t is_pacdot(uint8_t, uint8_t);
uint8_t is_wall(uint8_t, uint8_t, E_DIRECTION);
uint8_t is_ghost(uint8_t, uint8_t);
uint8_t is_ghost_same_pos(void);

void set_pacdot(uint8_t, uint8_t);
void reset_pacdot(uint8_t, uint8_t);


/* Public Functions ----------------------------------------------------------*/
/**
 * @brief  	Init Pac-Man game
 * @param  	None
 * @note  	Call when you want to init game
 * @retval 	None
 */
void game_init(void) {
	lcd_clear(BACKGROUND_COLOR);

	// Pacman starts initially at (0,0)
	// Set 1 to all cells which have available pac dot, otherwise set 0
	/*
	for (unsigned char i = 0; i <= MAZE_ROW_N - 1; i++){
		for (unsigned char j = 0; j <= MAZE_COLUMN_N - 1; j++){
			maze.cells[i][j].is_pac_dot = 1;
			pac_dot_draw(1.5*MAZE_CELL_WIDTH + i*20, 1.5*MAZE_CELL_WIDTH + j*20, PAC_DOTS_COLOR);
		}
	}
	*/

	// Set up wall
	maze_bit[0]  = 0x3CF;
	maze_bit[2*MAZE_ROW_N] = 0x3CF;
	for (uint8_t i = 1; i <= 2*MAZE_ROW_N - 1; i++){
		//horizontal line
		if (i % 2 == 0){
			switch (i){
			case 3*2:
			case 2*MAZE_ROW_N - 3*2:
				maze_bit[i] = 0xCC;
				break;
			default:
				maze_bit[i] = 0x0;
			}
		}
		//vertical line and pacdot
		else{
			switch(i){
			case 7:
			case 9:
			case 11:
			case 2*MAZE_ROW_N - 7:
			case 2*MAZE_ROW_N - 9:
			case 2*MAZE_ROW_N - 11:
				maze_bit[i] = 0x1BAABB;
				break;
			default:
				maze_bit[i] = 0x1AAAAB;
			}
		}
	}
	for (uint8_t i = 11 + 2; i < 2*MAZE_ROW_N - 11; i += 2){
		maze_bit[i] = 0xAAAAA;
	}

	//draw initially
	for (uint8_t row = 0; row <= 2*MAZE_ROW_N; row++){
		for (uint8_t shift = 0; shift < 2*MAZE_COLUMN_N + 1; shift++){
			if (row % 2 == 0){
				if (maze_bit[row] & (1<<shift)){
					wall_draw(row, shift);
				}
			}
			// line and pacdot
			else{
				if (maze_bit[row] & (1<<shift)){
					if (shift % 2 == 0){
						wall_draw(row, shift);
					}
					else{
						pac_dot_draw(row/2, MAZE_COLUMN_N - 1 - shift/2, PAC_DOTS_COLOR);
					}
				}
			}
		}
	}

	maze_bit[1] &= ~(1 << 19);

	led_7seg_set_digit(0, 0, 0);
	led_7seg_set_digit(0, 1, 0);

	/*
	 * TO DO
	 *
	 * 3. Init Pac-Man object.
	 */
	pacman.direction = STOP;
	pacman.i = 0; pacman.j = 0;
	pacman.score = 0;
	pacman_draw(pacman.i, pacman.j, PACMAN_COLOR);
	/*
	 * TO DO
	 *
	 * 4. Init Ghost object.
	 * - Firstly, you have to initialize default values for the ghost object.
	 * - Then, draw ghost in the first position.
	 */
	srand(HAL_GetTick());
	ghost[0].color = GHOST_COLOR_0;
	ghost[1].color = GHOST_COLOR_1;
	ghost[2].color = GHOST_COLOR_2;
	ghost[3].color = GHOST_COLOR_3;
	for (uint8_t i = 0; i < 4; i++){
	ghost[i].direction = RIGHT;
	ghost[i].i = 13; ghost[i].j = 9 - i;
	ghost[i].i_pre = 13; ghost[i].j_pre = 9 - i;
	ghost_draw(ghost[i].i, ghost[i].j, ghost[i].color);
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

			while (is_ghost_same_pos()){
				ghost_direction_process();
			}

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
		pacman_draw(pacman.i_pre, pacman.j_pre, BACKGROUND_COLOR);
		pacman_draw(pacman.i, pacman.j, PACMAN_COLOR);
	}
	// GHOST
	for (uint8_t i = 0; i < 4; i++){
		if (ghost[i].direction != STOP){
			// delete previous location
			if ((!is_ghost(ghost[i].i_pre, ghost[i].j_pre))
				&& !(ghost[i].i_pre == pacman.i && ghost[i].j_pre == pacman.j)){
				ghost_draw(ghost[i].i_pre, ghost[i].j_pre, BACKGROUND_COLOR);
				if (is_pacdot(ghost[i].i_pre, ghost[i].j_pre)){
					pac_dot_draw(ghost[i].i_pre, ghost[i].j_pre, PAC_DOTS_COLOR);
				}
			}
			ghost_draw(ghost[i].i, ghost[i].j, ghost[i].color);
		}
	}
	led_7seg_set_digit(pacman.score % 10, 3, 0);
	led_7seg_set_digit((pacman.score / 10) % 10, 2, 0);
	led_7seg_set_digit(pacman.score / 100, 1, 0);
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
	if (is_pacdot(pacman.i, pacman.j)){
			reset_pacdot(pacman.i, pacman.j);
			pacman.score++;
	}
	// restart game
	if (button_count[15] == 60){
		game_init();
	}
	// Win
	else if (pacman.score == 139){
		if (time_show == 0){
			lcd_clear(BACKGROUND_COLOR);
			lcd_show_string(50, 100, "YOU WIN", BLUE, WHITE, 32, 1);
			time_show = time_show + 1;
		}
		if (time_show == 60){
			game_init();
			time_show = 0;
			return 1;
		}
		time_show = (time_show + 1) % 61;
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

		if (time_show == 60){
			game_init();
			time_show = 0;
			return 1;
		}
		time_show = (time_show + 1) % 61;
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
	if (is_wall(pacman.i, pacman.j, pacman.direction)){
		pacman.direction = STOP;
	}
	switch (pacman.direction) {
	case UP:
		(pacman.i - 1 < 0) ? (pacman.i = MAZE_ROW_N - 1) : (pacman.i--);
		break;

	case DOWN:
		(pacman.i + 1 > MAZE_ROW_N - 1) ? (pacman.i = 0) : (pacman.i++);
		break;

	case LEFT:
		(pacman.j - 1 < 0) ? (pacman.j = MAZE_COLUMN_N - 1) : (pacman.j--);
		break;

	case RIGHT:
		(pacman.j + 1 > MAZE_COLUMN_N - 1) ? (pacman.j = 0) : (pacman.j++);
		break;
	case STOP:
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
	E_DIRECTION direction = 0;
	for (uint8_t i = 0; i < 4; i++){
		direction = rand() % 4;
		ghost[i].direction = direction;
		if (is_wall(ghost[i].i, ghost[i].j, ghost[i].direction)){
			ghost[i].direction = STOP;
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
			(ghost[i].i - 1 < 0) ? (ghost[i].i = MAZE_ROW_N - 1) : (ghost[i].i--);
			break;

		case DOWN:
			(ghost[i].i + 1 > MAZE_ROW_N - 1) ? (ghost[i].i = 0) : (ghost[i].i++);
			break;

		case LEFT:
			(ghost[i].j - 1 < 0) ? (ghost[i].j = MAZE_COLUMN_N - 1) : (ghost[i].j--);
			break;

		case RIGHT:
			(ghost[i].j + 1 > MAZE_COLUMN_N - 1) ? (ghost[i].j = 0) : (ghost[i].j++);
			break;

		case STOP:

			break;
		}
	}
}

void pac_dot_draw(uint16_t i, uint8_t j, uint16_t color) {
	/*
	 * TO DO
	 *
	 * Draw whatever you like
	 */
	//pac_dot_draw(row, shift, PAC_DOTS_COLOR);
	lcd_draw_circle((j+1.5)*MAZE_CELL_WIDTH, (i + 1.5)*MAZE_CELL_WIDTH, color, 4, 1);
}

void pacman_draw(uint16_t i, uint8_t j, uint16_t color) {
	lcd_draw_circle((j + 1.5)*MAZE_CELL_WIDTH, (i + 1.5)*MAZE_CELL_WIDTH, color, 7, 1);
}

void ghost_draw(uint16_t i, uint8_t j, uint16_t color) {
	lcd_draw_circle((j+1.5)*MAZE_CELL_WIDTH, (i+1.5)*MAZE_CELL_WIDTH, color, 7, 1);
}
// 1,0
void wall_draw(uint8_t i, uint8_t j){
	// horizontal line
	if ((i % 2 == 0) && (j < 10)){
		lcd_fill((MAZE_COLUMN_N - 1 - j + 1)*MAZE_CELL_WIDTH - 2, (i/2+1)*MAZE_CELL_WIDTH - 2, (MAZE_COLUMN_N - 1 - j + 1 + 1)*MAZE_CELL_WIDTH + 2, (i/2+1)*MAZE_CELL_WIDTH + 2, WALL_COLOR);
		//lcd_draw_line((MAZE_COLUMN_N - 1 - j + 1)*MAZE_CELL_WIDTH, (i/2+1)*MAZE_CELL_WIDTH, (MAZE_COLUMN_N - 1 - j + 1 + 1)*MAZE_CELL_WIDTH, (i/2+1)*MAZE_CELL_WIDTH, WALL_COLOR);
	}

	// vertical line
	else{
		lcd_fill((MAZE_COLUMN_N - j/2 + 1)*MAZE_CELL_WIDTH - 2, (i/2 + 1)*MAZE_CELL_WIDTH, (MAZE_COLUMN_N - j/2 + 1)*MAZE_CELL_WIDTH + 2, (i/2 + 2)*MAZE_CELL_WIDTH, WALL_COLOR);
		//lcd_draw_line((MAZE_COLUMN_N - j/2 + 1)*MAZE_CELL_WIDTH, (i/2 + 1)*MAZE_CELL_WIDTH, (MAZE_COLUMN_N - j/2 + 1)*MAZE_CELL_WIDTH, (i/2 + 2)*MAZE_CELL_WIDTH, WALL_COLOR);
	}
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

uint8_t is_pacdot(uint8_t i, uint8_t j){
	if (maze_bit[2*i + 1] & (1 << (2*MAZE_COLUMN_N - (2*j + 1)))){
		return 1;
	}
	return 0;
}

uint8_t is_wall(uint8_t i, uint8_t j, E_DIRECTION direction1){
	switch (direction1){
	case UP:
		return !!(maze_bit[2*i] & (1 << (MAZE_COLUMN_N - 1 - j)));
	case DOWN:
		return !!(maze_bit[2*i + 2] & (1 << (MAZE_COLUMN_N - 1 - j)));
	case LEFT:
		return !!(maze_bit[2*i + 1] & (1 << (2*MAZE_COLUMN_N - (2*j))));
	case RIGHT:
		return !!(maze_bit[2*i + 1] & (1 << (2*MAZE_COLUMN_N - (2*j + 2))));
	case STOP:
	default:
		return 1;
	}
}

void set_pacdot(uint8_t i, uint8_t j){
	maze_bit[2*i + 1] |= (1 << (2*MAZE_COLUMN_N - (2*j + 1)));
}

void reset_pacdot(uint8_t i, uint8_t j){
	maze_bit[2*i + 1] &= ~(1 << (2*MAZE_COLUMN_N - (2*j + 1)));
}

uint8_t is_ghost_same_pos (void){
	uint8_t i1;
	uint8_t j1;
	uint8_t i2;
	uint8_t j2;
	for (uint8_t i = 0; i < 3; i++){
		i1 = ghost[i].i;
		j1 = ghost[i].j;
		switch (ghost[i].direction) {
		case UP:
			(i1 - 1 < 0) ? (i1 = MAZE_ROW_N - 1) : (i1--);
			break;

		case DOWN:
			(i1 + 1 > MAZE_ROW_N - 1) ? (i1 = 0) : (i1++);
			break;

		case LEFT:
			(j1 - 1 < 0) ? (j1 = MAZE_COLUMN_N - 1) : (j1--);
			break;

		case RIGHT:
			(j1 + 1 > MAZE_COLUMN_N - 1) ? (j1 = 0) : (j1++);
			break;

		case STOP:
			break;
		}
		for (uint8_t j = i + 1; j < 4; j++){
			i2 = ghost[j].i;
			j2 = ghost[j].j;
			switch (ghost[j].direction) {
			case UP:
				(i2 - 1 < 0) ? (i2 = MAZE_ROW_N - 1) : (i2--);
				break;

			case DOWN:
				(i2 + 1 > MAZE_ROW_N - 1) ? (i2 = 0) : (i2++);
				break;

			case LEFT:
				(j2 - 1 < 0) ? (j2 = MAZE_COLUMN_N - 1) : (j2--);
				break;

			case RIGHT:
				(j2 + 1 > MAZE_COLUMN_N - 1) ? (j2 = 0) : (j2++);
				break;

			case STOP:
				break;
			}
			if (i1 == i2 && j1 == j2){
				return 1;
			}
		}
	}
	return 0;
}

uint8_t is_ghost(uint8_t i, uint8_t j){
	for (uint8_t index = 0; index < 4; index++){
		if (ghost[index].i == i && ghost[index].j == j){
			return 1;
		}
	}
	return 0;
}
