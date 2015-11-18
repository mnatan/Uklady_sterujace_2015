#include <avr/io.h>
#include <util/delay.h>

#define DELAY 100

inline void init_gpio()
{
    DDRB = 0xFF;
    DDRD = 0x00;
    PORTD = 0xFF;
}

inline void snake()
{
    PORTB = _BV(PB0);
    _delay_ms(DELAY);

    uint8_t it;
    while (1) {
        for (it = 0; it < 7; ++it) {
            PORTB <<= 1;
            _delay_ms(DELAY);
        }
        for (it = 0; it < 7; ++it) {
            PORTB >>= 1;
            _delay_ms(DELAY);
        }
    }
}

inline void wait_for_button()
{
    while (PIND & (1 << PD2)) {}
}

int main()
{
    init_gpio();

    wait_for_button();
    snake();
}
