#include <stdlib.h>

void setup() {
  // Initialize serial communication at 9600 baud rate
  Serial.begin(9600);
  // Seed the random number generator
  randomSeed(0);
}

void loop() {
  // Generate a random integer between 0 and 99
  int randomNumber = random(100);
  // Print the random number to the serial port
  Serial.print(randomNumber);
  // Wait for 1 second
  delay(1000);
}
