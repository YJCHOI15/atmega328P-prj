#ifndef INLINE_MAIN_H
#define INLINE_MAIN_H

#include <avr/io.h>
#include "seokmin.h"
#include <stdio.h>


// Define the levels for temperature, water pressure, and nozzle position
#define TEMP_LOW 20
#define TEMP_MEDIUM 30
#define TEMP_HIGH 40
#define TEMP_INTERVAL 10

#define WATER_PRESSURE_LOW 20
#define WATER_PRESSURE_MEDIUM 30
#define WATER_PRESSURE_HIGH 40
#define WATER_PRESSURE_INTERVAL 10

#define NOZZLE_POS_LOW 0
#define NOZZLE_POS_MEDIUM 50 * 7
#define NOZZLE_POS_HIGH 50 * 14
#define NOZZLE_POS_INTERVAL 50 * 7

#define SEAT_TEMP_LOW 20
#define SEAT_TEMP_MEDIUM 30
#define SEAT_TEMP_HIGH 40
#define SEAT_TEMP_INTERVAL 10

// Switch and LED bit positions
#define LED_POWER_SAVE_BIT 0
#define LED_SPRAY_BIT 1
#define LED_DRY_BIT 2

// Relay pin
#define WATER_RELAY_PIN 17 // Switch 갱신용
#define SEAT_RELAY_PIN 18

#define LED_WATER_RELAY_PIN 17
#define LED_SEAT_RELAY_PIN 18

#define LED_BLINK_NO_WATER 19

#define LED_SERVO_PIN 20




extern volatile uint8_t selectedUser;

extern volatile bool current_water_heater_relay;
extern volatile bool current_seat_heater_relay;



// Inline function to handle switch input
static inline void handleSwitchInput(uint32_t switchState, volatile UserData *userData, volatile uint32_t *LED_data) {
    if (switchState & 0x0F) {  // Process switches 0-3 for temperature and water pressure
        if (switchState & (1 << 0) && userData->temperature < TEMP_HIGH) userData->temperature += TEMP_INTERVAL;
        else if (switchState & (1 << 1) && userData->temperature > TEMP_LOW) userData->temperature -= TEMP_INTERVAL;
        else if (switchState & (1 << 2) && userData->water_pressure < WATER_PRESSURE_HIGH) userData->water_pressure += WATER_PRESSURE_INTERVAL;
        else if (switchState & (1 << 3) && userData->water_pressure > WATER_PRESSURE_LOW) userData->water_pressure -= WATER_PRESSURE_INTERVAL;
    }
    
    if (switchState & 0xF0) {  // Process switches 4-7 for seat temperature and nozzle length
        if (switchState & (1 << 4) && userData->seat_temperature < SEAT_TEMP_HIGH) userData->seat_temperature += SEAT_TEMP_INTERVAL;
        else if (switchState & (1 << 5) && userData->seat_temperature > SEAT_TEMP_LOW) userData->seat_temperature -= SEAT_TEMP_INTERVAL;
        else if (switchState & (1 << 6) && userData->nozzle_position < NOZZLE_POS_HIGH) userData->nozzle_position += NOZZLE_POS_INTERVAL;
        else if (switchState & (1 << 7) && userData->nozzle_position > NOZZLE_POS_LOW) userData->nozzle_position -= NOZZLE_POS_INTERVAL;
    }
    
    // Handle toggling operations (Switches 8-12)
    if (switchState & (1 << 8)) *LED_data ^= (1 << LED_SPRAY_BIT);  // Toggle spray LED
    if (switchState & (1 << 9)) *LED_data ^= (1 << LED_DRY_BIT);    // Toggle dry LED // LED와 Relay 같이 결선
    
    // Handle user switching
    // if (switchState & (1 << 10)) {
    //     selectedUser = (selectedUser + 1) % 2;
    //     *LED_data = (*LED_data & ~((uint32_t)0b11 << 16)) | ((uint32_t)1 << (16 + selectedUser));
    // }
    
    // Handle saving and loading user settings
    if (switchState & (1 << 10)) {
        EEPROM_saveUserData(userData, 0);
        *LED_data = (*LED_data | ((uint32_t)1 << 15)) & ~((uint32_t)1 << 16);
    }
    if (switchState & (1 << 11)) {
        EEPROM_saveUserData(userData, 1);
        *LED_data = (*LED_data | ((uint32_t)1 << 16)) & ~((uint32_t)1 << 15);
    }

    if (switchState & (1 << 12)) {
        EEPROM_loadUserData(userData, 0);
        *LED_data = (*LED_data | ((uint32_t)1 << 15)) & ~((uint32_t)1 << 16);
    }
    if (switchState & (1 << 13)) {
        EEPROM_loadUserData(userData, 1);
        *LED_data = (*LED_data | ((uint32_t)1 << 16)) & ~((uint32_t)1 << 15);
    }


    
    if(switchState & ((uint32_t)1 << WATER_RELAY_PIN)) {
        if (current_water_heater_relay == true) {
            *LED_data |= ((uint32_t)1 << LED_WATER_RELAY_PIN);
            // USART_transmit_string("Water Heater: ON\n");
        }
        else {
            *LED_data &= ~((uint32_t)1 << LED_WATER_RELAY_PIN);
            // USART_transmit_string("Water Heater: OFF\n");
        }
    }

    if(switchState & ((uint32_t)1 << SEAT_RELAY_PIN)) {
        if (current_seat_heater_relay == true) {
            *LED_data |= ((uint32_t)1 << LED_SEAT_RELAY_PIN);
            // USART_transmit_string("Water Heater: ON\n");
        }
        else {
            *LED_data &= ~((uint32_t)1 << LED_SEAT_RELAY_PIN);
            // USART_transmit_string("Water Heater: OFF\n");
        }
    }


}

