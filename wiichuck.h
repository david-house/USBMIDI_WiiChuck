// This is a C port from the WiiChuck class on arduino.cc
// https://playground.arduino.cc/Main/WiiChuckClass
// The base code is attributed to those authors
// This is just a port for use in PSoC


// include guard
#ifndef WiiChuck_h
#define WiiChuck_h
    
// PSoC project API
#include "project.h"
#include "i2c_helper.h"
    
#define WIICHUCK_I2C_ADDRESS 0x52

// these may need to be adjusted for each nunchuck for calibration
#define ZEROX 510  
#define ZEROY 490
#define ZEROZ 460
#define RADIUS 210  // probably pretty universal
#define DEFAULT_ZERO_JOY_X 124
#define DEFAULT_ZERO_JOY_Y 132

uint8_t cnt;
uint8_t wiichuck_status_request_array[1];
uint8_t wiichuck_status_result_array[6];              // array to store wiichuck output
uint8_t averageCounter;
int i;
int total;
uint8_t zeroJoyX;   // these are about where mine are
uint8_t zeroJoyY; // use calibrateJoy when the stick is at zero to correct
int lastJoyX;
int lastJoyY;
int angles[3];


typedef struct {
    uint8_t JX, JY;
    uint16_t AX, AY, AZ;
    uint8_t C, Z;
} WiiChuck_Status;

WiiChuck_Status wiichuck_state;

uint8_t lastZ, lastC; // converted from bool


uint8_t joyX;
uint8_t joyY;
uint8_t buttonZ; // converted from bool
uint8_t buttonC; // converted from bool

uint8_t initialization_buffer[2][2];
    
void wiichuck_begin();
void wiichuck_get_status();
void wiichuck_print_i2c_status();
void wiichuck_get_raw_motion_status();
void wiichuck_print_raw_motion_status();
void wiichuck_get_motion_status();


#endif
    