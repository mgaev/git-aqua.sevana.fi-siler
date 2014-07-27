#!/bin/sh
OBJS=$(pwd)/tmp
rm -rf ${OBJS} # clean up
mkdir -p ${OBJS} # create

#
#  For Mains
#
g++ -O2 -c Mains/SilKiller.cpp -o ${OBJS}/SilKiller.o

#
#  For VAD
#
g++ -O2 -c VAD/Codld8cp.cpp -o ${OBJS}/Codld8cp.o
g++ -O2 -c VAD/Lpccp.cpp    -o ${OBJS}/Lpccp.o
g++ -O2 -c VAD/Tab_ld8k.cpp -o ${OBJS}/Tab_ld8k.o
g++ -O2 -c VAD/Vad.cpp      -o ${OBJS}/Vad.o

#
#  For Wave
#
g++ -O2 -c Wave/CSmtSamples.cpp     -o ${OBJS}/CSmtSamples.o
g++ -O2 -c Wave/TranslateFormat.cpp -o ${OBJS}/TranslateFormat.o

#
#  Link all
#
g++ -O2 -o ./SilEr ${OBJS}/*.o
rm -rf ${OBJS}

