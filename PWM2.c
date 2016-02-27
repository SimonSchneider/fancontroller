/*
 * fanController.c
 *
 * Created: 2016-01-30
 * Author: Simon Schneider
 */ 

/*
ATmega8, 48, 88, 168, 328

/Reset PC6|1   28|PC5
       PD0|2   27|PC4
       PD1|3   26|PC3       LCD_RW ( 6)
       PD2|4   25|PC2       LCD_E  ( 5)
       PD3|5   24|PC1       LCD_RS ( 3)
       PD4|6   23|PC0 
       Vcc|7   22|Gnd
       Gnd|8   21|Aref
       PB6|9   20|AVcc
       PB7|10  19|PB5 SCK   LCD_D7 (13)
  OC0B PD5|11  18|PB4 MISO  LCD_D6 (14)
  OC0A PD6|12  17|PB3 MOSI  LCD_D5 (11)
       PD7|13  16|PB2       LCD_D4 (12)
       PB0|14  15|PB1       switch (17)
*/

#define F_CPU 4000000UL  // 4 MHz standard clock

#include <avr/power.h>
#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd.h"
volatile uint16_t rpm;
volatile uint16_t fan;

ISR(TIMER0_COMPA_vect)
{
  static uint16_t n;
  n++;
  if(n = 16383)
  {
    rpm = 91.62 * fan;
    fan = 0;
  }
}

ISR(INT0_vect)
{
  fan++;
}

void init(void)
{
  //set clock speed to 4 MHz
  clock_prescale_set(clock_div_2);

  //init LCD screen
  lcd_init(LCD_DISP_ON);
  lcd_clrscr();

  //IN  (PD0 PD1 - buttons) (PD2 - RPM interrupt)
  PORTD = (1 << PD0) | (1 << PD1) | (1 << PD2);
  //OUT (PD5 PD6 - PWM output)
  DDRD  = (1 << PD5) | (1 << PD6);

  //Interrupts for RPM on int0 (PD2) on rising edge
  EICRA = (0 << ISC11) | (0 << ISC10)
	| (1 << ISC01) | (1 << ISC00);
  EIMSK = (0 << INT1)  | (1 << INT0);

  //PWM setup at 25kHz
  TCCR0A = (1 << COM0A1) | (0 << COM0A0) 
         | (1 << COM0B1) | (0 << COM0B0)
         | (1 << WGM01)  | (1 << WGM00);
  TCCR0B = (1 << WGM02)
         | (0 << CS02)   | (0 << CS01)   | (1 << CS00);
  TIMSK0 = (0 << OCIE0B) | (1 << OCIE0A) | (0 << TOIE0);
  //output compare reg A (PD6), output reg B (PD5)
  OCR0A  = 160;
  OCR0B  = 1;
}

void pwm_button(void)
{
  uint8_t press;
  uint8_t pwm_value = 0;
  char textbuffer[16];

  while(1)
  {
    OCR0B = pwm_value;
    if(pwm_value == 161)
    {
      pwm_value = 160;
    } else if(pwm_value > 161)
    {
      pwm_value = 0;
    }
    sprintf(textbuffer, "RPM value = %4u", rpm);
    lcd_gotoxy(0,0);
    lcd_puts(textbuffer);
    sprintf(textbuffer, "PWM value = %4u", pwm_value);
    lcd_gotoxy(0,1);
    lcd_puts(textbuffer);
    press = (PIND & 0b00000011);
    switch(press)
    {
      case 1:
        pwm_value++;
        break;
      case 2:
        pwm_value--;
        break;
      default:
        break;
    }
    _delay_ms(50);
  }
}

int main(void)
{
  init();
  pwm_button();
}
