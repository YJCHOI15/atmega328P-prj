#ifndef SEOKMIN_H
#define SEOKMIN_H

#include <avr/io.h>
#include <stdint.h>
#include <avr/eeprom.h>

// EEPROM 주소 설정
#define EEPROM_WATER_PRESSURE_ADDR 0x00
#define EEPROM_TEMPERATURE_ADDR    0x01
#define EEPROM_NOZZLE_POSITION_ADDR 0x02

#define EEPROM_USER1_BASE_ADDR 0x00 // EEPROM 시작 주소( user0 data)
#define EEPROM_USER2_BASE_ADDR 0x10
#define EEPROM_USER3_BASE_ADDR 0x20

// 74HC165 핀 정의
#define Switch_74HC165_SH_CP PD5      // 시프트 클럭 핀
#define Switch_74HC165_PL   PD6       // 병렬 로드 핀
#define Switch_74HC165_QH   PD7       // 시리얼 데이터 출력 핀

// 74HC595 핀 정의
#define LED_74HC595_SH_CP PB0      // 시프트 클럭 핀
#define LED_74HC595_ST_CP PB1      // 저장 클럭 핀
#define LED_74HC595_DS    PB3      // 데이터 입력 핀
// #define LED_74HC595_OE    PB4      // 출력 활성화 핀
#define LED_NUM    16
#define NUM_Switch 16

// Sleep
#define PRESSURE_SENSOR_PIN PD2  // 압력 센서가 연결된 핀 (INT0)
#define FIVE_MINUTES 300          // 5분을 초 단위로 표현

// 사용자 데이터를 저장할 구조체
typedef struct {
    uint8_t water_pressure;
    uint8_t temperature;
    int32_t nozzle_position;
    uint8_t seat_temperature;
} UserData;

// 함수 선언
void seokmin_init_system(void);
// Switch
void Switch_74HC165_read(void);
void LED_74HC595_write(uint32_t data);
// void Sleep_initsetup(void);
// void Sleep_enter(void);
// void Sleep_exit(void);
// void Sleep_check_pressure(void);
// uint16_t read_adc(uint8_t channel);

// eeprom
// void EEPROM_usersave(uint8_t water_pressure, uint8_t temperature, uint8_t nozzle_position);
// void EEPROM_userload(uint8_t *water_pressure, uint8_t *temperature, uint8_t *nozzle_position);
void EEPROM_saveUserData(volatile UserData *data, uint8_t user_number);
void EEPROM_loadUserData(volatile UserData *data, uint8_t user_number);
uint8_t EEPROM_read(uint16_t addr);
void EEPROM_write(uint16_t addr, uint8_t data);

// Sleep
extern volatile uint8_t Sleep_person_sitting;
extern volatile uint16_t Sleep_timer_count;

void Sleep_setup_external_interrupt_INT0(void);  // 외부 인터럽트 설정 함수
void Sleep_setup_timer(void);               // 타이머 설정 함수
void Sleep_enter(void);               // 절전 모드 진입 함수

#endif
