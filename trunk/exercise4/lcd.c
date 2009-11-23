
#include <string.h>

#define DB7 BIT23
#define DB6 BIT22
#define DB5 BIT21
#define DB4 BIT20
#define DB3 BIT19
#define DB2 BIT18
#define DB1 BIT17
#define DB0 BIT16

#define ALL	(DB0|DB1|DB2|DB3|DB4|DB5|DB6|DB7)
#define ALL4    (DB4|DB5|DB6|DB7)

#define RW  BIT22
#define RS  BIT24
#define E	BIT25

#define lcd_backlight_on() { IODIR0 |= BIT30; \
  							 IOSET0 = BIT30; }
#define lcd_backlight_off() { IODIR0 &= ~BIT30; \
  							 IOCLR0 = BIT30; }
  							 
#define lcd_en_rs_out() ( IODIR1 |= E | RS )
  							 
#define lcd_rw_out() (IODIR0 |= RW )
  							 
#define lcd_data8_out() ( IODIR1 |= ALL )
#define lcd_data8_in()  ( IODIR1 &=  ~ALL )
  						
#define lcd_data4_out() ( IODIR1 |= ALL4 )
#define lcd_data4_in() ( IODIR1 &= ~ALL4 )
  							
#define lcd_data8_read() \
	((IOPIN1 & ALL))

#define lcd_data4_read() \
	((IOPIN1 & ALL4))

#define lcd_data8_set(c) \
	{ IOCLR1 = ALL; IOSET1 = ((uint32_t)c)<<16; }	

#define lcd_data4_set(c) \
	{ IOCLR1 = ALL; IOSET1 = ((((uint32_t)c)<<16) & ALL4); }	

  							 
#define lcd_rs_set() (IOSET1 = RS)
#define lcd_rs_clear() (IOCLR1 = RS)
  							 
#define lcd_en_set() (IOSET1 = E)
#define lcd_en_clear() (IOCLR1 = E)
  							 
#define lcd_rw_set() (IOSET0 = RW)
#define lcd_rw_clear() (IOCLR0 = RW)
  							 
void busywait(uint64_t microseconds)
{
	volatile uint64_t i = 0;
	uint64_t endLoop = microseconds * 60;
	
	while ( i < endLoop)
	{
		i++;
	}
}
  							 
#include "lcd_driver/char-lcd.c"


