#include <avr/io.h>
#include <util/delay.h>

inline
void init_gpio()
{
    DDRB = 0xFF; // choose segment
    DDRD = 0xFF; // choose display
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
    init_gpio();
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

    PORTD = ~_BV(PD0); // select first display
    select_display(0);
}
