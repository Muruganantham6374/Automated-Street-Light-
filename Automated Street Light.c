#include <xc.h>
#include <stdint.h>
#include <stdlib.h>

#pragma config FOSC = EXTRC_CLKOUT
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config MCLRE = ON
#pragma config CP = OFF
#pragma config CPD = OFF
#pragma config BOREN = OFF
#pragma config IESO = ON
#pragma config FCMEN = ON
#pragma config LVP = OFF

#pragma config BOR4V = BOR40V
#pragma config WRT = OFF

#define _XTAL_FREQ 4000000

volatile uint8_t t1 = 0, t2 = 0, t3 = 0;
volatile unsigned char sec = 0;
volatile unsigned char min = 1;
volatile unsigned char hour = 2;
volatile unsigned char old_sec = 0xFF;

int h1, h2, h3, h4;
int m1, m2, m3, m4;
int minutes = 0x00;            //on-1
int hours = 0x00;

int h11, h12, h13, h14;
int m11, m12, m13, m14;      //off-1
int minutes1 = 0x00;
int hours1 = 0x00;
int d1 = 1;
int d=1;


int h21, h22, h23, h24;
int m21, m22, m23, m24;     //on-2
int minutes2 = 0x00;
int hours2 = 0x00;

int h31, h32, h33, h34;
int m31, m32, m33, m34;    //off-2
int minutes3 = 0x00;
int hours3 = 0x00;


volatile unsigned int press_ms = 0;
volatile unsigned char reset_request = 0;

static const uint8_t on[3] = {0x3F, 0x37, 0x40};
static const int off[3] = {0x3F, 0x71, 0x71};
static const int end[4] = {0x79, 0x54, 0x5E, 0x0A};
static const uint8_t sel[4] = {0x6D, 0x79, 0x38, 0x0B};
static const int add_on_1[4] = {4, 5, 6, 7};
static const int add_on_2[4] = {12, 13, 14, 15};
static const int add_off_1[4] = {8, 9, 10, 11};
static const int add_off_2[4] = {16, 17, 18, 19};
static const int pos[4] = {14, 13, 11, 7};

static const uint8_t seg[12] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x00, 0x40};


void GPIO_INIT(void);
void I2C_INIT(void);
void TIMER_INIT(void);

void PRINT_7SEGMENT(int, int);
void SEGMENT(int, int, int, int);
void SET_SEGMENT(int, int, int, int);

static inline void wait(void);
void I2C_Send(unsigned char);
static inline unsigned char I2C_Read(unsigned char);

void SET_TIME(void);
void READ_VALUE(void);
void EEpROM(void);
void MENU_FUNCTION(void);
void Change_real_time(void);

void GPIO_INIT() {
    PORTA = PORTB = PORTC = PORTD = PORTE = 0x00;

    TRISA = 0x0F;
    TRISB = 0x00;
    TRISC = 0x18;
    TRISD = 0x00;
    TRISE = 0x00;

    ANSEL = 0x00;
    ANSELH = 0x00;
}

void I2C_INIT() {
    SSPSTAT = 0x80;
    SSPCON = 0x28;
    SSPCON2 = 0x00;
}

void TIMER_INIT() {
    OPTION_REG = 0x81;
    TMR0 = 6;
    T0IF = 0;
    T0IE = 1;
    PEIE = 1;
    GIE = 1;
}

void PRINT_7SEGMENT(int m1, int h1)
{
    uint8_t d1 = (h1 / 10) & 0x0F;
    uint8_t d2 = (h1 % 10) & 0x0F;
    uint8_t d3 = (m1 / 10) & 0x0F;
    uint8_t d4 = (m1 % 10) & 0x0F;

    RD1 = RD2 = RD3 = 0;
    RD0 = 1;
    PORTB = seg[d1];
    __delay_ms(2);
    RD0 = RD2 = RD3 = 0;
    RD1 = 1;
    PORTB = seg[d2];
    __delay_ms(2);
    RD1 = RD0 = RD3 = 0;
    RD2 = 1;
    PORTB = seg[d3];
    __delay_ms(2);
    RD1 = RD2 = RD0 = 0;
    RD3 = 1;
    PORTB = seg[d4];
    __delay_ms(2);
}

