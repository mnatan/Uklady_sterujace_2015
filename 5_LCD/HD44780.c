#include "HD44780.h"
#include <util/delay.h>

#define E  0x10
#define RS 0x20

void WriteNibble(unsigned char nibbleToWrite)
{
    PORTA |= E;
    PORTA = (PORTA & 0xF0) | (nibbleToWrite & 0x0F);
    PORTA &= ~E;
}


void WriteByte(unsigned char dataToWrite)
{
    WriteNibble(dataToWrite >> 4);
    WriteNibble(dataToWrite);
}


void LCD_Command(unsigned char command)
{
    PORTA &= ~RS; // set to instruction
    WriteByte(command);
    _delay_us(50);
}

void LCD_Text(char* text)
{
    while (*text) {
        PORTA |= RS;
        WriteByte(*text);
        _delay_us(50);
        ++text;
    }
}

void LCD_GoToXY(unsigned char x, unsigned char y)
{
    LCD_Command(0x80 | (y * 0x40 + x));
}

void LCD_Clear(void)
{
    LCD_Command(HD44780_CLEAR);
}

void LCD_Home(void)
{
    LCD_Command(HD44780_HOME);
}

void LCD_Initalize(void)
{
    _delay_ms(50);
    for(uint8_t i = 0; i < 3; ++i) {
        WriteNibble(0x03);
        _delay_ms(5);
    }
    WriteNibble(0x02);
    _delay_ms(1);

    LCD_Command(0x28);
    _delay_ms(1);

    LCD_Command(0x08);
    _delay_ms(1);

    LCD_Command(0x01);
    _delay_ms(1);

    LCD_Command(0x06);
    _delay_ms(1);

    LCD_Command(0x0F);
    _delay_ms(1);
}
