
/*
Set the bit frequency to I2C_BIT_RATE
*/
void i2cInit() 
{

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


}



