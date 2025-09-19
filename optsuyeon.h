// optsuyeon.h

#ifndef OPTSUYEON_H
#define OPTSUYEON_H
#include <avr/io.h>
#include <stdio.h> // snprintf 사용을 위한 헤더 파일

// 서보모터 관련 상수 및 변수
#define SERVO_PIN PB2
// #define PRESSURE_THRESHOLD 500
extern int simulatedPressure;

// 서보모터 관련 함수 선언
void INIT_TIMER1(void);
void set_servo_angle(int angle);           // 함수 이름 변경: setServoAngle → set_servo_angle
void flush_toilet(int angle); // 함수 이름 변경: 기존에 없던 함수 추가


// 스텝모터 관련 상수 및 변수
#define IN1 PC5
#define IN2 PC4
#define IN3 PB5
#define IN4 PB4

#define IN5 PC6
#define IN6 PD4
#define IN7 PD1  // D8 핀에 해당
#define IN8 PD0  // D9 핀에 해당

#define STEP_COUNT_1CM 50
#define STEPS_PER_DEGREE 11.38
#define SPRAY_ANGLE_1 15
#define SPRAY_ANGLE_2 30
#define SPRAY_ANGLE_3 45

#define STEPS_FOR_ANGLE_1 (int)(SPRAY_ANGLE_1 * STEPS_PER_DEGREE)
#define STEPS_FOR_ANGLE_2 (int)(SPRAY_ANGLE_2 * STEPS_PER_DEGREE)
#define STEPS_FOR_ANGLE_3 (int)(SPRAY_ANGLE_3 * STEPS_PER_DEGREE)

// 스텝모터 관련 함수 선언
void setup_motors(void);            // 함수 이름 변경: setupMotors → setup_motors
void move_nozzle(int steps, int direction); // 기존의 move_nozzle_forward와 move_nozzle_backward를 통합
void set_spray_level(int level);
void move_spray(int steps);         // 분사 각도 제어 함수 추가

// 시리얼 통신 관련 함수 선언
// void setupSerial(void);
// void serialPrint(const char* str);

#endif
