#!/bin/sh
g++ -O3 -I../include -L../bin/x64/release blocks.main.cpp blocks.cpp -llibruntime -oblocks -lstdc++
g++ -O3 -I../include -L../bin/x64/release travel.main.cpp travel.cpp -llibruntime -otravel -lstdc++
