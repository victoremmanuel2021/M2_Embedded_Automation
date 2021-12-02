#ifndef_PROJECT_WakeupRing_H_
#define_PROJECT_WakeupRing_H_
#define F_CPU 8000000ul
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
 
/***MACROS*/
 
#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
 
#define uchar unsigned char
#define uint unsigned int
 
#define LCDPORTDIR DDRB
#define LCDPORT PORTB
#define rs 0
#define rw 1
#define en 2
 
#define RSLow (LCDPORT&=~(1<<rs))
#define RSHigh (LCDPORT|=(1<<rs))
#define RWLow (LCDPORT&=~(1<<rw))
#define ENLow (LCDPORT&=~(1<<en))
#define ENHigh (LCDPORT|=(1<<en))
 
#define KeyPORTdir DDRA
#define key PINA
#define KeyPORT PORTA
 
#define OK 3
#define UP 0
#define DOWN 1
#define DEL 3
#define MATCH 1
#define ENROL 2
 
#define enrol (key & (1<<ENROL))   // key 1
#define match (key & (1<<MATCH))   // key 4
#define delet (key & (1<<DEL))   // key 2
#define up (key & (1<<UP))     // key 3
#define down (key & (1<<DOWN)) // key 4
#define ok (key & (1<<OK))      // key 2
 
#define LEDdir DDRC
#define LEDPort PORTC
 
#define LED 3
#define BUZ 2
 
#define LEDHigh (LEDPort += (1<<LED))  
#define LEDLow (LEDPort &= ~(1<<LED))
 
#define BUZHigh (LEDPort += (1<<BUZ))
#define BUZLow (LEDPort &= ~(1<<BUZ))
 
#define HIGH 1
#define LOW 0
 
#define PASS 0
#define ERROR 1
 
#define check(id) id=up<down?++id:down<up?--id:id;
 
#define maxId 5
#define dataLenth 6
#define eepStartAdd 10