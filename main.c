/*
 * Thermometer based on NTC sensor
 * Author: Mikolaj Arciszewski
 */
#ifndef F_CPU
#define F_CPU 1000000UL
#endif
#include <math.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "lcd.h"

#define N_OF_SAMPLES 10
#define R_D 10000.0 //resistance of voltage divider
#define BETA 4150.0 //NTC material constant
#define T_0 298.15	//reference temperature
#define R_0 10000.0

double measurePress(void);

int main (void)
{
	double temperature = 0;
	char str[10];

	//lcd output
	DDRD = 0b01111111;
	lcd_init(LCD_DISP_ON);

	DDRC &=~ _BV(PC0);
	PORTC |= _BV(PC0);

	set_sleep_mode(SLEEP_MODE_ADC);

	//voltage reference - AVCC with external capacitor at AREF pin, input - ADC5
	ADMUX = _BV(REFS0)|_BV(MUX0)|_BV(MUX2);
	//ADC Enable, ADC Interrupt Enable, ADC Prescaler 64
	ADCSRA = _BV(ADEN)|_BV(ADIE)|_BV(ADPS2)|_BV(ADPS1);

	sei();
	while(1){
		temperature = measurePress();

		lcd_clrscr();
		sprintf(str, "%.2f", temperature);
		lcd_puts(str);
		lcd_putc(0xDF);
		lcd_putc('C');
		_delay_ms(500);
	}
	return 0;
}

ISR(ADC_vect){}

double measurePress(void){

	volatile int adcSamples[N_OF_SAMPLES];
	double adcAverage = 0,
			rThermistor;

	//sampling
	for (int i = 0; i<N_OF_SAMPLES; i++){
		ADCSRA |= _BV(ADSC);	//start convertion
		sleep_mode();			//noice reduction
		adcSamples[i] = ADC;
		adcAverage += (double)ADC;
		_delay_ms(10);
	}
	adcAverage = adcAverage / N_OF_SAMPLES;
	rThermistor = R_D * ( (1023 / adcAverage) - 1);
	//returns temperature in oC
	return ((BETA * T_0) / (BETA + (T_0 * log(rThermistor / R_0)))) - 273.15;
}
