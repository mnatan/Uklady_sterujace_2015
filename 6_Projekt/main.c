#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>
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
typedef enum {MENU, GAME} state;
typedef enum {MAIN, SETTINGS} state_of_menu; // lol
typedef void (*void_ptr)();

mode ss_mode = INCREMENT;
int counter = 0; // time counter
uint8_t nums[4] = {9,9,9,9}; // currently displayed number for 7-segment
uint8_t bvs[10]; // number definitions

state game_state;
state_of_menu menu_state;
line menu_position;
uint8_t time_setting;
uint8_t button_delay;

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
        if (!(counter % 400))
            button_delay = 0;
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
    select_display(5); // dafuq
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
    /*LCD_Home();*/
    LCD_GoToXY(0, menu_position);
}

inline
button get_button() {
    if (button_delay == 1)
        return NONE;

    if ( ~PINC & (1 << PC0) ) {
        button_delay = 1;
        return LEFT;
    }
    if ( ~PINC & (1 << PC1) ) {
        button_delay = 1;
        return ENTER;
    }
    if ( ~PINC & (1 << PC2) ) {
        button_delay = 1;
        return RIGHT;
    }
    if ( ~PINC & (1 << PC3) ) {
        button_delay = 1;
        return ESCAPE;
    }
    return NONE;
}


void display_game() {}

void game_left() {

}
void game_right() {

}
void game_enter() {

}
void game_escape() {

}

void set_countdown() {
    ss_mode = ++ss_mode % 3;
}

void set_time() {
    time_setting = (time_setting + 10) % 60;
}

void display_menu() {
    switch (menu_state) {
        case MAIN:
            strcpy(upper_line, " Start game");
            strcpy(lower_line, " Settings");
            break;
        case SETTINGS:
            /*strcpy(upper_line, " Game mode ");*/
            switch (ss_mode) {
                case STOP:
                    strcpy(upper_line, " Game mode: STOP");
                    break;
                case INCREMENT:
                    strcpy(upper_line, " Game mode: INC");
                    break;
                case DECREMENT:
                    strcpy(upper_line, " Game mode: DEC");
                    break;
            }
            sprintf(lower_line, " Time: %d", time_setting);
            break;
    }
}
void menu_left() { menu_position = UPPER; }
void menu_right() { menu_position = LOWER; }

void menu_enter() {
    if (menu_state == MAIN) {
        if (menu_position == UPPER) {
            game_state = GAME;
        } else if (menu_position == LOWER) {
            menu_state = SETTINGS;
        }
    } else if (menu_state = SETTINGS) {
        if (menu_position == UPPER) {
            set_countdown();
        } else if (menu_position == LOWER) {
            set_time();
        }
    }
}
void menu_escape() {

}

int main(void)
{
    init();

    game_state = MENU;
    void_ptr left_actions[2]   = { &menu_left,   &game_left };
    void_ptr right_actions[2]  = { &menu_right,  &game_right };
    void_ptr enter_actions[2]  = { &menu_enter,  &game_enter };
    void_ptr escape_actions[2] = { &menu_escape, &game_escape };

    menu_state = MAIN;
    menu_position = 0;

    time_setting = 30;

    while(1) {
        switch (game_state) {
            case MENU:
                display_menu();
                break;
            case GAME:
                display_game();
                break;
        }

        button button_pressed = get_button();
        switch (button_pressed) {
            case LEFT:
                (*left_actions[game_state])();
                break;
            case RIGHT:
                (*right_actions[game_state])();
                break;
            case ENTER:
                (*enter_actions[game_state])();
                break;
            case ESCAPE:
                (*escape_actions[game_state])();
                break;
        }
        refresh_7seg();
        refresh_LCD();
    }
}
