#include "HD44780.h"
#include <util/delay.h>

inline
void init_gpio() {
    DDRA = 0xFF;
}

void main () {
    init_gpio();
    LCD_Initalize();
    while (1) {
        _delay_ms(1000);
        LCD_GoToXY(0, 0);
        LCD_Text("wat");
        LCD_GoToXY(4, 1);
        LCD_Text("hue");
    }
}
