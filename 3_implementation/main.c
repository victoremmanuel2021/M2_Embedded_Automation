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
 
/*variable*/
 
uchar buf[20];
uchar buf1[20];
volatile uint ind;
volatile uint flag;
uint msCount=0;
uint g_timerflag=1;
volatile uint count=0;
uchar data[10];
uint id=1;
int s,a,b,c;
 
const char passPack[]={0xEF, 0x1, 0xFF, 0xFF, 0xFF, 0xFF, 0x1, 0x0, 0x7, 0x13, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1B};
const char f_detect[]={0xEF, 0x1, 0xFF, 0xFF, 0xFF, 0xFF, 0x1, 0x0, 0x3, 0x1, 0x0, 0x5};
const char f_imz2ch1[]={0xEF, 0x1, 0xFF, 0xFF, 0xFF, 0xFF, 0x1, 0x0, 0x4, 0x2, 0x1, 0x0, 0x8};
const char f_imz2ch2[]={0xEF, 0x1, 0xFF, 0xFF, 0xFF, 0xFF, 0x1, 0x0, 0x4, 0x2, 0x2, 0x0, 0x9};
const char f_createModel[]={0xEF,0x1,0xFF,0xFF,0xFF,0xFF,0x1,0x0,0x3,0x5,0x0,0x9};
char f_storeModel[]={0xEF,0x1,0xFF,0xFF,0xFF,0xFF,0x1,0x0,0x6,0x6,0x1,0x0,0x1,0x0,0xE};
const char f_search[]={0xEF, 0x1, 0xFF, 0xFF, 0xFF, 0xFF, 0x1, 0x0, 0x8, 0x1B, 0x1, 0x0, 0x0, 0x0, 0xA3, 0x0, 0xC8};
char f_delete[]={0xEF,0x1,0xFF,0xFF,0xFF,0xFF,0x1,0x0,0x7,0xC,0x0,0x0,0x0,0x1,0x0,0x15};
//const char f_readNotepad[]={0xEF,0x1,0xFF,0xFF,0xFF,0xFF,0x1,0x0,0x4,0x19,0x0,0x0,0x1E};
//char f_writeNotepad[]={0xEF,0x1,0xFF,0xFF,0xFF,0xFF,0x1,0x0,0x24};
 
int timeStamp[7],day;
 
enum
{
 CMD=0,
 DATA, 
};
 
void buzzer(uint);
 
void lcdwrite(char ch,char r)
{
     LCDPORT=ch & 0xF0;
RWLow;
     if(r == 1)
RSHigh;
else 
RSLow;
     ENHigh;
     _delay_ms(5);
ENLow;
_delay_ms(10);
  
     LCDPORT=ch<<4 & 0xF0;
RWLow;
if(r == 1)
RSHigh;
else
RSLow;
ENHigh;
_delay_ms(5);
ENLow;
_delay_ms(10); 
}
 
void lcdprint(char *str)
{
    while(*str)
    {
        lcdwrite(*str++,DATA);
        //__delay_ms(20);
    }
}
 
void lcdbegin()
{
    uchar lcdcmd[5]={0x02,0x28,0x0E,0x06,0x01};
    uint i=0;
    for(i=0;i<5;i++)
    lcdwrite(lcdcmd[i], CMD);
}
 
void serialbegin()
{
  UCSRC = (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1);
  UBRRH = (BAUD_PRESCALE >> 8);
  UBRRL = BAUD_PRESCALE;
  UCSRB=(1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
  sei();
}
 
ISR(USART_RXC_vect)
{
char ch=UDR;
buf[ind++]=ch;
if(ind>0)
flag=1;
//serial1Write(ch);
}
 
void serialwrite(char ch)
{
while ((UCSRA & (1 << UDRE)) == 0);
UDR = ch;
}
 
void serialprint(char *str)
{
    while(*str)
    {
        serialwrite(*str++);
    }
}
 
void serialprintln(char *str)
{
serialprint(str);
serialwrite(0x0d);
serialwrite(0x0a);
}
 
void serialFlush()
{
    for(int i=0;i<sizeof(buf);i++)
    {
        buf[i]=0;
    }
}
 
void SerialSoftWrite(char ch)
{
PORTD&=~(1<<7);
_delay_us(104);
for(int i=0;i<8;i++)
{
if(ch & 1)
PORTD|=(1<<7);
else
PORTD&=~(1<<7);
_delay_us(104);
ch>>=1;
}
PORTD|=(1<<7);
_delay_us(104);
}
 
void SerialSoftPrint(char *str)
{
while(*str)
{
SerialSoftWrite(*str);
str++;
}
}
 
void SerialSoftPrintln(char *str)
{
SerialSoftPrint(str);
SerialSoftWrite(0x0D);
SerialSoftWrite(0x0A);
}
 
int bcdtochar(char num)
{
return ((num/16 * 10) + (num % 16));
}
 
void RTC_start()
{
TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
while((TWCR&0x80)==0x00);
}
 
void RTC_stp()
{ 
TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);           //stop communication
}
 