void SEGMENT(int a, int b, int c, int d)
{
    RD1 = RD2 = RD3 = 0;
    RD0 = 1;
    PORTB = a;
    __delay_ms(2);
    RD0 = RD2 = RD3 = 0;
    RD1 = 1;
    PORTB = b;
    __delay_ms(2);
    RD1 = RD0 = RD3 = 0;
    RD2 = 1;
    PORTB = c;
    __delay_ms(2);
    RD1 = RD2 = RD0 = 0;
    RD3 = 1;
    PORTB = seg[d];
    __delay_ms(2);
}
void DIS_SEGMENT(int a, int b, int c, int d)
{
    RD1 = RD2 = RD3 = 0;
    RD0 = 1;
    PORTB = a;
    __delay_ms(2);
    RD0 = RD2 = RD3 = 0;
    RD1 = 1;
    PORTB = b;
    __delay_ms(2);
    RD1 = RD0 = RD3 = 0;
    RD2 = 1;
    PORTB = c;
    __delay_ms(2);
    RD1 = RD2 = RD0 = 0;
    RD3 = 1;
    PORTB = d;
    __delay_ms(2);
}

void SET_SEGMENT(int a, int b, int c, int d)
{
    RD1 = RD2 = RD3 = 0;
    RD0 = 1;
    PORTB = seg[a];
    __delay_ms(2);
    RD0 = RD2 = RD3 = 0;
    RD1 = 1;
    PORTB = seg[b];
    __delay_ms(2);
    RD1 = RD0 = RD3 = 0;
    RD2 = 1;
    PORTB = seg[c];
    __delay_ms(2);
    RD1 = RD2 = RD0 = 0;
    RD3 = 1;
    PORTB = seg[d];
    __delay_ms(2);
}

static inline void wait(void)
{
    while (!SSPIF);
    SSPIF = 0;
}

void I2C_Send(unsigned char send)
{
    ACKSTAT = 1;
    SSPBUF = send;
    while (ACKSTAT);
    wait();
}

static inline unsigned char I2C_Read(unsigned char last)
{
    unsigned char read;

    SSPCON2bits.RCEN = 1;
    while (!SSPIF);
    SSPIF = 0;

    read = SSPBUF;

    ACKDT = last ? 1 : 0;
    SSPCON2bits.ACKEN = 1;

    while (!SSPIF);
    SSPIF = 0;

    return read;
}

void SET_TIME() {
    SSPCON2bits.SEN = 1;
    wait();

    I2C_Send(0xD0);
    I2C_Send(0x00);
    I2C_Send(sec);
    I2C_Send(min);
    I2C_Send(hour);

    SSPCON2bits.PEN = 1;
    wait();
}

void READ_VALUE() {
    SSPCON2bits.SEN = 1;
    wait();

    I2C_Send(0xD0);
    I2C_Send(0x00);

    SSPCON2bits.RSEN = 1;
    wait();

    I2C_Send(0xD1);

    t1 = I2C_Read(0);
    t2 = I2C_Read(0);
    t3 = I2C_Read(1);

    PRINT_7SEGMENT(t2, t3);

    if (t1 != old_sec) {
        RC0 ^= 1;
        old_sec = t1;
    }

    SSPCON2bits.PEN = 1;
    wait();
}
void EEpROM(void)
{

    SET_SEGMENT(h3, h4, m3, m4);
    if (RA1)
    {
        __delay_ms(10);
        minutes++;
    }
    if (RA2)
    {
        __delay_ms(10);
        minutes--;
    }
    if (RA0)
    {
        d = 0;
    }
    if (minutes == 60)
    {
        minutes = 0;
        hours++;
    }
    if (hours >= 24)
    {
        hours = 0;
    }
    if (hours <=-1)
    {
        hours = 23;
    }

    if (minutes <= -1)
    {
        minutes = 59;
        hours--;
    }
    m3 = minutes / 10;
    m4 = minutes % 10;
    h3 = hours   / 10;
    h4 = hours   % 10;
}
void EEpROM_2(void) {

    SET_SEGMENT(h23, h24, m23, m24);
    if (RA1)
    {
        __delay_ms(100);
        minutes2++;
    }
    if (RA2)
    {
        __delay_ms(100);
        minutes2--;
    }
    if (RA0)
    {
        d = 0;
    }
    if (minutes2 == 60) {
        minutes2 = 0;
        hours2++;
    }
    if (hours2 >= 24) {
        hours2 = 0;
    }
    if (hours2 <=-1) {
        hours2 = 23;
    }

    if (minutes2 <= -1) {
        minutes2 = 59;
        hours2--;
    }
    m23 = minutes2 / 10;
    m24 = minutes2 % 10;
    h23 = hours2  / 10;
    h24 = hours2   % 10;
}
void EEpROM_3(void) {

    SET_SEGMENT(h33, h34, m33, m34);
    if (RA1)
    {
        __delay_ms(100);
        minutes3++;
    }
    if (RA2)
    {
        __delay_ms(100);
        minutes3--;
    }
    if (RA0)
    {
        d1 = 0;
    }
    if (minutes3 == 60) {
        minutes3 = 0;
        hours3++;
    }
    if (hours3 >= 24) {
        hours3 = 0;
    }
    if (hours3 <=-1) {
        hours3 = 23;
    }

    if (minutes3 <= -1) {
        minutes3 = 59;
        hours3--;
    }
    m33 = minutes3 / 10;
    m34 = minutes3 % 10;
    h33 = hours3   / 10;
    h34 = hours3   % 10;
}
void EEpROM_1(void) {

    SET_SEGMENT(h13, h14, m13, m14);
    if (RA1)
    {
         __delay_ms(100);
        minutes1++;

    }
    if (RA2)
    {
         __delay_ms(100);
        minutes1--;

    }
    if (RA0)
    {
        d1 = 0;
    }
    if (minutes1 == 60)
    {
        minutes1 = 0;
        hours1++;
    }
    if (hours1 >= 24)
    {
        hours1 = 0;
    }
    if (hours1 <=-1)
    {
        hours1 = 23;
    }

    if (minutes1 <= -1)
    {
        minutes1 = 59;
        hours1--;
    }
    m13 = minutes1 / 10;
    m14 = minutes1 % 10;
    h13 = hours1   / 10;
    h14 = hours1   % 10;
}

