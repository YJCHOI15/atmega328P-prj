#define F_CPU 16000000UL // 16 MHz 클럭
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>


#include "seokmin.h"
/*
seokmin.h
ISR(INT1_vect)
seokmin_init_system
Switch(74HC165)
LED(74HC595)
Sleep
EEPROM
*/
#include "yongjin.h"
#include "soo.h"

#include "optsuyeon.h"

#include "usart.h"
#include "adc.h"
#include "inline_main.h"


extern volatile uint32_t Switch_newstate;

// 사용자 데이터 구조체
volatile UserData userData = {WATER_PRESSURE_LOW, TEMP_LOW, NOZZLE_POS_LOW, SEAT_TEMP_LOW};  // 초기값 설정
/*
typedef struct {
uint8_t water_pressure;
uint8_t temperature; //WATER_TEMP
uint8_t nozzle_position;
uint8_t seat_temperature;
} UserData;
*/
volatile uint32_t LED_data = 0;
volatile uint32_t LED_data_backup = 0;
volatile uint8_t Switch_previous_state = 0xFF;
extern volatile uint8_t selectedUser;

// ADC relay
volatile bool current_water_heater_relay = false;
volatile bool new_water_heater_relay = false;

volatile bool current_seat_heater_relay = false;
volatile bool new_seat_heater_relay = false;

volatile int32_t current_nozzle_position = NOZZLE_POS_LOW;

