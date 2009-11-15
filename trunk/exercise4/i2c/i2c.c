
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
	if (command_len > 0)
	{
	// Start I2 master transact.
	I20CONSET = BIT5;
	print("START\n");

	// Wait for SI to raise
	while (!(I20CONSET & BIT3)) { }
	print("SI --> 1\n");

	// Check correct status
	if (I20STAT != 0x08) {
		print("Error in status register: STAT = ");
		printHex(I20STAT, 4);
		print("\n");
		return -1;
	}
	
	// Input slave address and R!W
	I20DAT = slave_address | 0;
	print("Writing slave address for write op");
	printHex(slave_address, 4);
	print("\n");

	// Clear SI
	I20CONCLR = BIT3;
	print("SI --> 0\n");

	// Wait for SI to raise
	while (!(I20CONSET & BIT3)) { }
	print("SI --> 1\n");

	// no ack received
	if (I20STAT != 0x18)
	{
		print("Error in status register: STAT = ");
		printHex(I20STAT, 4);
		print("\n");
		return -1;
	}

	// Write the command byte
	for (int i = 0; i < command_len; i++) {
		
		// Setup next command byte to data register.
		I20DAT = command[i];
		print("setup data with next command byte:");
		printHex(command[i],4);
		print("\n");

		// Clear SI
		I20CONCLR = BIT3;
		print("SI --> 0");

		// Wait for SI to raise
 	 	while (!(I20CONSET & BIT3)) { }
		print("SI --> 1");

		if (I20STAT != 0x18)
		{
			print("Error in status register: STAT = ");
			printHex(I20STAT, 4);
			print("\n");
			return -1;
		}
	}
	} // command_len > 0

	if (response_len > 0)
	{
	// Start I2 master transact.
	I20CONSET = BIT5;
	print("START\n");

	// Wait for SI to raise
	while (!(I20CONSET & BIT3)) { }
	print("SI --> 1\n");


	// Check correct status
	if (I20STAT != 0x08) {
		print("Error in status register: STAT = ");
		printHex(I20STAT, 4);
		print("\n");
		return -1;
	}
	
	// Input slave address and R!W
	I20DAT = slave_address | 1;
	print("Writing slave address for read op");
	printHex(slave_address | 1, 4);
	print("\n");


	// Clear SI
	I20CONCLR = BIT3;
	print("SI --> 0\n");

	// Wait for SI to raise
	while (!(I20CONSET & BIT3)) { }
	print("SI --> 1\n");

	// no ack received
	if (I20STAT != 0x40)
	{
		print("Error in status register: STAT = ");
		printHex(I20STAT, 4);
		print("\n");
		return -1;
	}

	// Read the response 
	for (int i = 0; i < response_len; i++) {
		
		if (i < response_len - 1)
		{
			// Set the AA bit
			I20CONSET = BIT2;			
			print("AA --> 1\n");
		}
		else
		{
			// Clear the AA bit
			I20CONCLR = BIT2;
			print("AA --> 0\n");
		}

		// Setup next command byte to data register.
		response[i] = I20DAT;
		print("read next response byte:");
		printHex(response[i],4);
		print("\n");

		// Clear SI
		I20CONCLR = BIT3;
		print("SI --> 0\n");

		// Wait for SI to raise
 	 	while (!(I20CONSET & BIT3)) { }
		print("SI --> 1\n");

		if (I20STAT != 0x40)
		{
			print("Error in status register: STAT = ");
			printHex(I20STAT, 4);
			print("\n");
			return -1;
		}		

	}


	// Set STOP bit
	I20CONSET = BIT4;
	print("STOP\n");

	// Wait for SI to raise
	while (!(I20CONSET & BIT3)) { }
	print("SI --> 1\n");



	} // response_len > 0



}



