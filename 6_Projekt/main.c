#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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
typedef enum {MARKED, UNMARKED} c_mode;

mode ss_mode = STOP;
mode timer_mode = DECREMENT;
int counter = 0; // time counter
uint8_t nums[4] = {9,9,9,9}; // currently displayed number for 7-segment
uint8_t bvs[10]; // number definitions

state game_state;
state_of_menu menu_state;
line menu_position;
uint8_t time_setting;
button last_button = NONE;

char* words[] = {
    "computer", "unit", "test", "boiler", "heaven", "markdown"
};
uint8_t selected_word = 0;
char word_buffer[17];

c_mode cursor_mode = UNMARKED;
uint8_t cursor_pos = 0;
uint8_t marked_pos = 0;

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

inline uint8_t time_up() {
    // Don't judge me
    return (nums[0] ==0 && nums[1] == 0 && nums[2] == 0 && nums[3] == 0);
}

inline
void ss_decrement() {
    if (time_up())
        return;

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

inline void init()
{

    DDRA = 0xFF; // LCD
    LCD_Initalize();

    DDRB = 0xFF; // 7-segment: choose segment
    DDRD = 0xFF; // 7-segment: choose display, and diodes (4:4)
    PORTD = 0x00;

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

inline void set_nums(uint8_t num)
{
    nums[0] = 0;
    nums[1] = num / 100;
    nums[2] = num / 10;
    nums[3] = num % 10;
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
    if (game_state == MENU)
        LCD_GoToXY(0, menu_position);
    else
        LCD_GoToXY(20, 20);
}

inline
button get_button() {
    button current = NONE;
    
    if ( ~PINC & (1 << PC0) ) {
        current =  LEFT;
    }
    if ( ~PINC & (1 << PC1) ) {
        current =  ENTER;
    }
    if ( ~PINC & (1 << PC2) ) {
        current =  RIGHT;
    }
    if ( ~PINC & (1 << PC3) ) {
        current =  ESCAPE;
    }
    if (current != last_button){
        last_button = current;
        return current;
    } else return NONE;
}

void game_over() {
    set_nums(0);
    ss_mode = STOP;
    game_state = MENU;
}

void game_win() {
    strcpy(upper_line, "You win");
    strcpy(lower_line, "   ");
    refresh_LCD();
    _delay_ms(5000);

    game_over();
}

void show_cursor() {
    if (cursor_mode == MARKED) {
        strcpy(lower_line, "X");
    } else {
        strcpy(lower_line, "            ");
    }
}

void display_game() {
    if (time_up()) {
        game_over();
    }

    if (strcmp(words[selected_word], word_buffer) == 0) {
        game_win();
    }

    strcpy(upper_line, word_buffer);
    show_cursor();
    LCD_GoToXY(cursor_pos, 0);
}

void game_left() {
    if (cursor_pos > 0) {
        --cursor_pos;
    }
    if (cursor_mode == MARKED) {
        char temp = word_buffer[cursor_pos];
        word_buffer[cursor_pos] = word_buffer[cursor_pos+1];
        word_buffer[cursor_pos+1] = temp;
    }
}
void game_right() {
    if (cursor_pos < strlen(word_buffer) - 1)
        ++cursor_pos;
    if (cursor_mode == MARKED) {
        char temp = word_buffer[cursor_pos];
        word_buffer[cursor_pos] = word_buffer[cursor_pos-1];
        word_buffer[cursor_pos-1] = temp;
    }

}
void game_enter() {
    if ( cursor_mode == UNMARKED) {
        marked_pos = cursor_pos;
        cursor_mode = MARKED;
    } else if ( cursor_mode == MARKED) {
        cursor_mode = UNMARKED;
    }
}
void game_escape() {
    game_over();
}

uint8_t randint(uint8_t upto) {
    return (rand() % upto) + 1;
}

uint8_t random_word() {
    uint8_t no_of_words = sizeof(words)/sizeof(char*);
    return randint(no_of_words) - 1;
}

void permute_string(char* string) {
    char original[16];
    strcpy(original, string);
    while (!strcmp(original, string)) {
        uint8_t length = strlen(string) - 1;
        for (uint8_t rep = 0; rep < 20; ++ rep) {
            uint8_t index1 = randint(length);
            uint8_t index2 = randint(length);

            char temp = string[index1];
            string[index1] = string[index2];
            string[index2] = temp;
        }
    }
}

void game_setup() {
    set_nums(time_setting);
    ss_mode = timer_mode;

    selected_word = random_word();
    strcpy(word_buffer, words[selected_word]);

    permute_string(word_buffer);
    strcpy(upper_line, word_buffer);
    strcpy(lower_line, "           ");

    cursor_mode = UNMARKED;
    cursor_pos = 0;
}

void set_countdown() {
    timer_mode = ++timer_mode % 3;
}

void set_time() {
    time_setting = (time_setting % 60) + 10;
}

void display_menu() {
    switch (menu_state) {
        case MAIN:
            strcpy(upper_line, " Start game");
            strcpy(lower_line, " Settings");
            break;
        case SETTINGS:
            /*strcpy(upper_line, " Game mode ");*/
            switch (timer_mode) {
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
            game_setup();
        } else if (menu_position == LOWER) {
            menu_state = SETTINGS;
        }
    } else if (menu_state == SETTINGS) {
        if (menu_position == UPPER) {
            set_countdown();
        } else if (menu_position == LOWER) {
            set_time();
        }
    }
}

void menu_escape() {
    menu_state = MAIN;
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

    time_setting = 60;

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
                srand(counter);
                (*left_actions[game_state])();
                break;
            case RIGHT:
                srand(counter);
                (*right_actions[game_state])();
                break;
            case ENTER:
                srand(counter);
                (*enter_actions[game_state])();
                break;
            case ESCAPE:
                srand(counter);
                (*escape_actions[game_state])();
                break;
        }
        refresh_7seg();
        refresh_LCD();
    }
}
