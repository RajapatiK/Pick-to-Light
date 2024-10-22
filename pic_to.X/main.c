
#include <xc.h>
#include "main.h"
#include "ssd_display.h"
#include "isr.h"
#include "digital_keypad.h"
#include "eeprom.h"
#include "can.h"


static unsigned char ssd[MAX_SSD_CNT];
static unsigned char digit[] = {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, SMLN, UDSR, SMLI, SMLD, SMLU, UDSR, SMLS, SMLT};
unsigned char key_detected = 0;
static unsigned char key = 0xFF, flag = 0,flag_2 = 0;
unsigned char c1 = 0, c2 = 0, c3 = 0, c4 = 0,n_id[4],u_st[4];
extern unsigned char can_payload[13];
static unsigned int count, time1 = 0, time2 = 0, time3 = 0;


void init_config(void) 
{
    INTEDG0 = 1;
    INT0F = 0;
    INT0E = 1;
    PEIE = 1;
    GIE = 1;
    init_ssd_control();
    init_digital_keypad();
    init_can();
}
void delay(unsigned short factor)
{
	unsigned short i, j;
	for (i = 0; i < factor; i++)
	{
		for (j = 500; j--; );
	}
}

void main(void) 
{
    init_config();
    static int i = 10,j=0;
    static char selected_field = 1;

    while (1) 
    {
        if (key_detected) 
        {
            if (flag == 0) 
            {
                ssd[0] = digit[10];
                ssd[1] = digit[11];
                ssd[2] = digit[12];
                ssd[3] = digit[13];
                flag = 1;
            }
            else if (flag == 1) 
            {
                key = read_digital_keypad(STATE_CHANGE);
                if (key == SWITCH3) 
                {
                    if (i >= 18) i = 10;
                    ssd[0] = digit[i++];
                    ssd[1] = digit[i++];
                    ssd[2] = digit[i++];
                    ssd[3] = digit[i++];
                }
            }
            if (key == SWITCH2) {
                flag = 2;
            }
            if (flag == 2) 
            {
                key = read_digital_keypad(STATE_CHANGE);

                if (key == SWITCH2) 
                {
                    selected_field++;
                    if (selected_field > 4) 
                    {
                        selected_field = 1;
                    }
                }
                if (key == SWITCH1) 
                {
                    if (selected_field == 1) {
                        c1++;
                        if (c1 >= 10) c1 = 0;
                    }
                    else if (selected_field == 2) {
                        c2++;
                        if (c2 >= 10) c2 = 0;
                    }
                    else if (selected_field == 3) {
                        c3++;
                        if (c3 >= 10) c3 = 0;
                    }
                    else if (selected_field == 4) {
                        c4++;
                        if (c4 >= 10) c4 = 0;
                    }
                }
                ssd[0] = digit[c4];
                ssd[1] = digit[c3];
                ssd[2] = digit[c2];
                ssd[3] = digit[c1];
                
                if (selected_field == 1) {
                    ssd[3] |= 0x10;
                }
                else if (selected_field == 2) {
                    ssd[2] |= 0x10;
                }
                else if (selected_field == 3) {
                    ssd[1] |= 0x10;
                }
                else if (selected_field == 4) 
                {
                    ssd[0] |= 0x10;
                }
                if(key == SWITCH3)
                {
                    if(i<=14)
                    {
                        write_internal_eeprom(0,c4);
                        write_internal_eeprom(1,c3);
                        write_internal_eeprom(2,c2);
                        write_internal_eeprom(3,c1);
                        flag = 0;
                        key_detected = 0;                                               
                    }
                    else
                    {
                        write_internal_eeprom(4,c4);
                        write_internal_eeprom(5,c3);
                        write_internal_eeprom(6,c2);
                        write_internal_eeprom(7,c1);
                        TXB0D0 = read_internal_eeprom(0);
                        TXB0D1 = read_internal_eeprom(1);
                        TXB0D2 = read_internal_eeprom(2);
                        TXB0D3 = read_internal_eeprom(3);
                        TXB0D4 = read_internal_eeprom(4);
                        TXB0D5 = read_internal_eeprom(5);
                        TXB0D6 = read_internal_eeprom(6);
                        TXB0D7 = read_internal_eeprom(7);
                        can_transmit(); 
                        delay(1000);
                        flag = 0;
                        key_detected = 0;
                    }                        
                }
            }
            
            display(ssd);
        }
        else if (can_receive()) 
        {
            flag_2 = 1;
            for (unsigned char k = 0; k < 4; k++) 
            {
                n_id[k] = read_internal_eeprom(k);
                if ((n_id[k] + 48) != can_payload[D0 + k]) 
                {
                    flag_2 = 0;                    
                    break;
                } 
                if(flag_2 == 1)
                    count = (((can_payload[D4]-48)*1000) + ((can_payload[D5]-48)*100) + ((can_payload[D6] - 48)*10) + (can_payload[D7]-48));   
            }
        }
        else if (flag_2 == 1) 
        {
            key = read_digital_keypad(LEVEL);
            if (key == SWITCH1)
            {
                time1++;
                if (time1 >= 50) 
                {
                    time1 = 0;
                    count++;
                    if (count == 10000)
                        count = 0;
                }
            } 
            else if (time1 > 0 && time1 < 50) 
            {
                time1 = 0;
                count++;
                if (count == 10000)
                    count = 0;
            }
            if (key == SWITCH2) 
            {
                time2++;
                if (time2 >= 50) 
                {
                    time2 = 0;
                    count--;
                    if (count > 10000)
                        count = 9999;
                }
            } 
            else if (time2 > 0 && time2 < 50) 
            {
                time2 = 0;
                count--;
                if (count > 10000)
                    count = 9999;
            }
            if (key == SWITCH3) 
            {
                time3++;
                if (time3 > 100)
                    time3 = 0;
            } 
            else if (time3 > 0 && time3 < 100) 
            {
                time3 = 0;
                write_internal_eeprom(4,(count / 1000));
                write_internal_eeprom(5,(count / 100) % 10);
                write_internal_eeprom(6,(count / 10) % 10);
                write_internal_eeprom(7,(count % 10));
                TXB0D0 = n_id[0];
                TXB0D1 = n_id[1];
                TXB0D2 = n_id[2];
                TXB0D3 = n_id[3];
                TXB0D4 = (count / 1000);
                TXB0D5 = ((count / 100) % 10);
                TXB0D6 = ((count / 10) % 10);
                TXB0D7 = (count % 10);
                can_transmit();
                delay(1000);
                flag_2 = 0;                
            }
            ssd[0] = digit[(count / 1000)];
            ssd[1] = digit[((count / 100) % 10)];
            ssd[2] = digit[((count / 10) % 10)];
            ssd[3] = digit[(count % 10)];
            display(ssd);                        
        }
    }
}