void RTC_read()
{ 
TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
while((TWCR&0x80)==0x00);
TWDR=0xD0;                                         //RTC write (slave address)
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT)));
TWDR=0x00;                                         //RTC write (word address)
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT)));
TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);              //start RTC  communication again
while ((TWCR&0x80)==0x00);
TWDR=0xD1;                                        // RTC command to read
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT)));  
}
 
void sec_init(unsigned char d)
{  
TWDR=d;                                       //second init
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT)));  
}
 
void min_init(unsigned char d)
{  
TWDR=d;                                       //minute init
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT))); 
}
 
void hr_init(unsigned char d)
{ 
TWDR=d;                                        //hour init
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT))); 
}
 
void day_init(unsigned char d)
{ 
TWDR=d;                                          //days init
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT))); 
}
 
void date_init(unsigned char d)
{ 
TWDR=d;                                          //date init
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT))); 
}
 
void month_init(unsigned char d)
{ 
TWDR=d;                                         //month init
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT))); 
}
 
void yr_init(unsigned char d)
{ 
TWDR=d;                                         //year init
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT))); 
}
 
int sec_rw()
{
TWCR|=(1<<TWINT)|(1<<TWEA);                         //RTC second read
while((TWCR & 0x80)==0x00);
return bcdtochar(TWDR);
}
 
int min_rw()
{ 
TWCR|=(1<<TWINT);                                   //RTC minute read
TWCR|=(1<<TWEA);
while((TWCR & 0x80)==0x00);
return bcdtochar(TWDR);
}
 
int hr_rw()
{ 
TWCR|=(1<<TWINT)|(1<<TWEA);                         //RTC hour read
while((TWCR & 0x80)==0x00);
return bcdtochar(TWDR);
}
 
int day_rd()
{ 
TWCR|=(1<<TWINT)|(1<<TWEA);                         //RTC day read
while((TWCR&0x80)==0x00);
return bcdtochar(TWDR);
}
 
int date_rw()
{ 
TWCR|=(1<<TWINT)|(1<<TWEA);                      //RTC date read
while((TWCR & 0x80)==0x00);
return bcdtochar(TWDR);
}
 
int month_rw()
{ 
TWCR|=(1<<TWINT)|(1<<TWEA);                     //RTC month read
while((TWCR & 0x80)==0x00);
return bcdtochar(TWDR);
}
 
int yr_rw()
{ 
TWCR|=(1<<TWINT);                                 //RTC year read
TWCR&=(~(1<<TWEA));
while((TWCR & 0x80)==0x00);
return bcdtochar(TWDR);
}
 
void device()
{
TWDR=0xD0;                                         //RTC write (slave address)
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT)));
 
TWDR=0x00;                                        // word address write
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT)));
}
 
void RTCTimeSet()
{
RTC_start();
device();
sec_init(0);
min_init(0x47);
hr_init(0x22);
day_init(0x03);
date_init(0x23);
month_init(0x08);
yr_init(0x19);
RTC_stp();
}
 
