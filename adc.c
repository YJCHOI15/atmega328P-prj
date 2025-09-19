#include "adc.h"

// ADC 초기화 함수
void ADC_init() {
    ADMUX = (1 << REFS0); // AVCC를 기준 전압으로 사용
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); // 분주비 64 설정
}

// ADC 변환 시작 및 값 읽기
uint16_t ADC_read(uint8_t channel) {
    ADMUX = (ADMUX & 0xF8) | (channel & 0x07); // 채널 선택
    ADCSRA |= (1 << ADSC);           // 변환 시작
    while (ADCSRA & (1 << ADSC));    // 변환 완료 대기
    return ADC;
}
