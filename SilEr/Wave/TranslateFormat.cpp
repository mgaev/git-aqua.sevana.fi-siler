#include <stdio.h>
#include <string.h>
#include <math.h>
#include "WavFiles.h"
#include "CSmtSamples.h"

#define cmFFTDblBufferSize    512
#define cmFFTBufferSize       256

static float iMuTable[256] = 
{
   -32124, -31100, -30076, -29052,
   -28028, -27004, -25980, -24956, -23932, -22908, -21884, -20860,
   -19836, -18812, -17788, -16764, -15996, -15484, -14972, -14460,
   -13948, -13436, -12924, -12412, -11900, -11388, -10876, -10364,
   -9852, -9340, -8828, -8316, -7932, -7676, -7420, -7164, -6908,
   -6652, -6396, -6140, -5884, -5628, -5372, -5116, -4860, -4604,
   -4348, -4092, -3900, -3772, -3644, -3516, -3388, -3260, -3132,
   -3004, -2876, -2748, -2620, -2492, -2364, -2236, -2108, -1980,
   -1884, -1820, -1756, -1692, -1628, -1564, -1500, -1436, -1372,
   -1308, -1244, -1180, -1116, -1052, -988, -924, -876, -844, -812,
   -780, -748, -716, -684, -652, -620, -588, -556, -524, -492, -460,
   -428, -396, -372, -356, -340, -324, -308, -292, -276, -260, -244,
   -228, -212, -196, -180, -164, -148, -132, -120, -112, -104, -96,
   -88, -80, -72, -64, -56, -48, -40, -32, -24, -16, -8, 0, 32124,
   31100, 30076, 29052, 28028, 27004, 25980, 24956, 23932, 22908,
   21884, 20860, 19836, 18812, 17788, 16764, 15996, 15484, 14972,
   14460, 13948, 13436, 12924, 12412, 11900, 11388, 10876, 10364,
   9852, 9340, 8828, 8316, 7932, 7676, 7420, 7164, 6908, 6652, 6396,
   6140, 5884, 5628, 5372, 5116, 4860, 4604, 4348, 4092, 3900, 3772,
   3644, 3516, 3388, 3260, 3132, 3004, 2876, 2748, 2620, 2492, 2364,
   2236, 2108, 1980, 1884, 1820, 1756, 1692, 1628, 1564, 1500, 1436,
   1372, 1308, 1244, 1180, 1116, 1052, 988, 924, 876, 844, 812,
   780, 748, 716, 684, 652, 620, 588, 556, 524, 492, 460, 428, 396,
   372, 356, 340, 324, 308, 292, 276, 260, 244, 228, 212, 196, 180,
   164, 148, 132, 120, 112, 104, 96, 88, 80, 72, 64, 56, 48, 40,
   32, 24, 16, 8, 0
};

