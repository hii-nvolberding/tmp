#include <stdlib.h>
// Define compile flag for mode selection 
// Uncomment one of the following lines to select mode 
// #define MODE_BROADCAST 
#define MODE_REPLY

// Packet structure constants
#define PACKET_SIZE 46
#define HEADER_BYTE1 0xFD
#define HEADER_BYTE2 0x08
#define BAUD_RATE 460800
#define CRC_EXTRA 0xB9
#define SEQ_INDEX 4

byte packet = {0xFD,  // STX
                0x22, // LEN
                0x00, // INC FLAGS
                0x00, // CMP FLAGS
                0x03, // SEQ
                0xFF, // SYS ID
                0x44, // COMP ID
                0x46, // MSG ID (low bits)
                0x00, // MSG ID (med bits)
                0x00, // MSG ID (high bits)
                0xD4, // Payload (unmodified)
                0x05, 
                0x91, 
                0x05, 
                0xDD, 
                0x03, 
                0x45, 
                0x06, 
                0xE8, 
                0x03, 
                0xDC, 
                0x05, 
                0xE8,  
                0x03, 
                0xE8, 
                0x03, 
                0x01, 
                0x00, 
                0xDC, 
                0x05, 
                0xDC, 
                0x05, 
                0xDC, 
                0x05, 
                0xFD, 
                0x05, 
                0x71, 
                0x03, 
                0x71, 
                0x03, 
                0x71, 
                0x03, 
                0x71, 
                0x03, 
                0x34, // checksum (low byte)
                0x6B}; // checksum (high byte)

void setup() {
    Serial.begin(115200);
    transmitPacket();
   // Initialize serial communication
   Serial.begin(BAUD_RATE);

   // Initialize packet with default values
   initializePacket();

   // Seed the random number generator
   randomSeed(0);
}

void loop() {
   #ifdef MODE_BROADCAST
     // Broadcast mode: continuously send packets without delay
     updatePacket();
     transmitPacket();
   #endif

   #ifdef MODE_REPLY
     // Reply mode: listen for incoming packets and respond
     if (Serial.available() >= 5) {
       // Check for header bytes
       if (Serial.read() == HEADER_BYTE1) {
         if (Serial.read() == HEADER_BYTE2) {

           // Skip bytes at index 2 and 3
           Serial.read();
           Serial.read();

           // Read byte at index 4
           byte valueAtIndex4 = Serial.read();

           // Clear any remaining bytes in the receive buffer
           while (Serial.available()) {
             Serial.read();
           }

           // Update packet with the new counter value
           packet[SEQ_INDEX] = valueAtIndex4 + 1;

           // Update random values and CRC
           updateRandomValues();
           updateCRC();

           // Transmit the packet
           transmitPacket();
         }
       }
     }
   #endif
}

void initializePacket() {
   // Set header bytes
   packet[0] = HEADER_BYTE1;
   packet[1] = HEADER_BYTE2;
   packet[2] = 0; // Counter starts at 0
   packet[3] = 0; // Reserved byte

   // Initialize remaining bytes to 0
   for (int i = 4; i < PACKET_SIZE; i++) {
     packet[i] = 0;
   }
}

void updatePacket() {
   #ifdef MODE_BROADCAST
     // In broadcast mode, increment counter by 1
     packet[SEQ_INDEX]++;
   #endif

   updateRandomValues();
   updateCRC();
}

void updateRandomValues() {
   // Randomize bytes from index 4 to 17
   for (int i = 10; i <= PACKET_SIZE-3; i++) {
     packet[i] = (i%2) ? random(200, 256) : random(1, 10); 
   }
}

void updateCRC() {
   // Calculate CRC checksum of first 18 bytes plus CRC_EXTRA
   uint16_t crc = calculateCRC(packet, 18, CRC_EXTRA);

   packet[PACKET_SIZE-2] = crc & 0xFF;         // Low byte
   packet[PACKET_SIZE-1] = (crc >> 8) & 0xFF;  // High byte
   
}

void transmitPacket() {
   // Send the entire packet over serial
   Serial.write(packet, PACKET_SIZE);
   Serial.flush();
}

uint16_t calculateCRC(byte* data, int length, byte crc_extra) {
   // CRC-16/MCRF4XX implementation
   uint16_t crc = 0xFFFF;

   for (int i = 0; i < length; i++) {
     crc = crc ^ ((uint16_t)data[i]);
     for (int j = 0; j < 8; j++) {
       if (crc & 0x0001) {
         crc = (crc >> 1) ^ 0x8408; // 0x8408 is the reversed polynomial for CRC-16/MCRF4XX
       } else {
         crc = crc >> 1;
       }
     }
   }

   // Add the CRC_EXTRA byte
   crc = crc ^ ((uint16_t)crc_extra);
   for (int j = 0; j < 8; j++) {
     if (crc & 0x0001) {
       crc = (crc >> 1) ^ 0x8408;
     } else {
       crc = crc >> 1;
     }
   }

   return crc;
}

