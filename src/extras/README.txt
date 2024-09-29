This directory contains extra files that need to be placed in library directories.
Files in this directory is not to be compiled here, but moved to thier respective directory
and compiled there. 
List of files:
NMEA2000_can.h - this lives here pio/libdeps/esp32s3box/NMEA2000-library/src. This replaces the existing version.
            It has changes to include the twai driver files.
NMEA2000_esp32_twai.cpp - this lives here pio/libdeps/esp32s3box/NMEA2000-library/src. This is a new twai driver for the S3 
            version of esp32.
NMEA2000_esp32_twai.h - this lives here pio/libdeps/esp32s3box/NMEA2000-library/src. This is a new twai driver for the S3 
            version of esp32.