/***************************************************
Adafruit VS1053 Codec Breakout:  https://www.adafruit.com/products/1381
****************************************************/

// include SPI, MP3 and SD libraries
#include <Adafruit_VS1053.h>
#include <SD.h>
#include <SPI.h>

// These are the pins used for the breakout example
#define BREAKOUT_RESET 9 // VS1053 reset pin (output)
#define BREAKOUT_CS 10   // VS1053 chip select pin (output)
#define BREAKOUT_DCS 8   // VS1053 Data/command select pin (output)
// These are the pins used for the music maker shield
#define SHIELD_RESET -1 // VS1053 reset pin (unused!)
#define SHIELD_CS 7     // VS1053 chip select pin (output)
#define SHIELD_DCS 6    // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 4 // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3 // VS1053 Data request, ideally an Interrupt pin

// Drum Sensor Setup
int sensorPin = A0;  // select the input pin for the potentiometer
int sensorValue = 0; // variable to store the value coming from th

// Buttons Setup:
int buttonPin = 1;   // the number of the pushbutton pin
int buttonState = 0; // variable for reading the pushbutton status

// LEDs Setup:
int ledPin = 13; // the number of the LED pin

// Music Shield Setup
Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);
uint8_t volume = 5;

// Initial play floder and track counter
int playFolder = 2;
int track = 1;

// An array to store the number of files in each track directory
// TODO: find a better way to do this!
const byte maxTracks = 10;
int tackDirsFileCount[maxTracks];


void setup() {
  Serial.begin(9600);
  Serial.println("Adafruit VS1053 Library Test");

  // initialise the music player
  if (!musicPlayer.begin()) { // initialise the music player
    Serial.println(
        F("Couldn't find VS1053, do you have the right pins defined?"));
    while (1)
      ;
  }
  Serial.println(F("VS1053 found"));

  musicPlayer.sineTest(0x44, 500); // Make a tone to indicate VS1053 is working

  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1)
      ; // don't do anything more
  }
  Serial.println("SD OK!");

  // list files on SD card
  printDirectory(SD.open("/"), 0);
  
  // count how many files in each track directory
  TracksFileCounter();

  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(volume, volume);

  /***** Two interrupt options! *******/
  // This option uses timer0, this means timer1 & t2 are not required
  // (so you can use 'em for Servos, etc) BUT millis() can lose time
  // since we're hitchhiking on top of the millis() tracker
  // musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT);

  // This option uses a pin interrupt. No timers required! But DREQ
  // must be on an interrupt pin. For Uno/Duemilanove/Diecimilla
  // that's Digital #2 or #3
  // See http://arduino.cc/en/Reference/attachInterrupt for other pins
  // *** This method is preferred
  if (!musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT))
    Serial.println(F("DREQ pin is not an interrupt pin"));
}

void loop() {
  // Alternately, we can just play an entire file at once
  // This doesn't happen in the background, instead, the entire
  // file is played and the program will continue when it's done!
  // musicPlayer.playFullFile("track001.ogg");

  // read the state of the sensorValue:
  sensorValue = analogRead(sensorPin);
  if (sensorValue > 20) {

    if(tackDirsFileCount[playFolder] > 0) {
      char filename[13];
      sprintf(filename, "/%02d/track%03d.mp3", playFolder, track);
      Serial.println(filename);

      musicPlayer.stopPlaying();
      delay(10);
      if (!musicPlayer.startPlayingFile(filename)) {
        Serial.println("Could not open file");
        while (1);
      }

      if (track == tackDirsFileCount[playFolder])
        track = 1;
      else
        track++;
    } else {
      Serial.println("No files in this track");
    }
    

    delay(10);

    // Serial.println(filename);
    // Serial.println(F("Started playing"));

    // while (musicPlayer.playingMusic) {
    //   // file is now playing in the 'background' so now's a good time
    //   // to do something else like handling LEDs or buttons :)
    //   Serial.print(".");
    //   delay(10);
    // }

    // Serial.println("Done playing music");
  }
}

// Count the number of files in each track directory
void TracksFileCounter() {
  for (int i = 0; i < maxTracks; i++) {
    char dirName[13];
    sprintf(dirName, "/%02d", i);
    File dir = SD.open(dirName);
    if (dir) {
      tackDirsFileCount[i] = countFilesInDirectory(dir);
      dir.close();      
    }
  }
  
  // print summary
  for(int i = 0; i < maxTracks; i++) {
    Serial.print("Track ");
    Serial.print(i);
    Serial.print(" has ");
    Serial.print(tackDirsFileCount[i]);
    Serial.println(" files");
  }

}

// TODO: find abetter way to do the file listing!
// Count the number of files in a directory
int countFilesInDirectory(File dir) {
  int fileCount = 0;
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      // no more files
      // Serial.println("**nomorefiles**");
      break;
    }
    fileCount++;
    entry.close();
  }
  // Serial.println(fileCount);
  return fileCount;
}

/// File listing helper
void printDirectory(File dir, int numTabs) {

  while (true) {

    File entry = dir.openNextFile();
    if (!entry) {
      // no more files
      // Serial.println("**nomorefiles**");
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
    
}
