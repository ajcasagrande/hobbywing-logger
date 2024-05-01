#include <SPI.h>
#include <SD.h>


#define FILE_NAME "data.bin"

#define BUFFER_SIZE (32)
#define RESULT_SIZE (9)
#define MAX_DUPES (100)

File dataFile;

uint8_t buffer[BUFFER_SIZE];
uint8_t buffer1[BUFFER_SIZE];
int bufferIndex = 0;
bool bufferFlag = false;
uint32_t prevMs = 0;
uint32_t currMs = 0;
uint32_t dupePrevMs = 0;
uint32_t dupeCount = 0;

uint8_t result[RESULT_SIZE];

void setup()
{
  Serial.begin(9600); //This pipes to the serial monitor
  Serial.println("Initialize Serial Monitor");

  Serial1.begin(115200); //This is the UART, pipes to sensors attached to board
  Serial.println("Initialize Serial Hardware UART Pins");


  // Initialize SD card
  if (!SD.begin()) {
    Serial.println("SD card initialization failed!");
    return;
  }
  
  // Open the data file
  dataFile = SD.open(FILE_NAME, FILE_WRITE);
  if (!dataFile) {
    Serial.println("Error opening data file!");
    return;
  }
}


void handleBuffer() {
  bool isDupe = 0;
  if (memcmp(buffer, buffer1, BUFFER_SIZE*sizeof(uint8_t)) == 0) {
    // we have a dupe!
    isDupe = 1;
    dupeCount++;
    dupePrevMs = currMs;
    if (dupeCount < MAX_DUPES) {
      return;
    }
  }

    if (dupeCount > 0) {
      uint32_t diff = isDupe ? currMs - prevMs : dupePrevMs - prevMs;
      Serial.print(diff);
      Serial.print(",");
      Serial.println(dupeCount);
      dupeCount = 0;
      prevMs = dupePrevMs;
      if (isDupe) {
        prevMs = currMs;
        // we are still a dupe, but wanted to print out current progress
        return;
      }
    }

    result[0] = buffer[11];  // status: neutral=0, gas=1, brake=2
    result[1] = buffer[9];   // throttle input
    result[2] = buffer[10];  // throttle output
    result[3] = buffer[13];  // rpm low-byte
    result[4] = buffer[14];  // rpm high-byte
    result[5] = buffer[15];  // volt low-byte
    result[6] = buffer[16];  // volt high-byte
    result[7] = buffer[19];  // temperature
    result[8] = buffer[21];  // motorTemp
    
    uint32_t diff = prevMs == 0 ? 0 : currMs - prevMs;
    Serial.print(diff);
    Serial.print(",0,");
    printResult();

    prevMs = currMs;
    // copy to prev buffer
    memcpy(buffer1, buffer, BUFFER_SIZE*sizeof(uint8_t));
}

void writeBufferToFile() {
  // Write buffer to file in binary format
  dataFile.write(buffer, bufferIndex);
  dataFile.flush(); // Flush buffer to ensure data is written immediately
}

void printBuffer() {
  // Print the contents of the buffer
  for (int i = 0; i < BUFFER_SIZE; i++) {
    if (buffer[i] < 0x10) {
      Serial.print("0"); // Print leading zero for single digit
    }
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void printResult() {
  // Print the contents of the buffer
  for (int i = 0; i < RESULT_SIZE; i++) {
    if (result[i] < 0x10) {
      Serial.print("0"); // Print leading zero for single digit
    }
    Serial.print(result[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void loop() {
  // while (Serial1.available()) {
  //   Serial.print(Serial1.read(), HEX);
  // }
  // // Serial.println();
  // return;


  // Read data from Serial1 and print as hexadecimal
  while (Serial1.available()) {
    uint8_t receivedByte = Serial1.read();
    // Print each byte as a two-digit hexadecimal number
    // if (receivedByte < 0x10) {
    //   Serial.print("0"); // Print leading zero for single digit
    // }
    if (receivedByte == (uint8_t)0xFE) {
      // Serial.println();
      bufferIndex = 0;
      bufferFlag = true;
      currMs = micros();
    }
    // Serial.print(receivedByte, HEX);
    // Serial.print(" ");

      if (bufferFlag) {
        buffer[bufferIndex++] = receivedByte;
        if (bufferIndex >= BUFFER_SIZE) {
          // Call callback function to print buffer
          handleBuffer();
          // printBuffer();
          // writeBufferToFile();
          bufferFlag = false;
        }
      }
  }
}


