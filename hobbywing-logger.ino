#include <SPI.h>
#include <SD.h>


#define FILE_NAME "/data2.txt"

#define BUFFER_SIZE (32)
#define RESULT_SIZE (8)
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






#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define SCK  25
#define MISO  27
#define MOSI  26
#define CS  33

SPIClass spi = SPIClass(VSPI);

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if(!root){
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void createDir(fs::FS &fs, const char * path){
  Serial.printf("Creating Dir: %s\n", path);
  if(fs.mkdir(path)){
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char * path){
  Serial.printf("Removing Dir: %s\n", path);
  if(fs.rmdir(path)){
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

void readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)){
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char * path){
  Serial.printf("Deleting file: %s\n", path);
  if(fs.remove(path)){
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void testFileIO(fs::FS &fs, const char * path){
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if(file){
    len = file.size();
    size_t flen = len;
    start = millis();
    while(len){
      size_t toRead = len;
      if(toRead > 512){
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    Serial.printf("%u bytes read for %u ms\n", flen, end);
    file.close();
  } else {
    Serial.println("Failed to open file for reading");
  }


  file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }

  size_t i;
  start = millis();
  for(i=0; i<2048; i++){
    file.write(buf, 512);
  }
  end = millis() - start;
  Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
  file.close();
}

void setup(){
  Serial.begin(115200);
  spi.begin(SCK, MISO, MOSI, CS);

  if (!SD.begin(CS,spi,80000000)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  // listDir(SD, "/", 0);
  // createDir(SD, "/mydir");
  // listDir(SD, "/", 0);
  // removeDir(SD, "/mydir");
  // listDir(SD, "/", 2);
  // writeFile(SD, "/hello.txt", "Hello ");
  // appendFile(SD, "/hello.txt", "World!\n");
  // readFile(SD, "/hello.txt");
  // deleteFile(SD, "/foo.txt");
  // renameFile(SD, "/hello.txt", "/foo.txt");
  // readFile(SD, "/foo.txt");
  // testFileIO(SD, "/test.txt");
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));



  appendFile(SD, FILE_NAME, "***NEW RECORD***\n");


  Serial1.begin(115200, SERIAL_8N1, GPIO_NUM_32, GPIO_NUM_12); //This is the UART, pipes to sensors attached to board
  Serial.println("Initialize Serial Hardware UART Pins");

}










void setup2()
{
  Serial.begin(9600); //This pipes to the serial monitor
  Serial.println("Initialize Serial Monitor");

  Serial1.begin(115200, SERIAL_8N1, GPIO_NUM_32, GPIO_NUM_12); //This is the UART, pipes to sensors attached to board
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
      
      File file = SD.open(FILE_NAME, FILE_APPEND);
      if(!file){
        Serial.println("Failed to open file for appending");
        return;
      }
      
      file.print(diff);
      file.print(",");
      file.println(dupeCount);
      file.close();

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
    result[6] = buffer[19];  // temperature
    result[7] = buffer[21];  // motorTemp
    
    uint32_t diff = prevMs == 0 ? 0 : currMs - prevMs;
    printResult(diff);

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

void printResult(uint32_t diff) {
  File file = SD.open(FILE_NAME, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;
  }
  
  file.print(diff);
  file.print(",0,");


  // Print the contents of the buffer
  for (int i = 0; i < RESULT_SIZE; i++) {
    if (result[i] < 0x10) {
      file.print("0"); // Print leading zero for single digit
    }
    file.print(result[i], HEX);
    file.print(" ");
  }
  file.println();
  file.close();
}

int i = 0;
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


