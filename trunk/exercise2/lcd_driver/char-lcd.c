/* For lcdGoto */
#define LEFT  0x0
#define RIGHT 0x4
#define CURSOR_DISP_SHIFT 0x10

#define max(a,b) ((((int)a)>((int)b)) ? (a) : (b))

volatile int g_is8bit = 1;

/*
 * Reads both busy bit and address counter
 */
void read8(int *busy, int *address) {
	
	lcd_rs_clear();
	lcd_rw_set();
	lcd_data8_in();

	/* Clock setup time */
	busywait(1);

	lcd_en_set();

	/* Data output delay time */
	busywait(2);

	int bits = lcd_data8_read();
	*busy = (bits & DB7) != 0;
	*address = ((bits & ALL & ~DB7) >> 16);

	/* Clock pulse width */
	busywait(1);
	
	lcd_en_clear();
	
	/* Data hold time */
	busywait(1);
	
	/* Cycle time */
	busywait(1);
}

void read4(int *busy, int *address) 
{
	
	lcd_rs_clear();
	lcd_rw_set();
	lcd_data4_in();

	/* Clock setup time */
	busywait(1);

	lcd_en_set();

	/* Data output delay time */
	busywait(2);

	int bits = lcd_data4_read();
	*busy = (bits & DB7) != 0;
	*address = ((bits & ALL4 & ~DB7) >> 16);

	/* Clock pulse width */
	busywait(1);
	
	lcd_en_clear();

	/* Data hold time */
	busywait(1);

	lcd_en_set();

	/* Data output delay time */
	busywait(2);

	bits = lcd_data4_read();
	*address = ((bits & ALL4) >> 20);

	/* Clock pulse width */
	busywait(1);
	
	lcd_en_clear();
	
	/* Data hold time */
	busywait(1);

	/* Cycle time */
	busywait(1);	
}

void pollBusyBit()
{
	int busy, addr;

	while (1) {
		if(g_is8bit) {
			read8(&busy, &addr);
		}
		else {
			read4(&busy, &addr);
		}

		if (busy == 0)
			break;
	}
}

void generic_set8(int bits) 
{
	lcd_data8_out();
	
	/* Clock setup time */
	busywait(1);

	lcd_data8_set(bits);
	lcd_en_set();

	/* Clock pulse width */
	busywait(1);
	
	lcd_en_clear();
	
	/* Data hold time */
	busywait(1);
	
	/* Cycle time */
	busywait(1);

	pollBusyBit();
}

void generic_set4(int bits)
{
	lcd_data4_out();

	/* Clock setup time */
	busywait(1);

	lcd_data4_set(bits);
	lcd_en_set();

	/* Clock pulse width */
	busywait(1);

	lcd_en_clear();

    /* Data Hold time */
	busywait(1);

	lcd_data4_set(bits << 4);
	lcd_en_set();
	
	/* Clock pulse width */
	busywait(1);

	lcd_en_clear();

    /* Data Hold time */
	busywait(1);

	/* Cycle time */
	busywait(1);

	pollBusyBit();
}

void inst(int bits) {
	
	lcd_rs_clear();
	lcd_rw_clear();
	if(g_is8bit) {
		generic_set8(bits);
	}
	else {
		generic_set4(bits);
	}
}

void write(int bits) {
	
	lcd_rs_set();
	lcd_rw_clear();
	if(g_is8bit) {
		generic_set8(bits);
	}
	else {
		generic_set4(bits);
	}
}

void lcdPrintChar(char c) {
	write(c);
}

void lcdGoto(int dest_addr) {
	int busy;
	int addr;
	int delta;
	int direction = RIGHT;

	if (g_is8bit) {
		read8(&busy, &addr);
	}
	else {
		read4(&busy, &addr);
	}

	delta = dest_addr - addr;

	if (delta < 0)
	{
		delta = -delta;
		direction = LEFT;
	}

	for(int i = 0; i < delta; i++) 
	{
		inst(CURSOR_DISP_SHIFT | direction);
	}
}

void lcdPrintString(char *str) 
{
	int tok = strlen(str);
	for (int i = 0; i < strlen(str); ++i)
	{
		char c = str[i];
		if (c == '\n') {
			inst(0xC0);
			tok = i;
		}
		else {
			lcdPrintChar(c);
		}
	}

	if ((tok > 16) || (strlen(str) - tok > 16))
	{
		busywait(10000);
		for (int j = 0; j < max(strlen(str) - tok - 16, tok - 16); j++)
		{
			inst(0x18);
			busywait(1000);
		}
        
	}
}

void set_bitmode(int mode) 
{
	if (mode == 4) {
		inst(0x28);
	}
	else if (mode == 8){
		inst(0x38);
	}
	else {
		return;
	}

	g_is8bit = (mode == 8);
}


void lcdInit() 
{
	lcd_en_rs_out();
	lcd_data8_out();
	lcd_rw_out(); 
	lcd_backlight_on(); 
	
	/* Function Set 1 line  */
	inst(0x38);

	/* Display on, Blink */
	inst(0x0F);

	/* Clear Screen*/ 
	inst(0x01);

	/* Increment */
	inst(0x06);

	return;
}


