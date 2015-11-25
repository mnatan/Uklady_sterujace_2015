#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

ISR(INT0_vect)
{
    PORTB++;
}

void initialize()
{
    DDRD &= ~(_BV(PD2));                // PD2 na wejście
    PORTD |= _BV(PD2);                  // pull-up resistor na PD2

    DDRB = 0xFF;                        // Diody na wyjście
    PORTB = 0;                          // 0 na diody

    DDRC = 0xFF;                        // Wyjście na C
    PORTC = 0;                          // zacznij od 0

    // Przerwanie
    GICR |= _BV(INT0);                  // Przerwania na INT0
    MCUCR |= _BV(ISC01) | _BV(ISC00);   // Zbocze wznoszące na przerwaniu INT0
    sei();                              // Włączenie przerwania
}

int main(void)
{
    initialize();

    while (1)
    {
        PORTC++;
        _delay_ms(500);
    }
}

