#include "pio.h"
#include "button.h"

#define BUT_PIO PIO_DEFINE (PORT_D, 7)

/** Return non-zero if button pressed.  */
int button_pressed_p (void)
{
    return (pio_input_get(BUT_PIO));
}


/** Initialise button1.  */
void button_init (void)
{
    pio_config_set(BUT_PIO, PIO_INPUT);
}

