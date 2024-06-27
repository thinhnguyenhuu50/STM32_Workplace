/*
 * ds3231.c
 */

/* Includes */
#include "ds3231.h"

#include "i2c.h"
#include "utils.h"

/* Constants */
#define DS3231_ADDRESS 0x68<<1

/* Variables */
uint8_t ds3231_buffer[7];

uint8_t ds3231_hours = 0;
uint8_t ds3231_min = 0;
uint8_t ds3231_sec = 0;
uint8_t ds3231_date = 0;
uint8_t ds3231_day = 0;
uint8_t ds3231_month = 0;
uint8_t ds3231_year = 0;

void ds3231_init() {
	ds3231_buffer[0] = DEC2BCD(30); //second
	ds3231_buffer[1] = DEC2BCD(22); //minute
	ds3231_buffer[2] = DEC2BCD(21); //hour
	ds3231_buffer[3] = DEC2BCD(6);  //day
	ds3231_buffer[4] = DEC2BCD(15); //date
	ds3231_buffer[5] = DEC2BCD(9);  //month
	ds3231_buffer[6] = DEC2BCD(23); //year

	if (HAL_I2C_IsDeviceReady(&hi2c1, DS3231_ADDRESS, 3, 50) != HAL_OK) {
		while (1)
			;
	};
}

void ds3231_write(uint8_t address, uint8_t value) {
	uint8_t temp = DEC2BCD(value);
	HAL_I2C_Mem_Write(&hi2c1, DS3231_ADDRESS, address, I2C_MEMADD_SIZE_8BIT,
			&temp, 1, 10);
}

uint8_t ds3231_read(uint8_t address) {
	uint8_t result;
	HAL_I2C_Mem_Read(&hi2c1, DS3231_ADDRESS, 0x00, I2C_MEMADD_SIZE_8BIT,
			&result, 1, 10);
	return BCD2DEC(result);
}

void ds3231_read_time() {
	HAL_I2C_Mem_Read(&hi2c1, DS3231_ADDRESS, 0x00, I2C_MEMADD_SIZE_8BIT,
			ds3231_buffer, 7, 10);

	ds3231_sec = BCD2DEC(ds3231_buffer[0]);
	ds3231_min = BCD2DEC(ds3231_buffer[1]);
	ds3231_hours = BCD2DEC(ds3231_buffer[2]);
	ds3231_day = BCD2DEC(ds3231_buffer[3]);
	ds3231_date = BCD2DEC(ds3231_buffer[4]);
	ds3231_month = BCD2DEC(ds3231_buffer[5]);
	ds3231_year = BCD2DEC(ds3231_buffer[6]);
}