static float iAlTable[256]=
{
   -5504, -5248, -6016, -5760, -4480, -4224, -4992, -4736,
   -7552, -7296, -8064, -7808, -6528, -6272, -7040, -6784,
   -2752, -2624, -3008, -2880, -2240, -2112, -2496, -2368,
   -3776, -3648, -4032, -3904, -3264, -3136, -3520, -3392,
   -22016,-20992,-24064,-23040,-17920,-16896,-19968,-18944,
   -30208,-29184,-32256,-31232,-26112,-25088,-28160,-27136,
   -11008,-10496,-12032,-11520, -8960, -8448, -9984, -9472,
   -15104,-14592,-16128,-15616,-13056,-12544,-14080,-13568,
   -344,  -328,  -376,  -360,  -280,  -264,  -312,  -296,
   -472,  -456,  -504,  -488,  -408,  -392,  -440,  -424,
   -88,   -72,   -120,  -104,  -24,   -8,    -56,   -40,
   -216,  -200,  -248,  -232,  -152,  -136,  -184,  -168,
   -1376, -1312, -1504, -1440, -1120, -1056, -1248, -1184,
   -1888, -1824, -2016, -1952, -1632, -1568, -1760, -1696,
   -688,  -656,  -752,  -720,  -560,  -528,  -624,  -592,
   -944,  -912,  -1008, -976,  -816,  -784,  -880,  -848,
   5504,  5248,  6016,  5760,  4480,  4224,  4992,  4736,
   7552,  7296,  8064,  7808,  6528,  6272,  7040,  6784,
   2752,  2624,  3008,  2880,  2240,  2112,  2496,  2368,
   3776,  3648,  4032,  3904,  3264,  3136,  3520,  3392,
   22016, 20992, 24064, 23040, 17920, 16896, 19968, 18944,
   30208, 29184, 32256, 31232, 26112, 25088, 28160, 27136,
   11008, 10496, 12032, 11520, 8960,  8448,  9984,  9472,
   15104, 14592, 16128, 15616, 13056, 12544, 14080, 13568,
   344,   328,   376,   360,   280,   264,   312,   296,
   472,   456,   504,   488,   408,   392,   440,   424,
   88,    72,    120,   104,   24,    8,     56,    40,
   216,   200,   248,   232,   152,   136,   184,   168,
   1376,  1312,  1504,  1440,  1120,  1056,  1248,  1184,
   1888,  1824,  2016,  1952,  1632,  1568,  1760,  1696,
   688,   656,   752,   720,   560,   528,   624,   592,
   944,   912,  1008,   976,   816,   784,   880,   848 
};


