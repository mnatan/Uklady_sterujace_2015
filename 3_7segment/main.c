#include <avr/io.h>
#include <util/delay.h>

inline
void init()
{
    DDRB = 0xFF; // choose segment
    DDRD = 0xFF; // choose display

    // timer
    OCR0 = 250;
    TCCR0 |= _BV(WGM01); // choose CTC mode
    TCCR0 |= _BV(CS01) | _BV(CS00); // 64 prescaler
    TIMSK |= _BV(OCIE0);
    sei();
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
    bvs[3] = 0b00101111;
    bvs[4] = 0b01100110;
    bvs[5] = 0b00101101;
    bvs[6] = 0b00111101;
    bvs[7] = 0b00000111;
    bvs[8] = 0b01111111;

    PORTD = ~_BV(PD0); // select first display
    select_display(0);
    uint8_t i;
    while (1) {
        for (i = 0; i < 9; ++i) {
            display_num(i);
            _delay_ms(400);
        }
    }

    /*while (1) {*/
    /*uint8_t iter;*/
    /*segments_all_on();*/
    /*_delay_ms(200);*/
    /*for (iter = 0; iter < 8; ++iter) {*/
    /*segment_off(iter);*/
    /*_delay_ms(200);*/
    /*}*/
    /*}*/
}

