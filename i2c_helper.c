#include "project.h"
#include "i2c_helper.h"

// clears out the transfer status log
void I2C_clear_xfer_status(I2C_Transfer_Status *xfer_status)
{
    xfer_status->bytes_read = 0;
    xfer_status->bytes_to_read = 0;
    xfer_status->bytes_to_write = 0;
    xfer_status->bytes_written = 0;
    xfer_status->counter_value = 0;
    xfer_status->i2c_read_xfer_result = 0;
    xfer_status->i2c_write_xfer_result = 0;
    xfer_status->ok = 0;
    xfer_status->read_cmd_result = 0;
    xfer_status->timeout_error = 0;
    xfer_status->write_cmd_result = 0;
    xfer_status->xfer_type = I2C_TRANSFER_TYPE_UNKNOWN;
}

// this will check several statuses of the I2C transfer and set the ok byte = 1 if OK
void I2C_check_transfer_status(I2C_Transfer_Status *xfer_status)
{
    xfer_status->ok = 1; // assume OK
    
    if ( (xfer_status->xfer_type == I2C_TRANSFER_TYPE_WRITE) | (xfer_status->xfer_type == I2C_TRANSFER_TYPE_WRITE_READ) )
    {
        // bytes to write didn't equal bytes written throw an error
        if (xfer_status->bytes_to_write != xfer_status->bytes_written)
            xfer_status->ok = 0;

        // bytes to read didn't equal bytes read throw an error
        if (xfer_status->bytes_to_read != xfer_status->bytes_read)
            xfer_status->ok = 0;       
        
        // if timeout happened mark as error
        if (xfer_status->timeout_error == 1)
            xfer_status->ok = 0;
        
        // if the write command result was anything but no error set to error
        if (xfer_status->write_cmd_result != I2C_MSTR_NO_ERROR)
            xfer_status->ok = 0;
    }
    
    if ( xfer_status->xfer_type == I2C_TRANSFER_TYPE_WRITE )
    {
        // if the xfer result was anything but write completed set to error
        // this is only for a write-only transaction
        if ((xfer_status->i2c_write_xfer_result & (~I2C_MSTAT_WR_CMPLT)) > 0)
            xfer_status->ok = 0;
    }
    
    if ( xfer_status->xfer_type == I2C_TRANSFER_TYPE_WRITE_READ )
    {
        // if the xfer result was anything but write completed set to error
        // this is only for a write-read transaction
        
        uint16_t expected_flags = I2C_MSTAT_WR_CMPLT | I2C_MSTAT_XFER_INP | I2C_MSTAT_XFER_HALT;
        if ((xfer_status->i2c_write_xfer_result & (~expected_flags)) > 0)
            xfer_status->ok = 0;
        
        // if the read command result was anything but no error set to error
        if (xfer_status->read_cmd_result != I2C_MSTR_NO_ERROR)
            xfer_status->ok = 0;
        
    }
}

void I2C_write(uint8_t slave_address, uint8_t *bytes, uint8_t byte_count, I2C_Transfer_Status *xfer_status)
{
    
    uint32_t final_i2c_ms_counter;
    uint8_t i2c_timeout = 0;
    
    // clear the I2C bus status
    I2C_MasterClearStatus();
    I2C_MasterClearReadBuf();
    I2C_MasterClearWriteBuf();
    
    // clear out the I2C status record
    I2C_clear_xfer_status(xfer_status);

    // set the transfer type
    xfer_status->xfer_type = I2C_TRANSFER_TYPE_WRITE;
    
    // log the number of bytes to write
    xfer_status->bytes_to_write = byte_count;
    
    // reset the systick timer to the timeout value
    i2c_ms_counter = 0;
    
    // send the write command and read teh result
    xfer_status->write_cmd_result = I2C_MasterWriteBuf(slave_address, bytes, byte_count, I2C_MODE_COMPLETE_XFER);
    
    // while the read is not complete and the timeout is not set, wait
    while ((!(I2C_MasterStatus() & I2C_MSTAT_WR_CMPLT)) && (i2c_ms_counter < I2C_TIMEOUT_MS)) {continue;}

    // get the terminal value of the i2c_ms_counter
    final_i2c_ms_counter = i2c_ms_counter;

    // if the counter has passed zero it is a timeout so set the flag
    if (final_i2c_ms_counter >= I2C_TIMEOUT_MS)
    {
        i2c_timeout = 1;
    }
        
    // get the I2C timeout counter value
    xfer_status->counter_value = (uint16_t)i2c_ms_counter;

    // get the transfer status result
    xfer_status->i2c_write_xfer_result = I2C_MasterStatus();

    // get the timeout flag
    xfer_status->timeout_error = i2c_timeout;

    // get the # of bytes actually sent
    xfer_status->bytes_written = I2C_MasterGetWriteBufSize();
    
    // clear the transfer status  
    I2C_MasterClearStatus();
    
    // parse the transfer status
    I2C_check_transfer_status(xfer_status);

}