void MENU_FUNCTION() {
    uint8_t a = 100, b = 1, c = 100;
    uint8_t change_value = 0, on_value = 1, off_value = 1;

    while (a--) {
        SEGMENT(sel[0], sel[1], sel[2], sel[3]);
    }

    while (b) {
        if (change_value == 0)
        {
            SEGMENT(on[0], on[1], on[2], on_value);

            if (RA0) {
                on_value++;
                change_value = 1;
                while (RA0);
                d = 1;
            }

            if (RA1 || RA2)
            {
                while (d)
                {
                    switch (on_value)
                    {
                        case 1:
                        {
                            m1 = eeprom_read(0);
                            m2 = eeprom_read(1);
                            m3 = m1 - 48;
                            m4 = m2 - 48;

                            h1 = eeprom_read(2);
                            h2 = eeprom_read(3);
                            h3 = h1 - 48;
                            h4 = h2 - 48;

                            minutes = (m3 * 10) + m4;
                            hours   = (h3 * 10) + h4;
                            SET_SEGMENT(h3, h4, m3, m4);
                            EEpROM();
                            break;
                        }

                        case 2:
                        {
                            m21 = eeprom_read(8);
                            m22 = eeprom_read(9);
                            m23 = m21 - 48;
                            m24 = m22 - 48;

                            h21 = eeprom_read(10);
                            h22 = eeprom_read(11);
                            h23 = h21 - 48;
                            h24 = h22 - 48;

                            minutes2 = (m23 * 10) + m24;
                            hours2   = (h23 * 10) + h24;
                            SET_SEGMENT(h23, h24, m23, m24);
                            EEpROM_2();
                            break;
                        }

                    }
                    eeprom_write(8, m23+48);
                    eeprom_write(9, m24+48);
                    eeprom_write(10,h23+48);      //WRITE ON_2
                    eeprom_write(11,h24+48);

                    eeprom_write(0,m3+48);
                    eeprom_write(1,m4+48);
                    eeprom_write(2,h3+48);       //WRITE ON_1
                    eeprom_write(3,h4+48);
                }
            }
        }
        else if (change_value == 1)
        {
            SEGMENT(off[0], off[1], off[2], off_value);
            if (RA0)
            {
                off_value++;
                change_value = 0;
                while (RA0);
                d1=1;
            }
            if (RA1 || RA2)
            {
                while (d1)
                {
                   switch (off_value)
                      {
                          case 1:
                          {
                            m11 = eeprom_read(4);
                            m12 = eeprom_read(5);
                            m13 = m11 - 48;
                            m14 = m12 - 48;

                            h11 = eeprom_read(6);
                            h12 = eeprom_read(7);
                            h13 = h11 - 48;
                            h14 = h12 - 48;

                            minutes1 = (m13 * 10) + m14;
                            hours1   = (h13 * 10) + h14;
                            SET_SEGMENT(h13, h14, m13, m14);
                            EEpROM_1();
                            break;
                        }

                        case 2:
                        {
                            m31 = eeprom_read(12);
                            m32 = eeprom_read(13);
                            m33 = m31 - 48;
                            m34 = m32 - 48;

                            h31 = eeprom_read(14);
                            h32 = eeprom_read(15);
                            h33 = h31 - 48;
                            h34 = h32 - 48;

                            minutes3 = (m33 * 10) + m34;
                            hours3   = (h33 * 10) + h34;
                            SET_SEGMENT(h33, h34, m33, m34);
                            EEpROM_3();
                            break;
                        }

                    }
                    eeprom_write(12,m33+48);
                    eeprom_write(13,m34+48);
                    eeprom_write(14,h33+48);      //WRITE OFF_2
                    eeprom_write(15,h34+48);

                    eeprom_write(4,m13+48);
                    eeprom_write(5,m14+48);
                    eeprom_write(6,h13+48);       //WRITE OFF_1
                    eeprom_write(7,h14+48);
                }

              }

          }


        if (off_value == 3)
        {
            change_value = 2;
            while (c--) SEGMENT(end[0], end[1], end[2], end[3]);
            a = b = c = 0;
        }
    }
}

