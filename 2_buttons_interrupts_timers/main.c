#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

ISR(INT0_vect)
{
    if ((uint8_t)(OCR2 + 10) > OCR2)
    {
        for (uint8_t i = 0; i < 10; i++)
        {
            OCR2++;
            _delay_ms(10);
        }
    }
    else
    {
        OCR2 = 255;
    }
}

ISR(INT1_vect)
{
    if ((uint8_t)(OCR2 - 10) < OCR2)
    {
        for (uint8_t i = 0; i < 10; i++)
        {
            OCR2--;
            _delay_ms(10);
        }
    }
    else
    {
        OCR2 = 0;
    }
}

void initialize()
{
    DDRD &= ~(_BV(PD2) | _BV(PD3));     // PD2 i PD3 na wejście
    PORTD |= _BV(PD2) | _BV(PD3);       // pull-up resistor na PD2 i PD3

    DDRB |= _BV(PB3);                   // Output diody na PB3 bo OC2

    DDRC = 0xFF;                        // Wyjście na C
    PORTC = 0;                          // zacznij od 0

    // Timer 2
    TCCR2 |= _BV(WGM21) | _BV(WGM20);   // Fast PWM
    TCCR2 |= _BV(COM21);                // set on CM reset on BOT
                                        // ^^^ non inverting mode
    TCCR2 |= _BV(CS20);                 // No prescaling

    OCR2 = 0;

    // Przerwania
    GICR |= _BV(INT0) | _BV(INT1);      // Przerwania na INT0 i INT1
    MCUCR |= _BV(ISC01) | _BV(ISC00);   // Zbocze wznoszące na przerwaniu INT0
    MCUCR |= _BV(ISC11) | _BV(ISC10);   // Zbocze wznoszące na przerwaniu INT1
    sei();                              // Włączenie przerwania
}

int main(void)
{
    initialize();

    while (1) {}
}
