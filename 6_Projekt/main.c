#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include "HD44780.c"

/*
 * Interface:
 * S1 - left
 * S2 - enter
 * S3 - right
 * S4 - escape
 */


typedef enum {STOP, INCREMENT, DECREMENT} mode;
typedef enum {LEFT, RIGHT, ENTER, ESCAPE, NONE} button;
typedef enum {UPPER, LOWER} line;
mode ss_mode = DECREMENT;
int counter = 0; // time counter
uint8_t nums[4] = {9,9,9,9}; // currently displayed number for 7-segment
uint8_t bvs[10]; // number definitions

// text for LCD display
char upper_line[16] = "hue hue";
char lower_line[16] = "wat wat";

inline
void ss_increment() {
    uint8_t iter;
    for(iter = 3; iter >= 0; --iter) {
        if (nums[iter] < 9) {
            ++nums[iter];
            break;
        } else {
            nums[iter] = 0;
        }
    }
}

inline
void ss_decrement() {
    uint8_t iter;
    for(iter = 3; iter >= 0; --iter) {
        if (nums[iter] > 0) {
            --nums[iter];
            break;
        } else {
            nums[iter] = 9;
        }
    }
}

ISR(TIMER0_COMP_vect) {
    if (counter > 1000) {
        counter = 0;
        switch (ss_mode) {
            case STOP:
                break;
            case INCREMENT:
                ss_increment();
                break;
            case DECREMENT:
                ss_decrement();
                break;
        }
    } else {
        ++counter;
    }
}

inline
void init()
{

    DDRA = 0xFF; // LCD
    LCD_Initalize();

    DDRB = 0xFF; // 7-segment: choose segment
    DDRD = 0xFF; // 7-segment: choose display, and diodes (4:4)

    DDRC = 0x00; // buttons
    PORTC = 0xFF; // pull-up resistors

    // timer
    OCR0 = 250;
    TCCR0 |= _BV(WGM01); // choose CTC mode
    TCCR0 |= _BV(CS01) | _BV(CS00); // 64 prescaler
    TIMSK |= _BV(OCIE0);
    sei();

    // Number definitions for 7-segment display
    bvs[0] = 0b00111111;
    bvs[1] = 0b00000110;
    bvs[2] = 0b01011011;
    bvs[3] = 0b01001111;
    bvs[4] = 0b01100110;
    bvs[5] = 0b01101101;
    bvs[6] = 0b01111101;
    bvs[7] = 0b00000111;
    bvs[8] = 0b01111111;
    bvs[9] = 0b01101111;
}

inline uint8_t bin_shift (uint8_t in, uint8_t shift)
{ return (in << shift | in >> (8 - shift)); }

inline void select_display(uint8_t num)
{ PORTD = bin_shift(~_BV(PD0), num); }

inline void segment_on(uint8_t num)
{ PORTB &= bin_shift(~_BV(PB0), num); }

inline void segment_off(uint8_t num)
{ PORTB |= bin_shift(_BV(PB0), num); }

inline void segments_all_off() { PORTB = 0xFF; }
inline void segments_all_on() { PORTB = 0x00; }

inline void segment_exclusive_on(uint8_t num)
{
    segments_all_off();
    segment_on(num);
}

inline void display_num(uint8_t num)
{
    PORTB = ~bvs[num];
}

static inline
void refresh_7seg() {
    uint8_t iter;
    for (iter = 0; iter < 4; ++iter) {
        select_display(iter);
        display_num(nums[iter]);
        _delay_ms(1);
    }
}

inline
void refresh_LCD() {
    LCD_GoToXY(0, 0);
    LCD_Text("                ");
    LCD_GoToXY(0, 0);
    LCD_Text(upper_line);
    LCD_GoToXY(0, 1);
    LCD_Text("                ");
    LCD_GoToXY(0, 1);
    LCD_Text(lower_line);
    LCD_Home();
}

inline
button get_button() {
    if ( ~PINC & (1 << PC0) )
        return LEFT;
    if ( ~PINC & (1 << PC1) )
        return ENTER;
    if ( ~PINC & (1 << PC2) )
        return RIGHT;
    if ( ~PINC & (1 << PC3) )
        return ESCAPE;
    return NONE;
}

int main(void)
{
    init();
    ss_mode = STOP;

    while(1) {
        button button_pressed = get_button();
        switch (button_pressed) {
            case LEFT:
                strcpy(upper_line, "left");
                break;
            case RIGHT:
                strcpy(upper_line, "right");
                break;
            case ENTER:
                strcpy(upper_line, "enter");
                break;
            case ESCAPE:
                strcpy(upper_line, "escape");
                break;
        }
        refresh_7seg();
        refresh_LCD();
    }
}