// Inline function to update LED status
static inline void updateLEDStatus(volatile const UserData *userData, volatile uint32_t *LED_data) {
    // 비트 클리어
    *LED_data &= ~(((uint32_t)0b111 << 3) | ((uint32_t)0b111 << 6) | ((uint32_t)0b111 << 9) | ((uint32_t)0b111 << 12) | ((uint32_t)0b11 << 17));

    // Update temperature LED (3~5번 LED)
    *LED_data |= ((userData->temperature >= TEMP_HIGH) ? (1 << 3) : 
                  (userData->temperature >= TEMP_MEDIUM) ? (1 << 4) : (1 << 5));

    // Update water pressure LED (6~8번 LED)
    *LED_data |= ((userData->water_pressure >= WATER_PRESSURE_HIGH) ? (1 << 6) :
                  (userData->water_pressure >= WATER_PRESSURE_MEDIUM) ? (1 << 7) : (1 << 8));

    // Update seat temperature LED (9~11번 LED)
    *LED_data |= ((userData->seat_temperature >= TEMP_HIGH) ? (1 << 9) : 
                  (userData->seat_temperature >= TEMP_MEDIUM) ? (1 << 10) : (1 << 11));

    // Update nozzle position LED (12~15번 LED)
    *LED_data |= ((userData->nozzle_position >= NOZZLE_POS_HIGH) ? (1 << 12) :
                  (userData->nozzle_position >= NOZZLE_POS_MEDIUM) ? (1 << 13) : (1 << 14));

    // Current selected user display (15~16번 LED)
    // uint8_t selectedUser = (*LED_data >> 16) & 0b11; // Extract the current selected user bits
    // *LED_data |= (1 << (16 + selectedUser));

    // water temperature relay
	if (current_water_heater_relay == true) {
		*LED_data |= (1UL << LED_WATER_RELAY_PIN);
		// USART_transmit_string("Water Heater: ON\n");
    }
    // else {
	// 	*LED_data &= ~(1 << WATER_RELAY_PIN);
	// 	// USART_transmit_string("Water Heater: OFF\n");
	// }

    if(current_seat_heater_relay == true) {
        *LED_data |= (1UL << LED_SEAT_RELAY_PIN);
        // USART_transit_string("Seat Heater: ON\n");
    }
}

#endif // INLINE_MAIN_H
