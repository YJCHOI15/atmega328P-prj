#include "soo.h"
#include "adc.h"
#include "seokmin.h"
#include "inline_main.h"
#include "usart.h"

float R1 = 10000;  // R1 값을 실제로 정의합니다.
float temp_seat_setting = 30.0;
float temp_water_setting = 30.0;
bool motorState = false;
bool lastButtonState = false;



void UART_print_float(float num) {
	int int_part = (int)num;
	int frac_part = (int)((num - int_part) * 100);

	if (frac_part < 0) {
		frac_part = -frac_part;
	}

	char buffer[30];
	snprintf(buffer, sizeof(buffer), "%d.%02d", int_part, frac_part);
	USART_transmit_string(buffer);
}

float thermistor(float average) {
	float Temp = log(average / 10000.0);
	Temp = Temp / 3950.0;
	Temp = Temp + (1.0 / (25 + 273.15));
	Temp = 1.0 / Temp;
	Temp = Temp - 273.15;
	return Temp;
}

// void setupPins() {
// 	DDRD |= (1 << SEAT_RELAY_PIN) | (1 << DRY_MOTOR_PIN);
// 	DDRD &= ~(1 << DRY_BUTTON_PIN);
// 	PORTD |= (1 << DRY_BUTTON_PIN);
	
// 	DDRB &= ~((1 << SEAT_TEMP_UP_PIN) | (1 << SEAT_TEMP_DOWN_PIN));
// 	PORTB |= (1 << SEAT_TEMP_UP_PIN) | (1 << SEAT_TEMP_DOWN_PIN);
// }

bool controlWaterHeater(volatile UserData *Data) {
	uint16_t adc_water_value = ADC_read(ADC_WATER_PIN);
	float R3 = R1 * (1023.0 / adc_water_value - 1.0);
	float water_Temp = thermistor(R3);
	bool relay_state = false;

	if (water_Temp < Data->temperature) {
		relay_state = true;
		// *Switch_newstate |= (1 << WATER_RELAY_PIN);
		// USART_transmit_string("Water Heater: ON\n");
		}
	else {
		relay_state = false;
		// Switch_newstate &= ~(1 << WATER_RELAY_PIN);
		// USART_transmit_string("Water Heater: OFF\n");
	}
	// return water_Temp;
	USART_transmit_string("Current water Temp: ");
	UART_print_float(water_Temp);
	USART_transmit_string(" °C\n");

	return relay_state;
}

bool controlSeatHeater(volatile UserData *Data) {
	uint16_t adc_seat_value = ADC_read(ADC_SEAT_PIN);
	float R2 = R1 * (1023.0 / adc_seat_value - 1.0);
	float currentSeatTemp = thermistor(R2);
	bool relay_state = false;

	// if (currentSeatTemp < Data->seat_temperature) {
	// 	Switch_newstate |= (1 << SEAT_RELAY_PIN);
	// 	USART_transmit_string("Seat Heater: ON\n");
	// 	} 
	// 	else {
	// 	Switch_newstate &= ~(1 << SEAT_RELAY_PIN);
	// 	USART_transmit_string("Seat Heater: OFF\n");
	// 	}

	if (currentSeatTemp < Data->seat_temperature) {
		relay_state = true;
		// *Switch_newstate |= (1 << WATER_RELAY_PIN);
		// USART_transmit_string("Water Heater: ON\n");
		}
	else {
		relay_state = false;
		// Switch_newstate &= ~(1 << WATER_RELAY_PIN);
		// USART_transmit_string("Water Heater: OFF\n");
	}

	USART_transmit_string("Current Seat Temp: ");
	UART_print_float(currentSeatTemp);
	USART_transmit_string(" °C\n");

	return relay_state;
}

// void controlMotor(volatile uint32_t data) {
// 	if (data & (1 << LED_DRY_BIT)) {
// 		PORTD |= (1 << DRY_MOTOR_PIN);
// 		USART_transmit_string("MOTOR ON!!\n");
// 		} else {
// 		PORTD &= ~(1 << DRY_MOTOR_PIN);
// 		USART_transmit_string("MOTOR OFF!!\n");
// 	}
// }