void Change_real_time()
{
    PRINT_7SEGMENT(0, 0);
}
void auto_on()
{

    if( t2==minutes && t3==hours || t2==minutes2 && t3==hours2 )
    {
            int t=150;
            RC1=1;

    }
    if( t2==minutes1 && t3==hours1 || t2==minutes3 && t3==hours3 )
    {

            int t=150;
            RC1=0;

    }

}
void set_reader()
{
    m1 = eeprom_read(0);
    m2 = eeprom_read(1);
    m3 = m1 - 48;
    m4 = m2 - 48;
    h1 = eeprom_read(2);
    h2 = eeprom_read(3);
    h3 = h1 - 48;
    h4 = h2 - 48;
    minutes = (m3 * 10) + m4;
    hours   = (h3 * 10) + h4;

    m11 = eeprom_read(4);
    m12 = eeprom_read(5);
    m13 = m11 - 48;
    m14 = m12 - 48;
    h11 = eeprom_read(6);
    h12 = eeprom_read(7);
    h13 = h11 - 48;
    h14 = h12 - 48;
    minutes1 = (m13 * 10) + m14;
    hours1   = (h13 * 10) + h14;

    m21 = eeprom_read(8);
    m22 = eeprom_read(9);
    m23 = m21 - 48;
    m24 = m22 - 48;
    h21 = eeprom_read(10);
    h22 = eeprom_read(11);
    h23 = h21 - 48;
    h24 = h22 - 48;
    minutes2 = (m23 * 10) + m24;
    hours2   = (h23 * 10) + h24;

    m31 = eeprom_read(12);
    m32 = eeprom_read(13);
    m33 = m31 - 48;
    m34 = m32 - 48;
    h31 = eeprom_read(14);
    h32 = eeprom_read(15);
    h33 = h31 - 48;
    h34 = h32 - 48;
    minutes3 = (m33 * 10) + m34;
    hours3   = (h33 * 10) + h34;

}

void __interrupt() isr(void) {
    if (T0IF) {
        TMR0 = 6;
        T0IF = 0;

        if (RA0 && RA1 && !RA2)
        {
            if (++press_ms >= 1000)
            {
                reset_request = 1;             //RESET
                press_ms = 0;
            }
        }
          else if (RA0 && !RA1 && !RA2)
        {
            if (++press_ms >= 1000)
            {
                MENU_FUNCTION();            //SELECTE AND SET TIME
                press_ms = 0;
            }
        } else if (RA1 && !RA0 && !RA2)
          {
            if (++press_ms >= 1000)
             {
                RC1 = 1;
                press_ms = 0;               //MANAUAL ON
             }
            }
           else if (RA2 && !RA0 && !RA1)
           {
            if (++press_ms >= 1000)
            {
                RC1 = 0;                   //MANUAL OFF
                press_ms = 0;
            }
           }

           else if (RA1 && RA2 && !RA0)
           {
            if (++press_ms >= 1000)
              {
                Change_real_time();       //ZERO REAL TIME CLOCK
                press_ms = 0;
              }
           }
           else
           {
            auto_on();
            press_ms = 0;                //FLOATING

           }
    }
}

void main(void) {
    GPIO_INIT();
    I2C_INIT();
    TIMER_INIT();
    SET_TIME();
    set_reader();
    while (1)
    {
        READ_VALUE();
        if (reset_request)
        {

        SWDTEN = 1;
        GIE = 0;

            while (1);
            {

            }
        }
    }
}
