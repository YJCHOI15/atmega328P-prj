#include "optsuyeon.h"
#define F_CPU 16000000UL // 16 MHz 클럭

#include <avr/io.h>
#include <stdio.h> // snprintf 사용을 위한 헤더 파일

#include <util/delay.h>
#include "usart.h"

int simulatedPressure = 50;  // 초기 압력값 (시뮬레이션용)

// 시리얼 통신 초기화 (9600 bps)
// void setupSerial() {
//     UBRR0H = (F_CPU / 16 / 9600 - 1) >> 8; // 통신 속도 상위 비트 설정
//     UBRR0L = (F_CPU / 16 / 9600 - 1); // 통신 속도 하위 비트 설정
//     UCSR0B = (1 << TXEN0); // 송신 허용
//     UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8비트 데이터 프레임 설정
// }

// // 시리얼 데이터를 송신하는 함수
// void serialPrint(const char* str) {
//     while (*str) {
//         while (!(UCSR0A & (1 << UDRE0))); // 데이터 레지스터 비어 있는지 확인
//         UDR0 = *str; // 데이터 송신
//         str++;
//     }
// }

// 서보모터 초기 설정 및 PWM 설정 함수
void INIT_TIMER1(void) {
    TCCR1A |= (1 << WGM11);
    TCCR1B |= (1 << WGM12) | (1 << WGM13);
    TCCR1B |= (1 << CS11); // 분주율 8, 2MHz
    ICR1 = 40000; // 20ms 주기
    TCCR1A |= (1 << COM1B1); // 비반전 모드 (PB2, 아두이노 10번 핀에 해당)
    DDRB |= (1 << SERVO_PIN);
}

// 서보모터 각도 설정 함수
void set_servo_angle(int angle) {
    int pulseWidth = (int)(40000 * (0.0325 + (0.085 / 180.0) * angle));
    OCR1B = pulseWidth;
}

// 서보모터를 특정 각도로 설정하고 대기 후 복귀하는 함수
void flush_toilet(int angle) {
    set_servo_angle(angle);
    _delay_ms(2000); // 대기 시간
    set_servo_angle(0);   // 초기 위치로 복귀
}

// 모터 핀 설정 및 초기화
void setup_motors() {
    DDRC |= (1 << IN1) | (1 << IN2) | (1 << IN5); // Set PC5, PC4, PC6 as outputs
    DDRB |= (1 << IN3) | (1 << IN4);              // Set PB5, PB4 as outputs
    DDRD |= (1 << IN6) | (1 << IN7) | (1 << IN8); // Set PD4, PD1, PD0 as outputs
}

// 노즐 위치 조정 함수 (방향 지정)
void move_nozzle(int steps, int direction) {
    for (int i = 0; i < steps; i++) {
        if (direction == 1) {  // 정방향
            // 1단계
            PORTC |= (1 << IN1); PORTB |= (1 << IN3);
            PORTC &= ~(1 << IN2); PORTB &= ~(1 << IN4);
            _delay_ms(10);

            // 2단계
            PORTC |= (1 << IN2); PORTB |= (1 << IN3);
            PORTC &= ~(1 << IN1); PORTB &= ~(1 << IN4);
            _delay_ms(10);

            // 3단계
            PORTC |= (1 << IN2); PORTB |= (1 << IN4);
            PORTC &= ~(1 << IN1); PORTB &= ~(1 << IN3);
            _delay_ms(10);

            // 4단계
            PORTC |= (1 << IN1); PORTB |= (1 << IN4);
            PORTC &= ~(1 << IN2); PORTB &= ~(1 << IN3);
        } else {  // 역방향
            // 1단계
            PORTC |= (1 << IN1); PORTB |= (1 << IN4);
            PORTC &= ~(1 << IN2); PORTB &= ~(1 << IN3);
            _delay_ms(10);

            // 2단계
            PORTC |= (1 << IN2); PORTB |= (1 << IN4);
            PORTC &= ~(1 << IN1); PORTB &= ~(1 << IN3);
            _delay_ms(10);

            // 3단계
            PORTC |= (1 << IN2); PORTB |= (1 << IN3);
            PORTC &= ~(1 << IN1); PORTB &= ~(1 << IN4);
            _delay_ms(10);

            // 4단계
            PORTC |= (1 << IN1); PORTB |= (1 << IN3);
            PORTC &= ~(1 << IN2); PORTB &= ~(1 << IN4);
        }
        _delay_ms(10);
    }
    // clear
    PORTC &= ~((1 << IN1) | (1 << IN2) | (1 << IN5));
    PORTB &= ~((1 << IN3) | (1 << IN4));
    PORTD &= ~((1 << IN6) | (1 << IN7) | (1 << IN8));
    
}


// 물 분사 강도 조절 함수
void set_spray_level(int level) {
    char buffer[50];
    int steps = 0;
    switch(level) {
        case 1:
            steps = STEPS_FOR_ANGLE_1;
            break;
        case 2:
            steps = STEPS_FOR_ANGLE_2;
            break;
        case 3:
            steps = STEPS_FOR_ANGLE_3;
            break;
        default:
            return;
    }
    move_spray(steps); // 분사 각도에 맞게 이동
    snprintf(buffer, sizeof(buffer), "Spray Level %d: %d steps\n", level, steps);
    USART_transmit_string(buffer);
}

// 물 분사 각도 제어 함수
void move_spray(int steps) {
    for (int i = 0; i < steps; i++) {
        // 1단계
        PORTC |= (1 << IN5); 
        PORTC &= ~(1 << IN5); 
        PORTD |= (1 << IN6);
        PORTD &= ~((1 << IN7) | (1 << IN8)); 
        _delay_ms(1);
        
        // 2단계
        PORTD |= (1 << IN7) | (1 << IN6);
        PORTD &= ~(1 << IN8); 
        _delay_ms(1);

        // 3단계
        PORTD |= (1 << IN7) | (1 << IN8);
        PORTD &= ~(1 << IN6);
        _delay_ms(1);

        // 4단계
        PORTC |= (1 << IN5);
        PORTC &= ~(1 << IN5); 
        PORTD |= (1 << IN8);
        PORTD &= ~((1 << IN6) | (1 << IN7)); 
        _delay_ms(1);
    }
}
