#ifndef YONGJIN_H
#define YONGJIN_H

#include <avr/io.h>

#define WATERLEVEL_ADC_PIN   PC2  // 수위 센서가 연결된 아날로그 핀
#define PRESSURE_ADC_PIN     PC3  // FSR402 압력 센서가 연결된 아날로그 핀

#define PRESSURE_THRESHOLD   800 // 절전 모드로 진입할 압력 임계값
#define WATER_THRESHOLD      500 // 물을 채우기 위한 임계값

#define Blink_LED_PIN        PB0

void check_pressure(uint16_t fsr_value);
void check_water_level(uint16_t water_value);
void enter_refill_water_mode(void);
void exit_refill_water_mode(void);

void stop_water_spray();
void start_led_blink();


#endif
