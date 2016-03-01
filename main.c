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
static int8_t cpuM = 0;
static int8_t hddM = 0;

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

uint8_t get_temp_cpu()
{
	return 25;
}

uint8_t get_temp_hdd()
{
	return 25;
}

void update_chamber_hdd(uint8_t temp) {
	/* hddM range	buff	fan1	fan2	fan3
	 * 0	0<25	0	0	0
	 * 1	20-30	0	0	1
	 * 2	25-35	0	1	1
	 * 3	30-40	1	1	1
	 * 3	35<	PWM	PWM	PWM
	 *
	 */
	int8_t pwm = 0;
	switch(hddM) {
	case 0:
		PORTD &= ~(1 << PD0); //Fan6
		PORTD &= ~(1 << PD1); //Fan5
		PORTD &= ~(1 << PD2); //Fan4
		pwm = 0;
		if(temp >= 25)
			hddM = 1;
		break;
	case 1:
		PORTD |=  (1 << PD0); //Fan6
		PORTD &= ~(1 << PD1); //Fan5
		PORTD &= ~(1 << PD2); //Fan4
		pwm = 0;
		if(temp <= 20)
			hddM = 0;
		else if(temp >= 30)
			hddM = 2;
		break;
	case 2:
		PORTD |=  (1 << PD0); //Fan6
		PORTD |=  (1 << PD1); //Fan5
		PORTD &= ~(1 << PD2); //Fan4
		pwm = 0;
		if(temp <= 25)
			hddM = 1;
		else if(temp >= 35)
			hddM = 3;
		break;
	case 3:
		PORTD |=  (1 << PD0); //Fan6
		PORTD |=  (1 << PD1); //Fan5
		PORTD |=  (1 << PD2); //Fan4
		pwm = 0;
		if(temp <= 30)
			hddM = 2;
		if(temp >= 40)
			hddM = 4;
		break;
	case 4:
		PORTD |=  (1 << PD0); //Fan6
		PORTD |=  (1 << PD1); //Fan5
		PORTD |=  (1 << PD2); //Fan4
		pwm = (160/(100-40))*(temp-40);
		if(temp <= 35)
			hddM = 3;
		break;
	default:
		hddM=3;
		break;
	}
	if(pwm<0)
		pwm = 0;
	if(pwm>160)
		pwm = 160;
	OCR1B = pwm;
	OCR2B = pwm;
}

void update_chamber_cpu(uint8_t temp) {
	/* cpuM range	buff	fan1	fan2	fan3
	 * 0	0<35	0	0	0
	 * 1	30-40	0	0	1
	 * 2	35-45	0	1	1
	 * 3	40-50	1	1	1
	 * 3	45<	PWM	PWM	PWM
	 *
	 */
	int8_t pwm = 0;
	switch(cpuM) {
	case 0:
		PORTB &= ~(1 << PB0); //Fan3
		PORTD &= ~(1 << PD7); //Fan2
		PORTB &= ~(1 << PB6); //Fan1
		pwm = 0;
		if(temp >= 35)
			cpuM = 1;
		break;
	case 1:
		PORTB |=  (1 << PB0); //Fan3
		PORTD &= ~(1 << PD7); //Fan2
		PORTB &= ~(1 << PB6); //Fan1
		pwm = 0;
		if(temp <= 30)
			cpuM = 0;
		else if(temp >= 40)
			cpuM = 2;
		break;
	case 2:
		PORTB |=  (1 << PB0); //Fan3
		PORTD |=  (1 << PD7); //Fan2
		PORTB &= ~(1 << PB6); //Fan1
		pwm = 0;
		if(temp <= 35)
			cpuM = 1;
		else if(temp >= 45)
			cpuM = 3;
		break;
	case 3:
		PORTB |=  (1 << PB0); //Fan3
		PORTD |=  (1 << PD7); //Fan2
		PORTB |=  (1 << PB6); //Fan1
		pwm = 0;
		if(temp <= 40)
			cpuM = 2;
		if(temp >= 50)
			cpuM = 4;
		break;
	case 4:
		PORTB |=  (1 << PB0); //Fan3
		PORTD |=  (1 << PD7); //Fan2
		PORTB |=  (1 << PB6); //Fan1
		//160/(100-45)*(temp-45)
		pwm = (160/(100-50))*(temp-50);
		//pwm = 2.909*temp-130.909;
		if(temp <= 45)
			cpuM = 3;
		break;
	default:
		cpuM=3;
		break;
	}
	if(pwm<0)
		pwm = 0;
	if(pwm>160)
		pwm = 160;
	OCR0B = pwm;
	OCR1A = pwm;
}

void test_temp()
{
	uint8_t temp=15;
	while(1) {
		for(temp = 15; temp < 60; temp++) {
			update_chamber_hdd(temp);
			update_chamber_cpu(temp);
			_delay_ms(300);
		}
		for(temp = 60; temp > 15; temp--) {
			update_chamber_hdd(temp);
			update_chamber_cpu(temp);
			_delay_ms(300);
		}
	}
}

int main(void)
{
	uint8_t temp;
	init();
	test_temp();
	while(1) {
		temp = get_temp_hdd();
		update_chamber_hdd(temp);
		temp = get_temp_cpu();
		update_chamber_cpu();
		_delay_ms(1000);
	}
	return 1;
}
