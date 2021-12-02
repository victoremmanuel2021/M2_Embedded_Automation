CIRCUIT DIAGRAM:


![image](https://user-images.githubusercontent.com/92036056/144447042-4ad6f2c2-c523-4702-9b3d-7f363c64d3f8.png)


It has Atmega32 microcontroller for controlling all the process of the project. The push or membrane button is used for enrolling, deleting, selecting IDs for attendance, a buzzer is used for indication and a 16x2 LCD to instruct user on how to use the machine. push or membrane buttons are directly connected to pin PA2 (ENROL key 1), PA3(DEL key 2), PA0(UP key 3), PA1(DOWN key 4) of microcontroller with respect to the ground or PA4. And a LED is connected at pin PC2 of microcontroller with respect to ground through a 1k resistor. Fingerprint module’s Rx and Tx directly connected at Serial pin PD1 and PD3 of microcontroller. 5v supply is used for powering the whole circuit by using LM7805 voltage regulator which is powered by 12v dc adaptor. A buzzer is also connected at pin PC3. A 16x2 LCD is configured in 4-bit mode and its RS, RW, EN, D4, D5, D6, and D7 are directly connected at pin PB0, PB1, PB2, PB4, PB5, PB6, PB7 of microcontroller. RTC module is connected at I2Cpin PC0 SCL and PC1 SDA. And PD7 is used as soft UART Tx pin for getting the current time.
