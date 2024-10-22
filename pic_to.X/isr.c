 #include <xc.h>
 #include "isr.h"

extern unsigned char key_detected;
extern unsigned char c1, c2, c3, c4,ch;
void __interrupt() isr(void)
{
	if (INT0F == 1)
	{
        c1 = 0, c2 = 0, c3 = 0, c4 = 0;
        key_detected = !key_detected;        
        INT0F = 0;
	}
//    if (RCIF == 1)
//    {
//        ch = RCREG;
//        RCIF = 0;
//    }
}