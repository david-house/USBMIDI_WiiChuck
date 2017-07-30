
#ifndef I2CHelper
#define I2CHelper
    
#define I2C_TIMEOUT_MS 500
    
#include <stdio.h>
#include "project.h"
    
    
enum I2C_Transfer_Type
{
    I2C_TRANSFER_TYPE_UNKNOWN,
    I2C_TRANSFER_TYPE_WRITE,
    I2C_TRANSFER_TYPE_WRITE_READ,
    I2C_TRANSFER_TYPE_READ
};
    
//this holds the results of an I2C Transfer
typedef struct {
    uint16_t counter_value;
    uint16_t i2c_write_xfer_result;
    uint16_t i2c_read_xfer_result;
    uint16_t write_cmd_result;
    uint16_t read_cmd_result;
    uint8_t bytes_to_write;
    uint8_t bytes_written;
    uint8_t bytes_to_read;
    uint8_t bytes_read;
    uint8_t timeout_error;
    uint8_t ok;
    enum I2C_Transfer_Type xfer_type;
} I2C_Transfer_Status;

// reference the systick counter variable
extern volatile uint32_t i2c_ms_counter;
    
I2C_Transfer_Status I2C_status;

// forward declarations
void I2C_clear_xfer_status(I2C_Transfer_Status *xfer_status);
void I2C_check_transfer_status(I2C_Transfer_Status *xfer_status);
void I2C_write(uint8_t slave_address, uint8_t *bytes, uint8_t byte_count, I2C_Transfer_Status *xfer_status);
void I2C_xfer(uint8_t slave_address, uint8_t *tx_bytes, uint8_t tx_count, uint8_t *rx_bytes, uint8_t rx_count, I2C_Transfer_Status *xfer_status, uint8_t i2c_write_mode, uint8_t i2c_read_mode);
void I2C_print_transfer_status(I2C_Transfer_Status *xfer_status);
void I2C_read(uint8_t slave_address, uint8_t *bytes, uint8_t byte_count, I2C_Transfer_Status *xfer_status);
#endif