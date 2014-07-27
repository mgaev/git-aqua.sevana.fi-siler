#!/bin/bash
echo "USING: gcc-mingw32 package for cross-compilation"
CROSSGPP=/usr/bin/i686-w64-mingw32-g++
#
#
mkdir ./tmp

#
#  For Mains
#
cd ./Mains/
${CROSSGPP} -O2 -c ./SilKiller.cpp -o ../tmp/SilKiller.o

#
#  For VAD
#
cd ../VAD/
${CROSSGPP} -O2 -c ./Codld8cp.cpp -o ../tmp/Codld8cp.o
${CROSSGPP} -O2 -c ./Lpccp.cpp    -o ../tmp/Lpccp.o
${CROSSGPP} -O2 -c ./Tab_ld8k.cpp -o ../tmp/Tab_ld8k.o
${CROSSGPP} -O2 -c ./Vad.cpp      -o ../tmp/Vad.o

#
#  For Wave
#
cd ../Wave/
${CROSSGPP} -O2 -c ./CSmtSamples.cpp     -o ../tmp/CSmtSamples.o
${CROSSGPP} -O2 -c ./TranslateFormat.cpp -o ../tmp/TranslateFormat.o

#
#  Link all
#
cd ../
${CROSSGPP} -O2 -static -o ./SilEr.exe ./tmp/*.o
rm -f ./tmp/*
rmdir ./tmp

