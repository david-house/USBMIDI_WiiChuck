#include "project.h"
#include "i2c_helper.h"
#include "wiichuck.h"


void wiichuck_begin()
{
    // instead of the common 0x40 -> 0x00 initialization, we
    // use 0xF0 -> 0x55 followed by 0xFB -> 0x00.
    // this lets us use 3rd party nunchucks (like cheap $4 ebay ones)
    // while still letting us use official oness.
    // only side effect is that we no longer need to decode bytes in _nunchuk_decode_byte
    // see http://forum.arduino.cc/index.php?topic=45924#msg333160
    
    uint8_t return_status = 0;
    wiichuck_status_request_array[0] = 0x00;
    
    initialization_buffer[0][0] = 0xF0; //0x40
    initialization_buffer[0][1] = 0x55; //0x00
    
    initialization_buffer[1][0] = 0xFB;
    initialization_buffer[1][1] = 0x00;
    
    // following the Arduino example these are written as distinct transactions
    uint8_t j = 0;
    
    for (j = 0; j < 1; j++)
    {
        I2C_write(WIICHUCK_I2C_ADDRESS, initialization_buffer[j], sizeof(initialization_buffer[j]), &I2C_status);
        CyDelay(1); // this is copied from the Arduino site
    }
    
    wiichuck_get_motion_status(); // note the first result will be all zeros or al 0xFF

    //update();            
    for (i = 0; i<3;i++) {
        angles[i] = 0;
    }
    zeroJoyX = DEFAULT_ZERO_JOY_X;
    zeroJoyY = DEFAULT_ZERO_JOY_Y;
}

void wiichuck_get_motion_status()
{
            
    I2C_read(WIICHUCK_I2C_ADDRESS, wiichuck_status_result_array,
        sizeof(wiichuck_status_result_array),
        &I2C_status);
    
    I2C_write(WIICHUCK_I2C_ADDRESS, wiichuck_status_request_array,
        sizeof(wiichuck_status_request_array),
        &I2C_status);
    
}

void wiichuck_print_i2c_status()
{
    I2C_print_transfer_status(&I2C_status);
}

void wiichuck_get_raw_motion_status()
{
    wiichuck_get_motion_status();
    
    wiichuck_state.JX = wiichuck_status_result_array[0];
    wiichuck_state.JY = wiichuck_status_result_array[1];

    
    wiichuck_state.AX = ((uint16_t)(0b00001100 & wiichuck_status_result_array[5])) >> 2;
    wiichuck_state.AY = ((uint16_t)(0b00110000 & wiichuck_status_result_array[5])) >> 4;
    wiichuck_state.AZ = ((uint16_t)(0b11000000 & wiichuck_status_result_array[5])) >> 6;
    
    wiichuck_state.AX |= (((uint16_t)wiichuck_status_result_array[2]) << 2);
    wiichuck_state.AY |= (((uint16_t)wiichuck_status_result_array[3]) << 2);
    wiichuck_state.AZ |= (((uint16_t)wiichuck_status_result_array[4]) << 2);
    
    wiichuck_state.C = (0b00000010 & wiichuck_status_result_array[5]) >> 1;
    wiichuck_state.Z = (0b00000001 & wiichuck_status_result_array[5]);
}

void wiichuck_print_raw_motion_status()
{
    char pbuf[80];
    
    sprintf(pbuf, "Wiichuck (jx, jy, ax, ay, az, c, z): %d %d %d %d %d %d %d\r\n", wiichuck_state.JX, wiichuck_state.JY, wiichuck_state.AX, wiichuck_state.AY, wiichuck_state.AZ, wiichuck_state.C, wiichuck_state.Z);
    UART_PutString(pbuf);

}

