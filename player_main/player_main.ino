/***************************************************
Adafruit "Music Maker" MP3 Shield for Arduino w/3W Stereo Amp - v1.0 https://www.adafruit.com/product/1788
****************************************************/

// include SPI, MP3 and SD libraries
#include <Adafruit_VS1053.h>
#include <SD.h>
#include <SPI.h>

// Pins used for the music maker shield
#define SHIELD_RESET -1 // VS1053 reset pin (unused!)
#define SHIELD_CS 7     // VS1053 chip select pin (output)
#define SHIELD_DCS 6    // VS1053 Data/command select pin (output)
#define CARDCS 4 // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3 // VS1053 Data request, ideally an Interrupt pin

// Buttons Setup (Connected to GPIO pins on the Music Shield):
#define drumButtonPin 2   // a button to switch to drum mode
#define fullSongButtonPin 3   // a button to play the full song preview
#define trackButtonPin 4   // a button to change the track

// Drum Sensor Setup (A Piezo vibration Sensor):
#define sensorPin A0  // select the input pin for the potentiometer
#define drumSensitivity 30 // select the sensitivity of the sensor Higher value = less sensitive
int sensorValue = 0; // variable to store the value coming from the sensor

#define tapDelay 80 // min delay between drum taps
unsigned long time_now = 0; // time counter

// LEDs Setup:
#define ledDelay 100 // delay between LED blinks when animating
int currentLed = 5; // LED to blink - 3 LEDs connected to GPIO 5, 6, 7 on the Music Shield
bool ledsState = false; // LEDs state on or off

// Music Shield Setup
Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);
uint8_t volume = 5;

// Initial play floder and track counter
int playFolder = 1;
int track = 1;

// An array to store the number of files in each track directory
// TODO: find a better way to do this!
const byte maxTracks = 10;
int tackDirsFileCount[maxTracks];


void setup() {
  Serial.begin(9600);
  Serial.println("Acapella Drum !!!");

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
  // printDirectory(SD.open("/"), 0);
  
  // count how many files in each track directory
  TracksFileCounter();

  // Configure buttons pins
  musicPlayer.GPIO_pinMode(trackButtonPin, INPUT);
  musicPlayer.GPIO_pinMode(fullSongButtonPin, INPUT);
  musicPlayer.GPIO_pinMode(drumButtonPin, INPUT);
  
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
  
  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(volume, volume);

  // Ready! 
  PlayTrack("/ready.mp3");
  Serial.println("Ready to Drum !!!");

}


void loop() {
  // Alternately, we can just play an entire file at once
  // This doesn't happen in the background, instead, the entire
  // file is played and the program will continue when it's done!
  // musicPlayer.playFullFile("track001.ogg");

  // CHANGE SONG TRACK
  if (musicPlayer.GPIO_digitalRead(trackButtonPin) > 0) {
  
    PlayTrack("/next.mp3");

    if(tackDirsFileCount[playFolder + 1] > 0 && playFolder < maxTracks - 1) {
      playFolder++;
    } 
    else {
      playFolder = 1;
    }
    
    track = 1;
    
    char folderName[13];
    sprintf(folderName, "Changed to Song %02d", playFolder);
    Serial.println(folderName);
  
    // Blink all LEDS
    for (uint8_t i=5; i<8; i++) { 
      musicPlayer.GPIO_pinMode(i, OUTPUT);
      musicPlayer.GPIO_digitalWrite(i, LOW);
    }
    delay(400);
    for (uint8_t i=5; i<8; i++) { 
      musicPlayer.GPIO_pinMode(i, OUTPUT);
      musicPlayer.GPIO_digitalWrite(i, HIGH);
    }
    delay(400);
  
  }
  
  // PLAY FULL SONG
  if (musicPlayer.GPIO_digitalRead(fullSongButtonPin) > 0) {
    char filename[13];
    sprintf(filename, "/%02d_full.mp3", playFolder);
    Serial.println(filename);
    PlayTrack(filename);
    track = 1;
    delay(200);
  }

  // SWITCH TO DRUM MODE
  if (musicPlayer.GPIO_digitalRead(drumButtonPin) > 0) {
    PlayTrack("/drum.mp3");
    track = 1;
    Serial.println("Drum Mode!");
    delay(200);
  }

  // Animate Leds
  if (track == 1 && musicPlayer.playingMusic == true) {
    ledsState = true;
    musicPlayer.GPIO_pinMode(currentLed, OUTPUT);
    musicPlayer.GPIO_digitalWrite(currentLed, LOW);
    
    if(millis() - time_now > ledDelay) {
      time_now = millis();
      musicPlayer.GPIO_digitalWrite(currentLed, HIGH);
      currentLed++;
      if (currentLed > 7) {
        currentLed = 5;
      }
    }
  } else if (musicPlayer.playingMusic == false && ledsState == true) {
    // turn all LEDs off
    ledsState = false;
    for (uint8_t i=5; i<8; i++) { 
      musicPlayer.GPIO_pinMode(i, OUTPUT);
      musicPlayer.GPIO_digitalWrite(i, HIGH);
    }
  }

  // Listen to DRUM SENSOR:
  sensorValue = analogRead(sensorPin);
  if (sensorValue > drumSensitivity) {
    if(tackDirsFileCount[playFolder] > 0) {
      char filename[13];
      sprintf(filename, "/%02d/track%03d.mp3", playFolder, track);
      Serial.println(filename);

      PlayTrack(filename);

      if (track == tackDirsFileCount[playFolder])
        track = 1;
      else
        track++;
    } else {
      Serial.println("No files in this track");
    }

    // Blink a random LED
    currentLed = random(5, 8);
    musicPlayer.GPIO_pinMode(currentLed, OUTPUT);
    musicPlayer.GPIO_digitalWrite(currentLed, LOW);
    delay(tapDelay);
    musicPlayer.GPIO_digitalWrite(currentLed, HIGH);
  }
}


// Play a track
void PlayTrack(char track_to_play[13]) {

  musicPlayer.stopPlaying();
  delay(10);
  if (!musicPlayer.startPlayingFile(track_to_play)) {
    Serial.println("Could not open file");
    while (1);
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

  // Print summary
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