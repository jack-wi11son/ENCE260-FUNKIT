#include <avr/io.h>
#include "system.h"

int main (void)
{
    system_init ();

    /* Initialise port to drive LED 1.  */
    
    DDRC |= (1<<2);

    while (1)
    {

        /* Turn LED 1 on.  */
        if ((PIND & (1<<7)) != 0) {
            PORTC |= (1<<2);
        } else {
            PORTC &= ~(1<<2);
        }
        

    }
}
