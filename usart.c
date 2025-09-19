#include "usart.h"
#include "seokmin.h"
#include <stdio.h>

void USART_init(unsigned int ubrr) {
    /* Baud rate 설정 */
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;

    /* 전송 활성화 (TX), 수신 비활성화 (RX는 활성화하지 않음) */
    UCSR0B = (1 << TXEN0);

    /* 8-bit 데이터 형식, 1 스톱 비트 */
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void USART_transmit(unsigned char data) {
    /* 전송 버퍼가 비어질 때까지 대기 */
    while (!(UCSR0A & (1 << UDRE0)))
        ;

    /* 데이터를 전송 버퍼에 기록 */
    UDR0 = data;
}

void USART_transmit_string(const char* str) {
    while (*str) {
        USART_transmit(*str++);
    }
}

// 사용 스위치에 따라 자료형 변경
void USART_Switch_print_pin_states(uint32_t pin_states) {
    for (uint8_t i = 0; i < 16; i++) {
        USART_transmit_string("Pin-");
        USART_transmit('0' + (i%10)); // 핀 번호 출력

        if (pin_states & (1 << i)) {
            USART_transmit_string(": HIGH\r\n"); // 핀이 HIGH일 경우
        } else {
            USART_transmit_string(": LOW\r\n"); // 핀이 LOW일 경우
        }
    }
    USART_transmit_string("-----------------\n\n\n");
}

void USART_EEPROM_debugUserData(UserData *data) {
    char buffer[50];
    
    // water_pressure 값을 전송
    sprintf(buffer, "Water Pressure: %u\r\n", data->water_pressure);
    USART_transmit_string(buffer);

    // temperature 값을 전송
    sprintf(buffer, "Temperature: %u\r\n", data->temperature);
    USART_transmit_string(buffer);

    // nozzle_position 값을 전송
    sprintf(buffer, "Nozzle Position: %ld\r\n", data->nozzle_position);
    USART_transmit_string(buffer);
}