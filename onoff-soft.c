#include <avr/io.h>
#include <util/delay.h>

inline void init_gpio()
{
    DDRB = 0xFF;
}

int main()
{
    init_gpio();
    while (1) {
        PORTB ^= _BV(PB1);
        for (volatile uint8_t x = 150; x > 0; x -= 2) {
            PORTB ^= _BV(PB1);
            for (volatile uint8_t y = 0; y < x; ++y);
            PORTB ^= _BV(PB1);
            _delay_ms(3);
        }

        for (volatile uint8_t x = 0; x < 150; x += 2) {
            PORTB ^= _BV(PB1);
            for (volatile uint8_t y = 0; y < x; ++y);
            PORTB ^= _BV(PB1);
            _delay_ms(3);
        }
    }
}
