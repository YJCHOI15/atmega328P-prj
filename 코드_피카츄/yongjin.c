#include "yongjin.h"
#define F_CPU 16000000UL // 16 MHz 클럭

#include "usart.h"
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "seokmin.h"

volatile uint8_t water_refill_mode = 0;
volatile uint8_t blink_flag = 0;
volatile uint8_t timer_initialized = 0;

void check_pressure(uint16_t fsr_value) {
	char buffer[32];

	// 압력 센서의 값을 구분하여 출력
	sprintf(buffer, "압력센서 ADC: %u\r\n", fsr_value);
	USART_transmit_string(buffer);

	// 압력 센서 값이 PRESSURE_THRESHOLD 이하이면 절전 모드로 진입
	if (fsr_value <= PRESSURE_THRESHOLD) {
		USART_transmit_string("LOW PRESSURE. ENTER THE SLEEPMODE\r\n");
		// _delay_ms(100);  // 절전 모드 진입 전 약간의 딜레이
		Sleep_enter(); // 절전 모드 진입
	}
}

void check_water_level(uint16_t water_value) {
	char buffer[32];

	// 수위 센서의 값을 구분하여 출력
	// sprintf(buffer, "수위센서 ADC: %u\r\n", water_value);
	// USART_transmit_string(buffer);

	// 수위 센서 값이 WATER_THRESHOLD 이하이면 물을 채움
	if (water_value <= WATER_THRESHOLD) {
		USART_transmit_string("ENTER THE REFILL WATER MODE!!!\r\n");
		_delay_ms(100);
		enter_refill_water_mode(); // 물 채움 모드 진입
	}
	else {
		if (water_refill_mode == 1)
			exit_refill_water_mode();
	}
}

void enter_refill_water_mode() {
	water_refill_mode = 1; // 물 채움 모드 활성화
	stop_water_spray(); // 물 분사 중지
	start_led_blink(); // LED 깜빡임 시작
}

// 물 분사 중지 함수
void stop_water_spray() {
	USART_transmit_string("Water spray stopped.\r\n");
	// 물 분사 중지에 해당하는 코드를 추가
}

// LED 깜빡임을 블로킹 함수으로 구현
/*
void start_led_blink() {
	while (water_refill_mode) {
		// 분사 LED 깜빡임
		PORTB |= (1 << Blink_LED_PIN); // LED ON
		_delay_ms(REFILL_DELAY);
		PORTB &= ~(1 << Blink_LED_PIN); // LED OFF
		_delay_ms(REFILL_DELAY);

		// water_refill_mode가 0이 될 때까지 깜빡임 유지
	}
}
*/

// LED 깜빡임을 논블로킹 방식으로 구현 (타이머2 사용)
// LED_PIN은 분사 표시 LED
ISR(TIMER2_COMPA_vect) {
	// 타이머 인터럽트 발생 시 LED 깜빡임 상태 전환
	if (blink_flag) {
		// PORTB &= ~(1 << Blink_LED_PIN); // LED OFF
		blink_flag = 0;
	} else {
		// PORTB |= (1 << Blink_LED_PIN); // LED ON
		blink_flag = 1;
	}
}

void start_led_blink() {
	if (!timer_initialized) {
		// 타이머2 설정
		TCCR2A |= (1 << WGM21); // CTC 모드
		TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20); // 분주비 1024
		OCR2A = 156; // 약 1초마다 인터럽트 발생 (16MHz 클럭 기준)
		TIMSK2 |= (1 << OCIE2A); // 비교 일치 인터럽트 허용
		timer_initialized = 1; // 타이머가 설정되었음을 표시
	}
}

void exit_refill_water_mode() {
	water_refill_mode = 0;
	timer_initialized = 0; // 타이머 설정을 초기화하여 다시 시작할 수 있도록 함
	TCCR2B &= ~((1 << CS22) | (1 << CS21) | (1 << CS20)); // 타이머 중지
	USART_transmit_string("Exit refill water mode.\r\n");
}