void show()
{
char tem[20];
sprintf(tem,"%d",timeStamp[0]);
lcdwrite(0x80,CMD);
lcdprint("Time:");
lcdprint(tem);
lcdwrite(':',DATA);
sprintf(tem,"%d",timeStamp[1]);
lcdprint(tem);
lcdwrite(':',DATA);
sprintf(tem,"%d",timeStamp[2]);
lcdprint(tem);
lcdprint("  ");
lcdwrite(0xc0,CMD);
lcdprint("Date:");
sprintf(tem,"%d",timeStamp[3]);
lcdprint(tem);
lcdwrite('/',DATA);
sprintf(tem,"%d",timeStamp[4]);
lcdprint(tem);
lcdwrite('/',DATA);
sprintf(tem,"%d",timeStamp[5]);
lcdprint("20");
if(timeStamp[5]<10)
lcdwrite('0',DATA);
lcdprint(tem);
lcdprint("   ");
}
 
void RTC()
{
RTC_read();
timeStamp[2]=sec_rw();
timeStamp[1]=min_rw();
timeStamp[0]=hr_rw();
day=day_rd();
timeStamp[3]=date_rw();
timeStamp[4]=month_rw();
timeStamp[5]=yr_rw();
RTC_stp();
show();
}
 
int eeprom_write(unsigned int add,unsigned char data)
{
while(EECR&(1<<EEWE));
EEAR=add;
EEDR=data;
EECR|=(1<<EEMWE);
EECR|=(1<<EEWE);
return 0;
}
char eeprom_read(unsigned int add)
{
while(EECR & (1<<EEWE));
EEAR=add;
EECR|=(1<<EERE);
return EEDR;
}
 
void saveData(int id)
{
uint cIndex= eeprom_read(id);
if(cIndex == 0)
cIndex=1;
uint cAddress= (cIndex*6) + (id-1)*48;
 
for(int i=0;i<6;i++)
eeprom_write(cAddress+i,timeStamp[i]);
eeprom_write(id,cIndex+1);
}
 
int sendcmd2fp(char *pack, int len)
{
  int res=ERROR;
  serialFlush();
  ind=0;
  _delay_ms(100);
  for(int i=0;i<len;i++)
  {
    serialwrite(*(pack+i));
  }
  _delay_ms(1000);
  if(flag == 1)
  {
    if(buf[0] == 0xEF && buf[1] == 0x01)
    {
        if(buf[6] == 0x07)   // ack
        {
        if(buf[9] == 0)
        {
            uint data_len= buf[7];
            data_len<<=8;
            data_len|=buf[8];
            for(int i=0;i<data_len;i++)
                data[i]=0;
            //data=(char *)calloc(data_len, sizeof(data));
            for(int i=0;i<data_len-2;i++)
            {
                data[i]=buf[10+i];
            }
            res=PASS;
        }
 
        else
        {
         res=ERROR;
        }
        }
    }
    ind=0;
    flag=0;
    return res;
}
return res;
}
 
uint getId()
{
    uint id=0;
    lcdwrite(1, CMD);
    while(1)
    {
        //check(id);
if(up == LOW)
{
id++;
buzzer(200);
}
else if(down == LOW)
{
id--;
if(id==0)
id=0;
buzzer(200);
}
        else if(ok == LOW)
{
buzzer(200);
            return id;
}
lcdwrite(0x80, CMD);
(void)sprintf((char *)buf1,"Enter Id:%d  ",id);
lcdprint((char *)buf1);
_delay_ms(200);
    }
}
 
void matchFinger()
{
    //  lcdwrite(1,CMD);
    //  lcdprint("Place Finger"); 
    //  lcdwrite(192,CMD);
    //  _delay_ms(2000);
     if(!sendcmd2fp((char *)&f_detect[0],sizeof(f_detect)))
     {
         if(!sendcmd2fp((char *)&f_imz2ch1[0],sizeof(f_imz2ch1)))
         {
            if(!sendcmd2fp((char *)&f_search[0],sizeof(f_search)))
            {
LEDHigh;
buzzer(200);
                uint id= data[0];
                     id<<=8;
                     id+=data[1];
                uint score=data[2];
                        score<<=8;
                        score+=data[3];
                (void)sprintf((char *)buf1,"Id: %d",(int)id);
                lcdwrite(1,CMD);
                lcdprint((char *)buf1);
 
saveData(id);
 
_delay_ms(1000);
lcdwrite(1,CMD);
lcdprint("Attendance");
lcdwrite(192,CMD);
lcdprint("Registered");
_delay_ms(2000);
LEDLow;
            }
            
            else
            {
LEDHigh;
                lcdwrite(1,CMD);
                lcdprint("Not Found");
buzzer(5000);
LEDLow;
            }
         }
else
{
LEDHigh;
lcdwrite(1,CMD);
lcdprint("Not Found");
buzzer(2000);
LEDLow;
}
     }
      
     else
     {
         //lcdprint("No Finger"); 
     }
      //_delay_ms(200);
}
 
