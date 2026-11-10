#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile uint8_t system_armed = 0;


ISR(INT0_vect) {
	system_armed = !system_armed;
}


ISR(USART_RXC_vect) {
	unsigned char received_cmd = UDR;
	if (received_cmd == 'T') {
		system_armed = !system_armed;
	}
}

void USART_Init(void) {
	UBRRH = (unsigned char)(6 >> 8);
	UBRRL = (unsigned char)6;
	UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
	UCSRC = (1 << URSEL) | (3 << UCSZ0);
}

void USART_Transmit(unsigned char data) {
	while (!(UCSRA & (1 << UDRE)));
	UDR = data;
}

int main(void) {
	DDRC = 0xFF;        
	DDRB |= (1 << PB0); 
	DDRA &= ~((1 << PA0) | (1 << PA1)); 
	
	
	DDRD &= ~(1 << PD2);
	PORTD |= (1 << PD2);
	
	
	MCUCR |= (1 << ISC01); 
	GICR |= (1 << INT0);  
	sei();                

	USART_Init();

	while (1) {
		
		uint8_t pir_triggered = (PINA & (1 << PA0)) ? 1 : 0;
		uint8_t mag_triggered = (PINA & (1 << PA1)) ? 1 : 0;

		
		if (pir_triggered) PORTC |= (1 << PC2); else PORTC &= ~(1 << PC2);
		if (mag_triggered) PORTC |= (1 << PC3); else PORTC &= ~(1 << PC3);

		
		uint8_t alarm_active = 0;
		if (system_armed == 1) {
			PORTC |= (1 << PC0);  
			PORTC &= ~(1 << PC1); 
			if (pir_triggered || mag_triggered) {
				alarm_active = 1; 
			}
			} else {
			PORTC &= ~(1 << PC0); 
			PORTC |= (1 << PC1);  
		}

		
		uint8_t tx_data = system_armed | (pir_triggered << 1) | (mag_triggered << 2);
		USART_Transmit(tx_data);
		
		
		if (alarm_active) {
			
			for(uint8_t i = 0; i < 25; i++) {
				PORTB |= (1 << PB0);  
				_delay_ms(1);
				PORTB &= ~(1 << PB0); 
				_delay_ms(1);
			}
			} else {
			PORTB &= ~(1 << PB0); 
			_delay_ms(50);        
		}
	}
}
