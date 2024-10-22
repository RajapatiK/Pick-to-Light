#include <xc.h>
#include "main.h"
//#include "ssd_display.h"
#include "isr.h"
//#include "digital_keypad.h"
//#include "eeprom.h"
#include "can.h"
#include "uart.h"
#include "clcd.h"


unsigned char flag = 0, flag_2 = 1, ch;
unsigned char j, a = 0, b = 0;
unsigned char n_id[4] = {0, 0, 0, 0}, u_st[4] = {0, 0, 0, 0};
extern unsigned char can_payload[13];

void init_config(void) 
{
    INTEDG0 = 1;
    INT0F = 0;
    INT0E = 1;
    PEIE = 1;
    GIE = 1;
    init_can();
    init_uart();
    init_clcd();
}

void delay(unsigned short factor) 
{
    unsigned short i, j;
    for (i = 0; i < factor; i++) {
        for (j = 500; j--;);
    }
}

void main(void) 
{
    init_config();
    unsigned char i = 0;
    puts("Enter the N_ID: ");
    while (1) 
    {       
        if (can_receive()) 
        {
            puts("N_ID: ");
            for (j = 0; j < 8; j++) {
                if (j == 4)
                    puts("\n\rU_ST: ");
                putch(can_payload[D0 + j] + '0');
            }
            puts("\n\r");
            puts("Enter the N_ID: ");
            flag_2 = 1;
        }
//        if(flag_2 == 0)
//        {
//            puts("Enter the N_ID: ");
//            flag_2 = 1;
//        }
        if (ch != '\0') 
        {
            putch(ch);
            if (flag_2 == 1) 
            {
                if (ch != '\r')
                {
//                    putch(ch);
                    if (a <= 3)
                        n_id[a++] = ch;
//                    ch = '\0';
                    
                }
                else if (ch == '\r') 
                {
                    if (a < 4) 
                    {
                        for (int i = 3; i >= 4 - a; i--) 
                        {
                            n_id[i] = n_id[i - (4 - a)];
                        }
                        for (int i = 0; i < 4 - a; i++) 
                        {
                            n_id[i] = '0';
                        }
                    } 
                    else 
                    {
                        flag_2 = 2;
                        puts("\n\rEnter the U_ST: ");
                        a = 0;
                    }
                }
            }
            if (flag_2 == 2) 
            {
                if (ch != '\r') 
                {
//                    putch(ch);
                    if (b <= 3)
                        u_st[b++] = ch;
//                    ch = '\0';
                } 
                else if (ch == '\r') 
                {
                    if (b < 4) 
                    {
                        for (int i = 3; i >= 4 - b; i--) 
                        {
                            u_st[i] = n_id[i - (4 - b)];
                        }
                        for (int i = 0; i < 4 - b; i++) 
                        {
                            u_st[i] = '0';
                        }
                    } 
                    else 
                    {
                        b = 0;
                        flag_2 = 3;
                    }
                }
            }
            if (flag_2 == 3) 
            {
                can_transmit();
                delay(1000);
                flag_2 = 1;
                puts("\n\rEnter the N_ID: ");
            }
        }
        ch = 0;
    }
}