void enrolFinger()
{
     lcdwrite(1,CMD);
     lcdprint("Enroll Finger");
     _delay_ms(2000);
     lcdwrite(1,CMD);
     lcdprint("Place Finger"); 
     lcdwrite(192,CMD);
     _delay_ms(1000);
for(int i=0;i<3;i++)
{
     if(!sendcmd2fp((char *)&f_detect[0],sizeof(f_detect)))
     {
         //lcdprint("Finger Detected");
         //__delay_ms(1000);
        if(!sendcmd2fp((char *)&f_imz2ch1[0],sizeof(f_imz2ch1)))
        {
lcdwrite(192,CMD);
            lcdprint("Finger Detected");
            _delay_ms(1000);
          //  lcdwrite(1,CMD);
          //  lcdprint("Tamplate 1");
          //  __delay_ms(1000);
            lcdwrite(1,CMD);
            lcdprint("Place Finger");
            lcdwrite(192,CMD);
            lcdprint("    Again   "); 
            _delay_ms(2000);
            if(!sendcmd2fp((char *)&f_detect[0],sizeof(f_detect)))
            {
                if(!sendcmd2fp((char *)&f_imz2ch2[0],sizeof(f_imz2ch2)))
                {
                    lcdwrite(1,CMD);
                    lcdprint("Finger Detected");
                    _delay_ms(1000);
                    if(!sendcmd2fp((char *)&f_createModel[0],sizeof(f_createModel)))
                    {
                        id=getId();
                        f_storeModel[11]= (id>>8) & 0xff;
                        f_storeModel[12]= id & 0xff;
                        f_storeModel[14]= 14+id; 
                       if(!sendcmd2fp((char *)&f_storeModel[0],sizeof(f_storeModel)))
                       {
    buzzer(200);
                            lcdwrite(1,CMD);
                            lcdprint("Finger Stored");
                            (void)sprintf((char *)buf1,"Id:%d",(int)id);
                            lcdwrite(192,CMD);
                            lcdprint((char *)buf1);
                            _delay_ms(1000);
                       }
                       
                       else
                       {
                            lcdwrite(1,CMD);
                            lcdprint("Finger Not Stored");
buzzer(3000);
                       }
                    }
                    else
                        lcdprint("Error");
                }
                else
                   lcdprint("Error");  
}   
else  
i=2;  
        } 
break;
     }
     if(i==2)
     {
lcdwrite(0xc0,CMD);
         lcdprint("No Finger"); 
     }
}
     _delay_ms(2000);
}
 
void deleteFinger()
{
    id=getId();
   f_delete[10]=id>>8 & 0xff;
   f_delete[11]=id & 0xff;
   f_delete[14]=(21+id)>>8 & 0xff;
   f_delete[15]=(21+id) & 0xff;
   if(!sendcmd2fp(&f_delete[0],sizeof(f_delete)))
  {
     lcdwrite(1,CMD);
     sprintf((char *)buf1,"Finger ID %d ",id);
     lcdprint((char *)buf1);
     lcdwrite(192, CMD);
     lcdprint("Deleted Success");
     
  }
   else
   {
       lcdwrite(1,CMD);
       lcdprint("Error");
   }
   _delay_ms(2000);
}
      
void lcdinst()
{
    lcdwrite(0x80, CMD);
    lcdprint("1-Enroll Finger");
    lcdwrite(0xc0, CMD);
    lcdprint("2-delete Finger");
    _delay_ms(10);
}
 
void buzzer(uint t)
{
BUZHigh;
for(int i=0;i<t;i++)
_delay_ms(1);
BUZLow;
}
 
