#define F_CPU 1000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


ISR(INT0_vect) {
	while (!(UCSRA & (1 << UDRE))); 
	UDR = 'T';                     
}

void USART_Init(void) {
	UBRRH = (unsigned char)(6 >> 8);
	UBRRL = (unsigned char)6;
	UCSRB = (1 << RXEN) | (1 << TXEN);
	UCSRC = (1 << URSEL) | (3 << UCSZ0);
}

unsigned char USART_Receive(void) {
	while (!(UCSRA & (1 << RXC)));
	return UDR;
}

int main(void) {
	DDRC = 0xFF;         
	
	
	DDRD &= ~(1 << PD2);
	PORTD |= (1 << PD2);
	
	
	MCUCR |= (1 << ISC01); 
	GICR |= (1 << INT0);   
	sei();                 

	USART_Init();

	while (1) {
		
		uint8_t received_data = USART_Receive();

		
		uint8_t is_armed = received_data & 0x01;
		uint8_t pir_triggered = received_data & 0x02;
		uint8_t mag_triggered = received_data & 0x04;

		
		if (is_armed) {
			PORTC |= (1 << PC0);
			PORTC &= ~(1 << PC1);
			} else {
			PORTC &= ~(1 << PC0);
			PORTC |= (1 << PC1);
		}

		
		if (pir_triggered) PORTC |= (1 << PC2); else PORTC &= ~(1 << PC2);
		if (mag_triggered) PORTC |= (1 << PC3); else PORTC &= ~(1 << PC3);
	}
}
