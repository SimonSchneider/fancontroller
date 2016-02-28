/*
 * fanController.c
 *
 * Created: 2016-01-30
 * Author: Simon Schneider
 */

/*
ATmega8, 48, 88, 168, 328

/Reset	PC6|1	28|PC5
B_down	PD0|2	27|PC4
B_up	PD1|3	26|PC3		LCD_RW ( 6)
	PD2|4	25|PC2		LCD_E  ( 5)
	PD3|5	24|PC1		LCD_RS ( 3)
	PD4|6	23|PC0
	Vcc|7	22|Gnd
	Gnd|8	21|Aref
	PB6|9	20|AVcc
	PB7|10	19|PB5	SCK	LCD_D7 (13)
OC0B	PD5|11	18|PB4	MISO	LCD_D6 (14)
OC0A	PD6|12	17|PB3	MOSI	LCD_D5 (11)
	PD7|13	16|PB2		LCD_D4 (12)
	PB0|14	15|PB1		switch (17)
*/

#define F_CPU 4000000UL  // 4 MHz standard clock

#include <avr/power.h>
#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay.h>
#include <inttypes.h>

void init(void)
{
	//set clock speed to 4 MHz
	clock_prescale_set(clock_div_2);

	//IN	PC4 TempSensor 2, PC5 TempSensor1
	PORTC	= (1 << PC4)	| (1 << PC5);
	//OUT	PB0 PowFan3	PB1 PWMFan3	PB2 PowFan4
	//	PB6 PowFan1
	DDRB	= (1 << PB0)	| (1 << PB1)	| (1 << PB2)
		| (1 << PB6);
	//OUT	PD0 PowFan6	PD1 PowFan5	PD2 PowFan4
	//	PD3 PWMFan5+6	PD5 PWMFan1+2	PD7 PowFan2
	DDRD	= (1 << PD0)	| (1 << PD1)	| (1 << PD2)
		| (1 << PD3)	| (1 << PD5)	| (1 << PD7);

	//Timer0 PWM setup @25kHz for Fan1+2
	TCCR0A	= (1 << COM0A1)	| (0 << COM0A0)
		| (1 << COM0B1) | (0 << COM0B0)
		| (1 << WGM01)	| (1 << WGM00);
	TCCR0B	= (1 << WGM02)
		| (0 << CS02)	| (0 << CS01)	| (1 << CS00);
	OCR0A	= 160;
	OCR0B	= 1;	//PD5

	//Timer1 PWM setup @25kHz for Fan3 and Fan4
	TCCR1A	= (1 << COM1A1) | (0 << COM1A0)
		| (1 << COM1B1) | (0 << COM1B1)
		| (1 << WGM11)	| (0 << WGM10);
	TCCR1B	= (1 << WGM13)	| (1 << WGM12)
		| (0 << CS12)	| (0 << CS11)	| (1 << CS00);
	ICR1	= 160;
	OCR1A	= 1;	//PB1
	OCR1B	= 1;	//PB2

	//Timer2 PWM setup @25kHz for Fan5+6
	TCCR2A	= (1 << COM2A1)	| (0 << COM2A0)
		| (1 << COM2B1)	| (0 << COM2B0)
		| (1 << WGM21)	| (1 << WGM20);
	TCCR2B	= (1 << WGM22)
		| (0 << CS22)	| (0 << CS21)	| (1 << CS20);
	OCR2A	= 160;
	OCR2B	= 1;	//PD3
}

void set_power(uint8_t a[])
{
	if(a[0] == 0)
		PORTB &= ~(1 << PB6);
	else
		PORTB |= (1 << PB6);
	if(a[1] == 0)
		PORTD &= ~(1 << PD7);
	else
		PORTD |= (1 << PD7);
	if(a[2] == 0)
		PORTB &= ~(1 << PB0);
	else
		PORTB |= (1 << PB0);
	if(a[3] == 0)
		PORTD &= ~(1 << PD2);
	else
		PORTD |= (1 << PD2);
	if(a[4] == 0)
		PORTD &= ~(1 << PD1);
	else
		PORTD |= (1 << PD1);
	if(a[5] == 0)
		PORTD &= ~(1 << PD0);
	else
		PORTD |= (1 << PD0);
}

void test(void)
{
	uint8_t pwm12	= 0;
	uint8_t pwm3	= 0;
	uint8_t pwm4	= 0;
	uint8_t pwm56	= 0;
	uint8_t pow[6]	= {0, 0, 0, 0 ,0, 0};

	while(1) {
		if(pwm12 > 160)
			pwm12 = 0;
		if(pwm3 > 160)
			pwm3 = 0;
		if(pwm4 > 160)
			pwm4 = 0;
		if(pwm56 > 160)
			pwm56 = 0;
		OCR0B = pwm12;
		OCR1A = pwm3;
		OCR1B = pwm4;
		OCR2B = pwm56;
		set_power(pow);
		_delay_ms(100);
		if(pow[0] == 1) {
			pow[0] = 0;
			pow[1] = 0;
			pow[2] = 0;
			pow[3] = 0;
			pow[4] = 0;
			pow[5] = 0;
		} else {
			pow[0] = 1;
			pow[1] = 1;
			pow[2] = 1;
			pow[3] = 1;
			pow[4] = 1;
			pow[5] = 1;
		}
		pwm12++;
		pwm3++;
		pwm4++;
		pwm56++;
	}
}

int main(void)
{
	init();
	test();
}
