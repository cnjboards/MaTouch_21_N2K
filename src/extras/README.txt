This directory contains extra files that need to be placed in the NMEA2000-library directory.
Files in this directory are not to be compiled here, but moved to the NMEA2000-library directory and compiled there.
The files should be moved after the "NMEA2000-library" has been downloaded from github.

List of files:
"NMEA2000_can.h": This file lives in ".pio\libdeps\esp32s3box\NMEA2000-library\src". This replaces the existing version. It has changes to use the new ESP32-S3 TWAI driver files.

"NMEA2000_esp32_twai.cpp": This file lives in ".pio\libdeps\esp32s3box\NMEA2000-library\src". It is the ESP32-S3 TWAI driver source.

"NMEA2000_esp32_twai.h": This file lives in ".pio\libdeps\esp32s3box\NMEA2000-library\src". It is the ESP32-S3 TWAI driver header.
