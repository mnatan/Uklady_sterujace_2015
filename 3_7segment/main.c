#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

int counter = 0;
uint8_t nums[4] = {0,0,0,0};
ISR(TIMER0_COMP_vect) {
    ++counter;
    if (counter > 1000) {
        counter = 0;
        uint8_t iter;
        for(iter = 3; iter >= 0; --iter) {
            if (nums[iter] < 9) {
                ++nums[iter];
                break;
            } else {
                nums[iter] = 0;
            }
        }
    } else {
        ++counter;
    }
}

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
