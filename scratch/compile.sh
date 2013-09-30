#!/bin/sh
g++ -O3 -I../include -L../bin/x64/release blocks.main.cpp -llibruntime -oblocks -lstdc++
