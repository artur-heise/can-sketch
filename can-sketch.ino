#include <can.h>              // Include the CAN library
#include <mcp2515.h>          // Include the MCP2515 library
#include <SPI.h>              // Include the SPI library

struct can_frame canMsg;      // Create a structure for the received CAN frame
struct can_frame new_canMsg;  // Create a structure for the CAN frame to send

MCP2515 mcp2515(10);          //  

void setup()                  // Configure the serial interface and CAN bus
{
  Serial.begin(115200);
  SPI.begin();
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS);
  mcp2515.setNormalMode();
}

int front_panel_command;      // Declare the FMUX panel command variable (byte 5, zero-based)
int steer_key_1;              // Declare the steering wheel button command variable
int steer_key_2;              // Declare the steering wheel button command variable
int volume;                   // Declare the variable for the current volume level

void loop()                   // Start the CAN bus data-reading loop
{
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK)          // Continue if there are no errors
  {
    if (canMsg.can_id == 0x122)                                 // If the frame came from the FMUX panel
    { 
      // Determine how volume is controlled. This determines which command to send on behalf of the
      // FMUX panel so the steering wheel volume buttons work correctly.
      if(canMsg.data[5] == 0)                                 // This value is present when the car wakes up if volume has not
      // yet been adjusted using the FMUX panel or steering wheel (via Arduino)
      {
        steer_key_1 = 0x08;                                 // Assign the volume-up button value
        steer_key_2 = 0x04;                                 // Assign the volume-down button value
        front_panel_command = 0x02;                         // Assign the value 0 (byte 5)
      }
      
      else if(canMsg.data[5] == 2)                            // This condition is true if volume has already been adjusted using
      // the FMUX panel or steering wheel (via Arduino)
      {
        steer_key_1 = 0x04;                                 // Assign the volume-down button value
        steer_key_2 = 0x08;                                 // Assign the volume-up button value
        front_panel_command = 0x00;                         // Assign the value 0 (byte 5)
      }
      
      else
      {
        // Do nothing; there is no other data
      }
      
      volume = canMsg.data[6];                                // Store the current FMUX panel volume level
    }
    
    if (canMsg.can_id == 0x21F)                                 // If the frame came from the steering wheel buttons
    {     
      if (canMsg.data[0] == steer_key_1)                      // Determine which button was pressed
      {                
        // The FMUX panel volume value cycles from 0 to 255. After 255, the next value is 0, and vice versa.
        if (volume == 255)
        {
          volume = 0;
        }
        
        else
        {
          volume++;   
        }
                
        new_canMsg.can_id  = 0x122;                  // Assign the FMUX panel ID
        new_canMsg.can_dlc = 8;                      // Set the frame length
        new_canMsg.data[0] = 0x00;                   // Empty data
        new_canMsg.data[1] = 0x00;                   // Empty data
        new_canMsg.data[2] = 0x00;                   // Empty data
        new_canMsg.data[3] = 0x00;                   // Empty data
        new_canMsg.data[4] = 0x00;                   // Empty data
        new_canMsg.data[5] = front_panel_command;    // Panel command
        new_canMsg.data[6] = volume;                 // Assign the new volume level
        new_canMsg.data[7] = 0x00;                   // Empty data
        
        mcp2515.sendMessage(&new_canMsg);            // Send the command to change the volume level
        
        delay(150);                                  // 0.15-second delay
      }
      
      if (canMsg.data[0] == steer_key_2)                      // Determine which button was pressed
      {       
        if (volume == 0)
        {
          volume = 255;
        }
        
        else
        {
          volume--;
        }
        
        new_canMsg.can_id  = 0x122;                  // Assign the FMUX panel ID
        new_canMsg.can_dlc = 8;                      // Set the frame length
        new_canMsg.data[0] = 0x00;                   // Empty data
        new_canMsg.data[1] = 0x00;                   // Empty data
        new_canMsg.data[2] = 0x00;                   // Empty data
        new_canMsg.data[3] = 0x00;                   // Empty data
        new_canMsg.data[4] = 0x00;                   // Empty data
        new_canMsg.data[5] = front_panel_command;    // Panel command
        new_canMsg.data[6] = volume;                 // Assign the new volume level
        new_canMsg.data[7] = 0x00;                   // Empty data
        
        mcp2515.sendMessage(&new_canMsg);            // Send the command to change the volume level
        
        delay(150);                                  // 0.15-second delay
      }
    }
  }
}
