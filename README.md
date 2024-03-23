# Voice Drum 
A prototype for a drum toy


## Technicall Details
Below is some information about the technical details of the project

### Hardware:
- 1: The project is based on the Arduino microcontroller
- 2: The sound is played using the [Adafruit "Music Maker" MP3 Shield for Arduino](https://www.adafruit.com/product/1788)

### SD card and files format notes:
- 1: All sound files should be in the .mp3 format
- 2: Each song needs to have a folder with named in the ```XX``` format, where XX is a number from 01 to 99
- 3: Inside each folder each slice of the song should be named in the following format: ```trackXXX.mp3``` where XXX is a number from 001 to 999
- 3: Each songs needs to include a full version named ```XX_full.mp3```, where XX is the same number as the folder

**IMPORTANT:**
If you copy the files to the SD card using a MacOS, the files will be copied with a hidden file called ```.DS_Store```. This file will cause the device to crash. To avoid this, you can use the following command to remove all the hidden files from the SD card:
```
cd /Volumes/NO\ NAME
dot_clean ./
``` 