#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

uint8_t nums[4] = {0,0,0,0};
ISR(TIMER0_COMP_vect) {
}

inline
void init()
{
    DDRA = 0xFF; // klawiatura
    DDRB = 0xFF; // choose segment
    DDRD = 0xFF; // choose display

    // timer
    OCR0 = 250;
    TCCR0 |= _BV(WGM01); // choose CTC mode
    TCCR0 |= _BV(CS01) | _BV(CS00); // 64 prescaler
    TIMSK |= _BV(OCIE0);
    sei();
}

inline uint8_t matrix_index(uint8_t num){
    num &= 0x0F;
    switch(num){
        case 0:
        case 1:
        case 2:
            return num;
        case 4:
            return 3;
        case 8:
            return 4;
    }
}

void check_button(){
    DDRA = 0xF0;
    PORTA = 0x0F;
    _delay_us(20);
    uint8_t row = matrix_index((~PINA) >> 4);

    DDRA = 0x0F;
    PORTA = 0xF0;
    _delay_us(20);
    uint8_t column = matrix_index(~PINA);
    
    uint8_t button = row*column;

    nums[2] = (button/10) % 10;
    nums[3] = (button) % 10;
    /*for (iter = 0; iter < 4; ++iter) {*/
        
    /*}*/
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

uint8_t bvs[10];
inline void display_num(uint8_t num)
{
    PORTB = ~bvs[num];
}

int main(void)
{
    init();

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

    while(1) {
        uint8_t iter;
        for (iter = 0; iter < 4; ++iter) {
            select_display(iter);
            display_num(nums[iter]);
            _delay_ms(1);
        }
    }
}