/*function to show attendence data on serial moinitor using softserial pin PD7*/
void ShowAttendance()
{
char buf[128];
lcdwrite(1,CMD);
lcdprint("Downloding....");
SerialSoftPrintln("Attendance Record");
SerialSoftPrintln(" ");
SerialSoftPrintln("S.No        ID1            ID2            Id3            ID4            ID5    ");
//serialprintln("Attendance Record");
//serialprintln(" ");
//serialprintln("S.No            ID1                     ID2                     Id3                     ID4                     ID5");
for(int cIndex=1;cIndex<=8;cIndex++)
{
sprintf((char *)buf,"%d    "
"%d:%d:%d  %d/%d/20%d    "  
"%d:%d:%d  %d/%d/20%d    "  
"%d:%d:%d  %d/%d/20%d    "  
"%d:%d:%d  %d/%d/20%d    "  
"%d:%d:%d  %d/%d/20%d    ",
cIndex,
eeprom_read((cIndex*6)),eeprom_read((cIndex*6)+1),eeprom_read((cIndex*6)+2),eeprom_read((cIndex*6)+3),
eeprom_read((cIndex*6)+4),eeprom_read((cIndex*6)+5),
eeprom_read((cIndex*6)+48),eeprom_read((cIndex*6)+1+48),eeprom_read((cIndex*6)+2+48),
eeprom_read((cIndex*6)+3+48),eeprom_read((cIndex*6)+4+48),eeprom_read((cIndex*6)+5+48),
eeprom_read((cIndex*6)+96),eeprom_read((cIndex*6)+1+96),eeprom_read((cIndex*6)+2+96),
eeprom_read((cIndex*6)+3+96),eeprom_read((cIndex*6)+4+96),eeprom_read((cIndex*6)+5+96),
eeprom_read((cIndex*6)+144),eeprom_read((cIndex*6)+1+144),eeprom_read((cIndex*6)+2+144),
eeprom_read((cIndex*6)+3+144),eeprom_read((cIndex*6)+4+144),eeprom_read((cIndex*6)+5+144),
eeprom_read((cIndex*6)+192),eeprom_read((cIndex*6)+1+192),eeprom_read((cIndex*6)+2+192),
eeprom_read((cIndex*6)+3+192),eeprom_read((cIndex*6)+4+192),eeprom_read((cIndex*6)+5+192));
 
SerialSoftPrintln(buf);
//serialprintln(buf);
}
lcdwrite(192,CMD);
lcdprint("Done");
_delay_ms(2000);
}
 
void DeleteRecord()
{
lcdwrite(1,CMD);
lcdprint("Please Wait...");
for(int i=0;i<255;i++)
eeprom_write(i,10);
_delay_ms(2000);
lcdwrite(1,CMD);
lcdprint("Record Deleted");
lcdwrite(192,CMD);
lcdprint("Successfully");
_delay_ms(2000); 
} 
 
int main()
{             
  LEDdir= 0xFF;
  LEDPort=0x03;
  KeyPORTdir=0xF0;
  KeyPORT=0x0F;  
  LCDPORTDIR=0xFF;
  DDRD+=1<<7;
  PORTD+=1<<7;
  serialbegin();
  SerialSoftPrint("Circuit Digest");
  //serialprint("Saddam Khan");
  buzzer(2000);
  
  lcdbegin();
  lcdprint("Attendance Systm");
  lcdwrite(192,CMD);
  lcdprint("Using AVR and FP");
  _delay_ms(2000);
  
  if(down == LOW)
  ShowAttendance();
  
  else if(delet == LOW)
DeleteRecord();    
 
  ind=0;    
  while(sendcmd2fp((char *)&passPack[0],sizeof(passPack)))
  {
     lcdwrite(1,CMD);
     lcdprint("FP Not Found");
     _delay_ms(2000);
     ind=0;
  }
  lcdwrite(1,CMD);
  lcdprint("FP Found");
  _delay_ms(1000);
  lcdinst();
  _delay_ms(2000);
  lcdwrite(1,CMD);
  //RTCTimeSet();
  while(1)
  { 
    RTC();
 
  //  if(match == LOW)
   // { 
matchFinger();
   // }
    
    if(enrol == LOW)
    {
buzzer(200);
        enrolFinger(); 
        _delay_ms(2000);
       // lcdinst();
    }
    
    else if(delet == LOW)
    {
buzzer(200);
        getId();
        deleteFinger();
        _delay_ms(1000);
    }
  } 
  return 0;
}
