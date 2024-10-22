 #include <xc.h>
 #include "isr.h"

//extern unsigned char key_detected;
extern unsigned char ch;
void __interrupt() isr(void)
{
    if (RCIF == 1)
    {
        ch = RCREG;
        RCIF = 0;
    }
}