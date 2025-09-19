#include "seokmin.h"
#include "yongjin.h"
#define F_CPU 16000000UL

#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "usart.h"
#include "adc.h"

volatile uint32_t Switch_newstate = 0;

volatile uint8_t Sleep_person_sitting = 0;  // 초기 상태: 사람이 일어나 있지 않음
volatile uint16_t Sleep_timer_count = 0;    // 5분 타이머 카운트 (초 단위)

volatile uint8_t selectedUser = 0;  // 사용자 선택 변수

// 외부 인터럽트 서비스 루틴 (INT1)
ISR(INT1_vect) {
    // cli();
    USART_transmit_string("-ISR-\n");
    // 외부 인터럽트가 발생하면 스위치 상태를 갱신
    Switch_74HC165_read();
    USART_Switch_print_pin_states(Switch_newstate);
    // sei();
}

// 시스템 초기화 함수
void seokmin_init_system(void) {
    // I/O 설정
    DDRB |= (1 << LED_74HC595_SH_CP) | (1 << LED_74HC595_ST_CP) | (1 << LED_74HC595_DS);  // 74HC595 제어 핀 출력 설정
    DDRD |= (1 << Switch_74HC165_SH_CP) | (1 << Switch_74HC165_PL);                     // 74HC165 제어 핀 출력 설정
    PORTD &= ~(1 << PD3);   //active High 

    // 외부 인터럽트 설정 (INT1)
    EICRA |= (1 << ISC11) | (1 << ISC10); // INT1을 상승 에지 트리거로 설정
    EIMSK |= (1 << INT1);   // INT1 인터럽트 활성화

    // 슬립 모드 설정
    DDRD &= ~(1 << PRESSURE_SENSOR_PIN);  // PD2 입력 설정
    Sleep_setup_external_interrupt_INT0();
    // Sleep_setup_timer();

    // 전역 인터럽트 활성화
    sei();

    // 초기 스위치 상태 읽기
    Switch_74HC165_read();
}

// 74HC165에서 스위치 상태를 읽는 함수
void Switch_74HC165_read(void) {
    PORTD &= ~(1 << Switch_74HC165_PL);  // 병렬 로드 신호를 Low로
    _delay_us(5);
    PORTD |= (1 << Switch_74HC165_PL);   // 병렬 로드 신호를 High로

    Switch_newstate = 0;
    for (int i = 0; i < NUM_Switch; i++) {
        PORTD |= (1 << Switch_74HC165_SH_CP);  // 시프트 클럭 상승
        if (PIND & (1 << Switch_74HC165_QH)) {  // 시리얼 데이터 읽기
            Switch_newstate |= (1 << (NUM_Switch - i - 1));
        }
        _delay_us(5);
        PORTD &= ~(1 << Switch_74HC165_SH_CP); // 시프트 클럭 하강
    }
}

// 74HC595에 데이터를 쓰는 함수
// LED 개수에 따라 데이터 자료형 변경
void LED_74HC595_write(uint32_t data) {
    // 24비트 데이터(3개의 74HC595)를 시프트
    for (int i = 0; i < 24; i++) {
        if (data & (1UL << (23 - i))) {
            PORTB |= (1 << LED_74HC595_DS);  // 데이터 비트 설정
        } else {
            PORTB &= ~(1 << LED_74HC595_DS); // 데이터 비트 클리어
        }

        // 시프트 클럭 펄스 생성
        PORTB |= (1 << LED_74HC595_SH_CP);  // 시프트 클럭 상승
        _delay_us(1);
        PORTB &= ~(1 << LED_74HC595_SH_CP); // 시프트 클럭 하강
    }

    // 저장 클럭 펄스 생성
    PORTB |= (1 << LED_74HC595_ST_CP);  // 저장 클럭 상승
    _delay_us(1);
    PORTB &= ~(1 << LED_74HC595_ST_CP); // 저장 클럭 하강
}

// EEPROM에 바이트를 쓰는 함수
void EEPROM_write(uint16_t addr, uint8_t data) {
    while (EECR & (1 << EEPE));  // EEPROM이 준비될 때까지 대기
    
    EEAR = addr;                 // 주소 설정
    EEDR = data;                 // 데이터 설정
    
    EECR |= (1 << EEMPE);        // EEPROM 쓰기 허용
    EECR |= (1 << EEPE);         // 쓰기 명령 실행
}

// EEPROM에서 바이트를 읽는 함수
uint8_t EEPROM_read(uint16_t addr) {
    while (EECR & (1 << EEPE));  // EEPROM이 준비될 때까지 대기
    
    EEAR = addr;                 // 주소 설정
    EECR |= (1 << EERE);         // 읽기 명령 실행
    
    return EEDR;                 // 읽은 데이터 반환
}


