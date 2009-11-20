
// I20CON register bits
#define I2EN	BIT6 // I2C Enable/Disable
#define STA		BIT5 // START flag
#define STO		BIT4 // STOP flag
#define SI		BIT3 // Interrupt flag 
#define AA		BIT2 // Assert Acknowledge

//States:
#define STAT_BUS_ERR		0x00

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

// Global data vars
uint8_t g_slave_address = 0;

uint8_t *g_command = 0;
int32_t g_command_len = 0;

uint8_t *g_response = 0;
int32_t g_response_len = 0;

int g_trans_count = 0;
int g_recv_count = 0;

int g_rw = 0;

void __attribute__ ((interrupt("FIQ"))) fiq_isr(void) {
	
	uint8_t status = I2STAT;
	
	switch(status) {
		case STAT_START:
			//I20CONCLR = STA
			//print("Clear START\n");
	
			// Input slave address and R!W
			I20DAT = g_slave_address | g_rw;
			print("Writing slave address for write op");
			printHex(g_slave_address, 4);
			print("\n");
			
			I20CONSET = AA;
			break;
		case STAT_SLA_W_ACK:
			// Put next command byte in data register.
			I20DAT = g_command[g_trans_count];

			print("Setup data with next command byte:");
			printHex(g_command[g_trans_count], 4);
			print("\n");
			
			g_trans_count++;

			break;
		case STAT_REPSTART:
			break;
		case STAT_SLA_W_NACK:
			// Transmit stop condition
			I20CONSET = STO | AA;
			break;
		case STAT_T_DAT_ACK:
			g_trans_count++;

			if(g_trans_count == g_command_len) { 
				I20CONSET = STO | AA;
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
			I20CONSET = AA;
			break;

		case STAT_SLA_R_NACK:
			I20CONSET = STO | AA;
			break;
		
		case STAT_R_DAT_ACK:
			// Setup next command byte to data register.
			g_response[g_recv_count] = I20DAT;

			print("Read next response byte:");
			printHex(g_response[g_recv_count], 4);
			print("\n");
			
			g_trans_count++;

			if(g_recv_count != g_recv_len) {
				I20CONSET = AA;
			}
			else {
				I20CONCLR = AA;
			}

			break;
		case STAT_R_DAT_NACK:
			I20CONSET = STO | AA;
			break;

		default:
			PRINT_STAT_ERR();
	}

	// Clear the SI bit
	I20CONCLR = SI;
	//PRINT_SI(0);
}

/*
Set the bit frequency to I2_BIT_RATE
*/
void i2cInit() 
{

	PINSEL0 |= (BIT4 | BIT6);
	PINSEL0 &= ~(BIT5 | BIT7);

	// Set the I20 Enable bit, master mode.
	I20CONSET = 0x40;

	// Setup the I20 bit rate.
	I20SCLH = ((CLOCKS_PCLK/I2C_BIT_RATE)/2);
	I20SCLL = ((CLOCKS_PCLK/I2C_BIT_RATE)/2);

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

@return the number of bytes read from the slave or 
		if the transaction fails the function returns 
		a negative number that indicated the nature of failure
*/
int32_t i2cMasterTransact(uint8_t slave_address,
						  uint8_t *command,
						  int32_t command_len,
						  uint8_t *response,
						  int32_t response_len) 
{
	// Enable I2C interface 
	I20CONSET = I2EN;
	I20CONCLR = (STA | SI | AA);
	print("Enabled I2C\n");

	g_slave_address = slave_address;

	g_command = command;
	g_command_len = command_len;

	g_response = response;
	g_response_len = response_len;

	if (command_len > 0)
	{
		g_rw = 0;

		I20CONSET = STA;
		print("START\n");


	} // command_len > 0

	if (response_len > 0)
	{
		g_rw = 1;

		// Start I2 master transact.
		I20CONSET = STA;
		print("START\n");

	} // response_len > 0
}


