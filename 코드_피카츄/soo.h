#ifndef SOO_H
#define SOO_H

#define F_CPU 16000000UL  // CPU 클럭 속도 설정

#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include "seokmin.h"

// 핀 정의
// #define SEAT_RELAY_PIN PD7
// #define WATER_RELAY_PIN PD6
// #define DRY_MOTOR_PIN PD3
// #define DRY_BUTTON_PIN PD4
// #define SEAT_TEMP_UP_PIN PB0
// #define SEAT_TEMP_DOWN_PIN PB1
// #define WATER_TEMP_UP_PIN PB3
// #define WATER_TEMP_DOWN_PIN PB2
#define ADC_SEAT_PIN PC0
#define ADC_WATER_PIN PC1
#define MAX_TEMP_SEAT 30.0
#define MAX_TEMP_WATER 30.0

extern float temp_seat_setting;
extern float temp_water_setting;
extern bool motorState;
extern bool lastButtonState;

extern float R1;  // R1을 extern으로 선언

// void UART_init(unsigned int ubrr);
// void UART_transmit(unsigned char data);
// void UART_print(const char* str);
void UART_print_float(float num);

float thermistor(float average);
void setupPins(void);
bool controlWaterHeater(volatile UserData *userData);
bool controlSeatHeater(volatile UserData *userData);
// void controlMotor(volatile uint32_t data);

#endif
