
#define DEBUG

// I20CON register bits
#define I2EN	BIT6 // I2C Enable/Disable
#define STA		BIT5 // START flag
#define STO		BIT4 // STOP flag
#define SI		BIT3 // Interrupt flag 
#define AA		BIT2 // Assert Acknowledge

//States:
#define STAT_BUS_ERR		0x00
#define STAT_NO_INFO		0xF8

// Master States:
//   General
#define STAT_START		    0x08
#define STAT_REPSTART 	    0x10

//   Master transmitter
#define STAT_SLA_W_ACK	    0x18
#define STAT_SLA_W_NACK	    0x20
#define STAT_T_DAT_ACK      0x28
#define STAT_T_DAT_NACK		0x30

//   Master receiver
#define STAT_SLA_R_ACK		0x40
#define STAT_SLA_R_NACK     0x48
#define STAT_R_DAT_ACK		0x50
#define	STAT_R_DAT_NACK		0x58

// Utility macros
#define WAIT_SI() 	\
	while (!(I20CONSET & SI)) { }

#define PRINT_SI(x)       \
	print("SI --> ");     \
	printNum(x);          \
	print("\n");      

#define PRINT_STAT_ERR()                        \
	print("Error in status register: STAT = "); \
	printHex(I20STAT, 4);                       \
	print("\n");								

#define CHECK_STAT(stat)             \
	if (I20STAT != (stat)) {         \
		PRINT_STAT_ERR();            \
		return -1;                   \
	}

typedef void (*completion_handler_t)(void);

// Global data vars
volatile uint8_t g_slave_address = 0;

volatile uint8_t *g_command = 0;
volatile int32_t g_command_len = 0;

volatile uint8_t *g_response = 0;
volatile int32_t g_response_len = 0;

volatile int g_trans_count = 0;
volatile int g_recv_count = 0;

volatile int g_rw = 0;

volatile int g_transact_ended = 0;

volatile completion_handler_t g_completion;

#ifdef USE_FIQ
void __attribute__ ((interrupt("FIQ"))) fiq_isr(void) 
#endif
void __attribute__ ((interrupt("IRQ"))) viq_9(void)
{
	uint8_t status = I20STAT;
        print("in irq\n");	
	switch(status) {
		case STAT_START:
			// Input slave address and R!W
			I20DAT = g_slave_address | g_rw;
#ifdef DEBUG
			print("Writing slave address + r!w\n");
			printHex(g_slave_address | g_rw, 4);
			print("\n");
#endif			
			I20CONSET = AA;
			I20CONCLR = STA;
			break;
		case STAT_SLA_W_ACK:
			// Put next command byte in data register.
			I20DAT = g_command[g_trans_count];
#ifdef DEBUG
			print("Setup data with next command byte:");
			printHex(g_command[g_trans_count], 4);
			print("\n");
#endif

		break;
		case STAT_REPSTART:
			break;
		case STAT_SLA_W_NACK:
			// Transmit stop condition
			I20CONSET = STO | AA;
			break;
		case STAT_T_DAT_ACK:
			g_trans_count++;
#ifdef DEBUG
			print("Setup data with next command byte:");
			printHex(g_command[g_trans_count], 4);
			print("\n");
			print("transcount=");
			printNum(g_trans_count);
			print("\n");
#endif

			if(g_trans_count == g_command_len) { 
				if(g_response_len > 0) {
					I20CONSET = STA | STO | AA;
					g_rw = 1;
				}
				else {
					I20CONSET = STO | AA;
					g_transact_ended = 1;
					if (g_completion != 0)
						g_completion();
				}
			}
			else {
				I20DAT = g_command[g_trans_count];
				I20CONSET = AA;
			}

			break;
		case STAT_T_DAT_NACK:
			// Transmit stop cond
			I20CONSET = STO | AA;
			break;
		
		// Master Receiver states:
		case STAT_SLA_R_ACK:
#ifdef DEBUG                    
                        print("STAT_SLA_R_ACK");
#endif

			I20CONSET = AA;
			break;

		case STAT_SLA_R_NACK:
#ifdef DEBUG			
			print("STAT_SLA_R_NACK");
#endif
			I20CONSET = STO | AA;
			break;
		
		case STAT_R_DAT_ACK:
			// Setup next command byte to data register.
			g_response[g_recv_count] = I20DAT;
#ifdef DEBUG
			print("Read next response byte:");
			printHex(g_response[g_recv_count], 4);
			print("\n");
#endif
			g_recv_count++;

			if(g_recv_count != g_response_len) {
				I20CONSET = AA;
			}
			else {
				I20CONCLR = AA;
			}

			break;
		case STAT_R_DAT_NACK:
#ifdef DEBUG                    
                        print("STAT_R_DAT_NACK\n");
#endif

			I20CONSET = STO | AA;
			g_transact_ended = 1;
			if (g_completion != 0)
				g_completion();
			break;

		case STAT_BUS_ERR:
			I20CONSET = STO;
			break;
		default:
#ifdef DEBUG
			PRINT_STAT_ERR();
#endif
	}

	// Clear the SI bit
	I20CONCLR = SI;
	vicUpdatePriority();
}

/*
Set the bit frequency to I2_BIT_RATE
*/
void i2cInit() 
{
	// Select P0.2, P0.3 pin functions to be SCL0 and SDA0 respectively
	PINSEL0 |= (BIT4 | BIT6);
	PINSEL0 &= ~(BIT5 | BIT7);

	// Setup the I20 bit rate.
	I20SCLH = ((CLOCKS_PCLK/I2C_BIT_RATE)/2);
	I20SCLL = ((CLOCKS_PCLK/I2C_BIT_RATE)/2);
	
	// Clear all flags
	I20CONCLR = 0xFF;          

	// Set the I20 Enable bit, master mode.
	I20CONSET = I2EN;

	print("Init the I2 controller\n");
}



/*
Write a command to a slave with a given slave address and read a response from it.
If the length of the response buffer is zero the function does not read from the slave at all.
This function is blocking

@param slave_address address of the slave to write command to
@param command the command to write
@param command_len length of the command
@param response the response received from the slave
@param response_len length of the response
@param onComplete completion hander for asynchronuous work

@return the number of bytes read from the slave or 
		if the transaction fails the function returns 
		a negative number that indicated the nature of failure
*/
int32_t i2cMasterTransact(uint8_t slave_address,
						  uint8_t *command,
						  int32_t command_len,
						  uint8_t *response,
						  int32_t response_len,
			completion_handler_t onComplete) 
{
	I20CONCLR = (STA | SI | AA);
	print("Enabled I2C\n");

	g_slave_address = slave_address;

	g_command = command;
	g_command_len = command_len;

	g_response = response;
	g_response_len = response_len;

	g_trans_count = 0;
	g_recv_count = 0;

	g_rw = 0;

	g_transact_ended = 0;

	if (command_len == 0) {
		g_rw = 1;
	}

	if ((g_rw == 1) && (response_len == 0))
		return 0;

	g_completion = onComplete;

	print("Start I2C0\n");
	printNum(g_rw);
	I20CONSET = STA;


}


