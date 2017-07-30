/* 

Work in process
Use PSoC USB MIDI device to play soft synth
Have Wii nunchuck control pitch, velocity via joystic
Have note on/off controlled by button
Have effects triggered by acceleration sensor values


*/
#include "project.h"
#include "wiichuck.h"
#include "i2c_helper.h"

volatile uint32_t i2c_ms_counter; // 16 bits gives up to 32 seconds on the positive side

void systick_handler()
{
    i2c_ms_counter++;
}


void MIDI_test()
{
        uint8_t midiOn[] = {0x90 | 0x01, 60, 127}; // channel 1, middle C, full velocity
        uint8_t midiOff[] = {0x80 | 0x01, 60, 0}; // channel 1, middle C, full velocity
            
        UART_PutString("MIDI Test! ");
        
        // put a message on the buffer from PSoC to the PC
        uint8_t t = USBMIDI_PutUsbMidiIn(3, midiOn, USBMIDI_MIDI_CABLE_00);
        
        if (t == USBMIDI_TRUE)
        {
            UART_PutString("True \r\n");
        }
        else
        {
            UART_PutString("False \r\n");
        }
        
        // this processes the buffer to the PC. If you don't call this it will wait until the buffer is full
        USBMIDI_MIDI_IN_Service();
        
        CyDelay(1000);
        
        t = USBMIDI_PutUsbMidiIn(3, midiOff, USBMIDI_MIDI_CABLE_00);
        if (t == USBMIDI_TRUE)
        {
            UART_PutString("True \r\n");
        }
        else
        {
            UART_PutString("False \r\n");
        }
        USBMIDI_MIDI_IN_Service();
        
        CyDelay(1000);    
}

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    USBMIDI_Start(0, USBMIDI_5V_OPERATION);
    UART_Start();
    USBMIDI_Init();
    I2C_Start();
    
    // these are for the I2C timeout used in i2c_helper
    CySysTickSetCallback(0, systick_handler);
    CySysTickStart();
    CySysTickEnable();

    UART_PutString("Initializing wiichuck\r\n");
    wiichuck_begin(); // initialize the wiichuck

    for(;;)
    {
        wiichuck_get_raw_motion_status();
      
        
        wiichuck_print_raw_motion_status();
        
        CyDelay(10);
    }
}

