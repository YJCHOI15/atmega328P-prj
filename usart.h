#ifndef USART_H
#define USART_H

#include <avr/io.h>
#include "seokmin.h"

void USART_init(unsigned int ubrr);
void USART_transmit(unsigned char data);
void USART_transmit_string(const char* str);
void USART_Switch_print_pin_states(uint32_t pin_states);
void USART_EEPROM_debugUserData(UserData *data);

#endif