void I2C_xfer(uint8_t slave_address, uint8_t *tx_bytes, uint8_t tx_count, uint8_t *rx_bytes, uint8_t rx_count, I2C_Transfer_Status *xfer_status, uint8_t i2c_write_mode, uint8_t i2c_read_mode)
{
    uint8_t i2c_timeout = 0;
    uint32_t final_i2c_ms_counter;
    
    // clear the I2C bus status
    I2C_MasterClearStatus();
    I2C_MasterClearReadBuf();
    I2C_MasterClearWriteBuf();
    
    // clear out the status record
    I2C_clear_xfer_status(xfer_status);
    
    // set the transfer type
    xfer_status->xfer_type = I2C_TRANSFER_TYPE_WRITE_READ;

    // log the number of bytes to write
    xfer_status->bytes_to_write = tx_count;
    
    // log the number of bytes to read
    xfer_status->bytes_to_read = rx_count;
    
    // reset the systick timer to the timeout value
    i2c_ms_counter = 0;
    
    
    // send the write command and read the result
    xfer_status->write_cmd_result = I2C_MasterWriteBuf(slave_address, tx_bytes, tx_count, i2c_write_mode);
    
    // while the write is not complete and the timeout is not set, wait
    while ((!(I2C_MasterStatus() & I2C_MSTAT_WR_CMPLT)) && (i2c_ms_counter < I2C_TIMEOUT_MS)) {continue;}

    // record the number of bytes written
    xfer_status->bytes_written = I2C_MasterGetWriteBufSize();
    
    // get the transfer status result
    xfer_status->i2c_write_xfer_result = I2C_MasterStatus();
    
    // clear the master status
    I2C_MasterClearStatus();
    
    // read the results
    xfer_status->read_cmd_result = I2C_MasterReadBuf(slave_address, rx_bytes, rx_count, i2c_read_mode);
    while ((!(I2C_MasterStatus() & I2C_MSTAT_RD_CMPLT)) && (i2c_ms_counter < I2C_TIMEOUT_MS)) {continue;}   
    
    // get the terminal value of the i2c_ms_counter
    final_i2c_ms_counter = i2c_ms_counter;

    // if the counter has passed zero it is a timeout so set the flag
    if (final_i2c_ms_counter >= I2C_TIMEOUT_MS)
    {
        i2c_timeout = 1;
    }
    
    // record if the timeout occurred
    xfer_status->timeout_error = i2c_timeout;

    // get the I2C timeout counter value
    xfer_status->counter_value = final_i2c_ms_counter;
    
    // record the number of bytes read
    xfer_status->bytes_read = I2C_MasterGetReadBufSize();
    
    // record the master transfer status
    xfer_status->i2c_read_xfer_result = I2C_MasterStatus();
    
    
    // clear the master status
    I2C_MasterClearStatus();
    
    // parse the transfer status
    I2C_check_transfer_status(xfer_status);

    
}


void I2C_read(uint8_t slave_address, uint8_t *bytes, uint8_t byte_count, I2C_Transfer_Status *xfer_status)
{
    
    uint32_t final_i2c_ms_counter;
    uint8_t i2c_timeout = 0;
    
    // clear the I2C bus status
    I2C_MasterClearStatus();
    I2C_MasterClearReadBuf();
    I2C_MasterClearWriteBuf();
    
    // clear out the I2C status record
    I2C_clear_xfer_status(xfer_status);

    // set the transfer type
    xfer_status->xfer_type = I2C_TRANSFER_TYPE_READ;
    
    // log the number of bytes to write
    xfer_status->bytes_to_read = byte_count;
    
    // reset the systick timer to the timeout value
    i2c_ms_counter = 0;
    
    // send the write command and read teh result
    xfer_status->read_cmd_result = I2C_MasterReadBuf(slave_address, bytes, byte_count, I2C_MODE_COMPLETE_XFER);
    
    // while the read is not complete and the timeout is not set, wait
    while ((!(I2C_MasterStatus() & I2C_MSTAT_RD_CMPLT)) && (i2c_ms_counter < I2C_TIMEOUT_MS)) {continue;}

    // get the terminal value of the i2c_ms_counter
    final_i2c_ms_counter = i2c_ms_counter;

    // if the counter has passed zero it is a timeout so set the flag
    if (final_i2c_ms_counter >= I2C_TIMEOUT_MS)
    {
        i2c_timeout = 1;
    }
        
    // get the I2C timeout counter value
    xfer_status->counter_value = (uint16_t)i2c_ms_counter;

    // get the transfer status result
    xfer_status->i2c_read_xfer_result = I2C_MasterStatus();

    // get the timeout flag
    xfer_status->timeout_error = i2c_timeout;

    // get the # of bytes actually read
    xfer_status->bytes_read = I2C_MasterGetReadBufSize();
    
    // clear the transfer status  
    I2C_MasterClearStatus();
    
    // parse the transfer status
    I2C_check_transfer_status(xfer_status);

}

void I2C_print_transfer_status(I2C_Transfer_Status *xfer_status)
{
    char outstr[64];
    
    if (xfer_status->ok)
    {
        UART_PutString("Overall command result: OK\r\n");
    }
    else
    {
        UART_PutString("Overall command result: ***ERROR****\r\n");
    }
    
    sprintf(outstr, "Timeout counter: 0x%04X\r\n", xfer_status->counter_value);
    UART_PutString(outstr);
    
    sprintf(outstr, "I2C write cmd result: 0x%04X\r\n", xfer_status->write_cmd_result);
    UART_PutString(outstr);

    sprintf(outstr, "I2C write xfer result: 0x%04X\r\n", xfer_status->i2c_write_xfer_result);
    UART_PutString(outstr);
    
    sprintf(outstr, "Byte count 0x%02X to write, 0x%02X written\r\n", xfer_status->bytes_to_write, xfer_status->bytes_written);
    UART_PutString(outstr);
     
    sprintf(outstr, "I2C read cmd result: 0x%04X\r\n", xfer_status->read_cmd_result);
    UART_PutString(outstr);
    
    sprintf(outstr, "I2C read xfer result: 0x%04X\r\n", xfer_status->i2c_read_xfer_result);
    UART_PutString(outstr);
    
    sprintf(outstr, "Byte count 0x%02X to read, 0x%02X read\r\n", xfer_status->bytes_to_read, xfer_status->bytes_read);
    UART_PutString(outstr);

    sprintf(outstr, "Timeout flag: 0x%02X\r\n", xfer_status->timeout_error);
    UART_PutString(outstr);

    
}