int main(void)
{
  char buffer1[32];
  char buffer2[32];
  /*seokmin init*/
//     // 시스템 초기화
//     seokmin_init_system();
    
//     // USART 초기화 (9600 baud rate 기준 UBRR 값 계산)
//     USART_init(103);  // 16MHz 클럭에서 9600bps

    // 스위치 초기 상태
    // 사람이 일어나 있음
    // PORTB &= ~(1 << Blink_LED_PIN);  // LED 끄기



  /* yongjin init */
    uint16_t fsr_value, water_value;

    
    
    setup_motors();        // 모터 핀 초기 설정
    INIT_TIMER1();         // 서보모터 PWM 설정
    set_servo_angle(0);    // 서보모터 초기 위치로 설정
    
    USART_init(103);  // 16MHz 클럭에서 9600bps
    ADC_init();
    
    seokmin_init_system();
    // USART 초기화 (9600 baud rate 기준 UBRR 값 계산)

  
    _delay_ms(1000);

    /* suhyeong */
    // setupPins();
  
    /*
    // 초기 sleep 판단
    fsr_value = ADC_read(PRESSURE_ADC_PIN);  
    // 수위 센서 값 읽기
    water_value = ADC_read(WATERLEVEL_ADC_PIN);
    // 센서 값에 따라 동작 결정
    check_pressure_and_water(fsr_value, water_value);
    */

    fsr_value = ADC_read(PRESSURE_ADC_PIN);
    // ISR Logic Change로 trigger 하기에 while에서는 불필요
    _delay_ms(1000);

    // 수위 센서 값 읽기
    water_value = ADC_read(WATERLEVEL_ADC_PIN);
    _delay_ms(1000);

    // 초기 sleep 판단
    // check_pressure(fsr_value);

    uint8_t sreg = SREG; // 현재 상태 레지스터 저장
    new_seat_heater_relay = 10;

    while (1)
    {
      // USART_transmit_string("*");
        /*yongjin test code */
        // 압력 센서 값 읽기 (FSR402)
         // 현재 상태 레지스터 저장
        // sreg = SREG;
        // cli();               // 인터럽트 비활성화
        fsr_value = ADC_read(PRESSURE_ADC_PIN);
        _delay_ms(1000);
        // SREG = sreg; // 이전 상태 복원 (자동으로 sei() 적용)
        // sei();
        // ISR Logic Change로 trigger 하기에 while에서는 불필요
        sprintf(buffer1, "압력센서 ADC: %u\r\n", fsr_value);
        USART_transmit_string(buffer1);
        _delay_ms(1000);  // 1초 대기
      
        // 수위 센서 값 읽기
        // sreg = SREG; // 현재 상태 레지스터 저장
        // cli();               // 인터럽트 비활성화
        water_value = ADC_read(WATERLEVEL_ADC_PIN);
        // _delay_ms(1000);
        // SREG = sreg; // 이전 상태 복원 (자동으로 sei() 적용)
        // sei();
        sprintf(buffer2, "수위센서 ADC: %u\r\n", water_value);
        USART_transmit_string(buffer2);
        // _delay_ms(1000);  // 1초 대기
        


        
        // 센서 값에 따라 동작 결정
        // check_pressure(fsr_value);
        // check_water_level(water_value); // 물부족 알림

        if(fsr_value <= PRESSURE_THRESHOLD) {
          // 자동 물내림
          // Servo Motor LED 대체
          LED_data |= (1UL << LED_SERVO_PIN);
          LED_74HC595_write(LED_data);
          LED_data &= ~(1UL << LED_SERVO_PIN);
          LED_74HC595_write(LED_data);

          _delay_ms(100);
          
            // 절전 모드 진입
            USART_transmit_string("절전 모드 진입\n");
            // 절전 모드 진입시 LED 모두 OFF
            LED_data_backup = LED_data;
            LED_data = 0;
            // 절전 LED
            LED_data |= (1UL << LED_POWER_SAVE_BIT);
            LED_74HC595_write(LED_data);
            Sleep_enter();
            // 복원
            LED_data = LED_data_backup;
            LED_74HC595_write(LED_data);
        }
        // else {
        //     // 절전 모드 해제
        //     USART_transmit_string("절전 모드 해제\n");
        //     // Sleep_exit();
        // }

        if(water_value <= WATER_THRESHOLD) {
            // 물을 채움
            // USART_transmit_string("물을 채움\n");
            // enter_refill_water_mode();
            // LED_BLINK_NO_WATER toggle
            Switch_newstate |= (1UL << LED_BLINK_NO_WATER);
            LED_data ^= (1UL << LED_BLINK_NO_WATER);
        }
        else {
            // 물 채우기 모드 해제
            // USART_transmit_string("물 채우기 모드 해제\n");
            // exit_refill_water_mode();
            // LED_BLINK_NO_WATER clear
            if (LED_data & (1UL << LED_BLINK_NO_WATER)){
              Switch_newstate |= (1UL << LED_BLINK_NO_WATER);
              LED_data &= ~(1UL << LED_BLINK_NO_WATER);
            }
        }




        /* seokmin test code */
        // 스위치 입력 확인
        if (Switch_newstate | 0x00000000) {
            // sreg = SREG; // 현재 상태 레지스터 저장
            // cli();  // 인터럽트 비활성화
            handleSwitchInput(Switch_newstate, &userData, &LED_data);  // 인라인 함수로 스위치 입력 처리
            updateLEDStatus(&userData, &LED_data);                    // 인라인 함수로 LED 상태 업데이트
            LED_74HC595_write(LED_data); // LED에 출력
            // Switch_previous_state = Switch_newstate; // 이전 상태 갱신
            Switch_newstate = 0; // 스위치 입력 초기화
            // SREG = sreg; // 이전 인터럽트 상태 복원
            // USART_transmit_string("v");
        }
      

        // /* suhyeong test code */
        new_water_heater_relay = controlWaterHeater(&userData);
        if(current_water_heater_relay != new_water_heater_relay) {
            Switch_newstate |= (1UL << WATER_RELAY_PIN);
            current_water_heater_relay = new_water_heater_relay;
            // USART_transmit_string("v");
        }
        // else{
        //     USART_transmit_string("x");
        // }
        _delay_ms(1000);

        new_seat_heater_relay = controlSeatHeater(&userData);
        // new_seat_heater_relay = !new_seat_heater_relay;
        if(current_seat_heater_relay != new_seat_heater_relay) {
            Switch_newstate |= (1UL << SEAT_RELAY_PIN);
            current_seat_heater_relay = new_seat_heater_relay;
            // USART_transmit_string("*");
        }
        _delay_ms(1000);

        // controlMotor(LED_data);


        // // 압력값 테스트
        // if (simulatedPressure < PRESSURE_THRESHOLD) {
        //     flush_toilet(180); // 서보모터를 180도로 회전하고 2초 대기 후 원위치
        //     _delay_ms(5000);          // 5초 대기 (다시 물 내림 방지)
        //     simulatedPressure = 50;   // 압력값 초기화
        //     USART_transmit_string("Pressure Threshold Reached. Flushed Toilet.\n");
        // }
        // set_servo_angle(0);

        if (current_nozzle_position != userData.nozzle_position) {
          int32_t diff = userData.nozzle_position - current_nozzle_position;
          if (diff > 0) {
            move_nozzle(diff, 1);   // 노즐 diff cm 전진
            USART_transmit_string("forward\n");
          } else {
            move_nozzle(-diff, 0);   // 노즐 diff cm 후진
            USART_transmit_string("backward\n");
          }
          current_nozzle_position = userData.nozzle_position;
        }

        // set_servo_angle(90);
        // USART_transmit_string("forward\n");
        // move_nozzle(50, 1);
        // USART_transmit_string("backward\n");
        // move_nozzle(50, 0);

        


        _delay_ms(1000);

    }
}
