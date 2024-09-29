/* this is a list of all changes needed to get this build to work

1: There is no esp32-s3 support in the nmea2000_esp32 driver library. 
You need to manually add 2 files to the nmea2000 src directory, 
NMEA2000_esp32_twai.cpp and NMEA2000_esp32_twai.h. 
You also must change the existing NMEA2000_can.h to include the 
new twai driver. These are all in the src/extras directory

2: */