// 구조체 데이터를 EEPROM에 저장하는 함수
void EEPROM_saveUserData(volatile UserData *data, uint8_t user_number) {
    uint16_t addr;
    switch (user_number) {
        case 0:
            addr = EEPROM_USER1_BASE_ADDR;
            break;
        case 1:
            addr = EEPROM_USER2_BASE_ADDR;
            break;
        case 2:
            addr = EEPROM_USER3_BASE_ADDR;
            break;
        default:
            return;  // Invalid user number
    }
    EEPROM_write(addr++, data->water_pressure);
    EEPROM_write(addr++, data->temperature);
    EEPROM_write(addr++, data->nozzle_position);
    EEPROM_write(addr, data->seat_temperature);
}

// 구조체 데이터를 EEPROM에서 불러오는 함수
void EEPROM_loadUserData(volatile UserData *data, uint8_t user_number) {
    uint16_t addr;
    switch (user_number) {
        case 0:
            addr = EEPROM_USER1_BASE_ADDR;
            break;
        case 1:
            addr = EEPROM_USER2_BASE_ADDR;
            break;
        case 2:
            addr = EEPROM_USER3_BASE_ADDR;
            break;
        default:
            return;  // Invalid user number
    }
    data->water_pressure = EEPROM_read(addr++);
    data->temperature = EEPROM_read(addr++);
    data->nozzle_position = EEPROM_read(addr++);
    data->seat_temperature = EEPROM_read(addr);

}

/*
Sleep
*/
// 외부 인터럽트 서비스 루틴
// ISR(INT0_vect) {
// 	// if (PIND & (1 << PRESSURE_SENSOR_PIN)) {
// 	// 	// 사람이 앉음
// 	// 	// Sleep_person_sitting = 1;
// 	// 	// Sleep_timer_count = 0;  // 타이머 리셋
// 	// 	// _delay_ms(1000);

// 	// 	// 타이머 멈춤
// 	// 	// TCCR1B &= ~((1 << CS12) | (1 << CS10));  // 타이머 클럭을 중지
// 	// 	} else {
// 	// 	// 사람이 일어남
// 	// 	Sleep_person_sitting = 0;

// 	// 	// 타이머 시작
// 	// 	TCCR1B |= (1 << CS12) | (1 << CS10);  // 1024 프리스케일 설정
// 	// }

//     // Wake up
//     USART_transmit_string("\n\nwake up\n\n");
// }


// 타이머1 비교 매치 인터럽트 서비스 루틴 (1초마다 호출)
// ISR(TIMER1_COMPA_vect) {
//     if (!Sleep_person_sitting) {
//         if (Sleep_timer_count < FIVE_MINUTES) {
//             Sleep_timer_count++;
//         } else {
//             // 5분이 지나면 절전 모드로 진입
//             Sleep_enter();
//         }
//     }
// }

// 외부 인터럽트 설정 함수
void Sleep_setup_external_interrupt_INT0(void) {
    // 외부 인터럽트 설정 (INT0: PD2)
    EICRA |= (1 << ISC00);  // any logical change 감지
    // EIMSK |= (1 << INT0);   // INT0 인터럽트 허용
}

// 타이머 설정 함수
// void Sleep_setup_timer(void) {
//     // 타이머1 설정: 1초마다 인터럽트 발생 (타이머는 사람이 일어났을 때만 시작)
//     TCCR1B |= (1 << WGM12);               // CTC 모드 설정
//     OCR1A = 15624;                        // 비교 값: 1초마다 인터럽트 (16MHz / 1024 / 1Hz)
//     TIMSK1 |= (1 << OCIE1A);              // 타이머 비교 매치 인터럽트 허용
//     // 타이머 클럭을 아직 활성화하지 않음 (사람이 일어날 때 활성화)
// }

// 절전 모드 진입 함수
void Sleep_enter(void) {
    // Wake up interrupt 설정
    EIMSK |= (1 << INT0);   // INT0 인터럽트 허용

    // Power-down 모드로 진입
    SMCR = (1 << SE) | (1 << SM1);  // SE=1 (sleep enable), SM1=1 (power-down 모드 설정)
    asm volatile("sleep");          // sleep 명령 실행
    
    EIMSK &= ~(1 << INT0);   // INT0 인터럽트 해제

    USART_transmit_string("\n\nwake up\n\n");
    SMCR &= ~(1 << SE);             // 슬립 모드 비활성화 // sleep 모드에서 깨어난 후 SE 비트 비활성화
}