// Приводим к отсчетам в формате "флоат"
float * CSmartSamples :: TranslateToNormalFormat(unsigned char * aPFileBytes, 
                                    int32 & aNSamples, int32 & aTrueSize)
{
  float         * iPOutData = NULL;
  short         * iPInSamplesShort;
  float         * iPInSamplesFloat;
  float           iTmpF, iTmpF2;
  short           iIPrt;
  unsigned char   iFPrt;
  int32           i, j, k;

  switch(dSampleType)
    {
      case 0: // "16bit"
        dNSamples &= 0x7FFFFFFEL;
        dNSamples /= sizeof(short);
        dNSamples /= dNChannels;
        if (dNChannels == 1)
          {
            iPOutData = new float[cmFFTDblBufferSize + aNSamples];
            memset(iPOutData, 0, (cmFFTDblBufferSize + aNSamples)*sizeof(float));
            iPInSamplesShort = (short *)aPFileBytes;
            for(i=0; i<dNSamples; i++)
              iPOutData[i] = float(iPInSamplesShort[i])/32768.0f;
          }
        else
          {
            iPOutData   = new float[cmFFTDblBufferSize + aNSamples];
            iPInSamplesShort = (short *)aPFileBytes;
            memset(iPOutData, 0, (cmFFTDblBufferSize + aNSamples)*sizeof(float));

            for(i=j=0; i<aNSamples; i++)
              {
                iTmpF = 0;
                for(k=0; k<dNChannels; k++, j++) iTmpF += float(iPInSamplesShort[j]);
                iTmpF /= (float)dNChannels;
                iPOutData[i] = iTmpF/32768.0f;
              }
          }
        aNSamples += cmFFTBufferSize;
      break;

      case 1: // "8bit"
        aNSamples /= dNChannels;
        iPOutData = new float[cmFFTDblBufferSize + aNSamples];
        memset(iPOutData, 0, sizeof(float)*(cmFFTDblBufferSize + aNSamples));
        if (dNChannels == 1)
          {
            for(i=0; i<aNSamples; i++) iPOutData[i] = 256.0f * (float(aPFileBytes[i]) - 128.0f);
          }
        else
          {
            for(i=j=0; i<aNSamples; i++)
              {
                iTmpF = 0;
                for(k=0; k<dNChannels; k++, j++) iTmpF += (float)(256.0 * ((float)(aPFileBytes[j]) - 128.0));
                iTmpF /= (float)dNChannels;
                iPOutData[i] = iTmpF/32768.0f;
              }
          }
        aNSamples += cmFFTBufferSize;
      break;

      case 2: // "a-law"
        aNSamples /= dNChannels;
        iPOutData = new float[cmFFTDblBufferSize + aNSamples];
        memset(iPOutData, 0, sizeof(float)*(cmFFTDblBufferSize + aNSamples));
        if (dNChannels == 1)
          {
            for(i=0; i<aNSamples; i++) iPOutData[i] = iAlTable[aPFileBytes[i]]/32768.0f;
          }
        else
          {
            for(i=j=0; i<aNSamples; i++)
              {
                iTmpF = 0;
                for(k=0; k<dNChannels; k++, j++) iTmpF += iAlTable[aPFileBytes[j]];
                iTmpF /= (float)dNChannels;
                iPOutData[i] = iTmpF/32768.0f;
              }
          }
        aNSamples += cmFFTBufferSize;
      break;

      case 3: // "u-law"
        aNSamples /= dNChannels;
        iPOutData = new float[cmFFTDblBufferSize + aNSamples];
        memset(iPOutData, 0, sizeof(float)*(cmFFTDblBufferSize + aNSamples));
        if (dNChannels == 1)
          {
            for(i=0; i<aNSamples; i++) iPOutData[i] = iMuTable[aPFileBytes[i]]/32768.0f;
          }
        else
          {
            for(i=j=0; i<aNSamples; i++)
              {
                iTmpF = 0;
                for(k=0; k<dNChannels; k++, j++) iTmpF += iMuTable[aPFileBytes[j]];
                iTmpF /= (float)dNChannels;
                iPOutData[i] = iTmpF/32768.0f;
              }
          }
        aNSamples += cmFFTBufferSize;
      break;

      case 4: // "32bit"
        aNSamples &= 0x7FFFFFFEL;
        aNSamples /= sizeof(float);
        aNSamples /= dNChannels;
        if (dNChannels == 1)
          {
            iPOutData = new float[cmFFTDblBufferSize + aNSamples];
            memset(iPOutData, 0, (cmFFTDblBufferSize + aNSamples)*sizeof(float));
            iPInSamplesFloat = (float *)aPFileBytes;
            memcpy(iPOutData, iPInSamplesFloat, aNSamples*sizeof(float));
          }
        else
          {
            iPOutData   = new float[cmFFTDblBufferSize + aNSamples];
            iPInSamplesFloat = (float *)aPFileBytes;
            memset(iPOutData, 0, (cmFFTDblBufferSize + aNSamples)*sizeof(float));

            for(i=j=0; i<aNSamples; i++)
              {
                iTmpF = 0;
                for(k=0; k<dNChannels; k++, j++) iTmpF += iPInSamplesFloat[j];
                iPOutData[i] = iTmpF /= (float)dNChannels;
              }
          }
        aNSamples += cmFFTBufferSize;
      break;

      case 5: // "24bit"
        aNSamples &= 0x7FFFFFFEL;
        aNSamples /= 3;
        aNSamples /= dNChannels;

        iPOutData   = new float[cmFFTDblBufferSize + aNSamples];
        memset(iPOutData, 0, (cmFFTDblBufferSize + aNSamples)*sizeof(float));

        for(i=j=0; i<aNSamples; i++)
          {
            iTmpF = 0;
            for(k=0; k<dNChannels; k++, j++, aPFileBytes+=3) 
              {
                iTmpF2 = 0;
                memcpy(&iFPrt, aPFileBytes+0, 1);
                memcpy(&iIPrt, aPFileBytes+1, 2);
                iTmpF2 = float(iIPrt) + float(iFPrt)/256.0f;
                iTmpF += iTmpF2/32768.0f;
              }
            iPOutData[i] = iTmpF /= (float)dNChannels;
          }
        aNSamples += cmFFTBufferSize;
      break;
    }
  aTrueSize = cmFFTDblBufferSize + aNSamples - cmFFTBufferSize;

  return(iPOutData);
}
