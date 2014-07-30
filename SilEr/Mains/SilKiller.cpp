#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Wave/WavFiles.h"
#include "../Wave/CSmtSamples.h"
#include "../VAD/Vad.h"

// Значения VAD
short * pVadis       = NULL;
long    dVaSize      =    0;
float   dLBoundSaveF = 0.03;
float   dRBoundSaveF = 0.03;
float   dMinSilF     = 0.03;
float   dStartTimeF  = -1.0;
float   dEndTimeF    = -1.0;
float   dSilEnergyF  = -1.0;
float   dAvgECoeffF  = 0.85;
bool    isVAD        = true;
bool    isTwoPassVAD = true;

int WriteWave(char * aPName, CSmartSamples * aPData)
{
  TWAV_HEADER         iOutWHeader;
  FILE              * iPOutWFile;
  short  * iPDataShort;
  long     i, j, dEOFData;

  iPOutWFile = fopen(aPName, "w+b");
  if (!iPOutWFile) return(-1);

  // Формируем заголовок
  iOutWHeader.RIFF           = isRIFF;
  iOutWHeader.FileLen        = sizeof(TWAV_HEADER) - 2*sizeof(int32);
  iOutWHeader.WAVE           = isWAVE;
  iOutWHeader.FMT            = isFMT;
  iOutWHeader.Maker          = isMaker;
  iOutWHeader.ModulationType = 1;
  iOutWHeader.NChannel       = 1;
  iOutWHeader.Diskret        = aPData->GetSampleRate();
  iOutWHeader.Speed          = aPData->GetSampleRate()*sizeof(short);
  iOutWHeader.Bytes          = isBytesPCM;
  iOutWHeader.Value          = isValuePCM;
  iOutWHeader.Data           = isData;
  iOutWHeader.InfLen         = 0;

  // Готовим звуковые данные
  dEOFData    = aPData->GetNSamples() - L_WINDOW; // 1024
  iPDataShort = (short *)aPData->GetPSamplesArray(esstShort);

  // заполняем колличество отсчетов в заголовке
  for(i=j=0; i<dEOFData; i+= L_FRAME, j++)
    if (pVadis[j] == 1)
      {
        iOutWHeader.FileLen += L_FRAME*sizeof(short);
        iOutWHeader.InfLen  += L_FRAME*sizeof(short);
      }

  // И записываем заголовок
  if (fwrite(&iOutWHeader, sizeof(TWAV_HEADER), 1, iPOutWFile) != 1) return(-2);

  // записываем звук
  for(i=j=0; i<dEOFData; i+= L_FRAME, j++)
    if (pVadis[j] == 1)
      {
        if (fwrite(iPDataShort+i, sizeof(short), L_FRAME, iPOutWFile) != L_FRAME) return(-3);
      }

  fclose(iPOutWFile);

  return(0);
}

int ProcessVAD(CSmartSamples * aPData)
{
  CVAD     mVAD;
  double   iAvgEnergy, iFramesCount, iTmpD;
  short  * iPDataShort;
  long     i, j, dEOFData;

  // Готовим ВАД
  mVAD.Init();

  // Готовим звуковые данные
  dEOFData    = aPData->GetNSamples() - L_WINDOW; // 1024
  iPDataShort = (short *)aPData->GetPSamplesArray(esstShort);

  // Вычисляем энергию звука
  iAvgEnergy = iFramesCount = 0;
  for(i=0; i<dEOFData; i+= L_FRAME, iFramesCount+=1.0)
    {
      iTmpD = mVAD.GoPreEnergy(iPDataShort + i);
      if      (iTmpD <   1.0) iTmpD =   1;
      else if (iTmpD > 100.0) iTmpD = 100;
      iAvgEnergy += iTmpD;
    }
  if (iFramesCount > 1.0) 
    {
      iAvgEnergy /= iFramesCount;
      iAvgEnergy *= dAvgECoeffF;
    }
  


  dVaSize = (dEOFData / L_FRAME) + 1;
  pVadis  = new short[dVaSize];
  memset(pVadis, 0, dVaSize*sizeof(short));

  if (isVAD||isTwoPassVAD)
    {
      // Настраиваем ВАД
      mVAD.Init();

      if (dSilEnergyF < 21.0) mVAD.SetEnergyBound(iAvgEnergy);
      else                    mVAD.SetEnergyBound(dSilEnergyF);

      // заполняем масив флагов
      for(i=j=0; i<dEOFData; i+= L_FRAME, j++) pVadis[j] = mVAD.GoOne(iPDataShort + i);

      if (isTwoPassVAD)
        {
          mVAD.Init();

          if (dSilEnergyF < 21.0) mVAD.SetEnergyBound(iAvgEnergy);
          else                    mVAD.SetEnergyBound(dSilEnergyF);

          for(i-=L_FRAME, j--; (i>0)&&(j>0); i-=L_FRAME, j--) pVadis[j] = mVAD.GoOneBack(iPDataShort + i);
        }
    }
  else
    {
      for(i=j=0; i<dEOFData; i+= L_FRAME,j++)
        {
          iTmpD = mVAD.GoPreEnergy(iPDataShort + i);

          if (iTmpD < dSilEnergyF) pVadis[j] = 0;
          else                     pVadis[j] = 1;
        }
    }


  return(0);
}

// Сохраняет левые границы
int SaveLBonds(CSmartSamples * aPData)
{
  long i, iCnt;
  long iLBoundSave;

  iLBoundSave = (L_FRAME - 1 + dLBoundSaveF * aPData->GetSampleRate() * aPData->GetNChannels()) / L_FRAME;

  for(i=dVaSize-1; i>0; i--)
    if ((pVadis[i] == 1)&&(pVadis[i-1] == 0))
      {
        iCnt = iLBoundSave;
        for(i--; (i>0)&&(iCnt>0); iCnt--, i--) pVadis[i] = 1;
      }

  return(0);
}

// Сохраняет правые границы
int SaveRBonds(CSmartSamples * aPData)
{
  long i, iCnt;
  long iRBoundSave;

  iRBoundSave = (L_FRAME - 1 + dRBoundSaveF * aPData->GetSampleRate() * aPData->GetNChannels()) / L_FRAME;

  for(i=0; i<dVaSize-1; i++)
    if ((pVadis[i] == 1)&&(pVadis[i+1] == 0))
      {
        iCnt = iRBoundSave;
        for(i++; (i<dVaSize-1)&&(iCnt>0); iCnt--, i++) pVadis[i] = 1;
      }

  return(0);
}

// Сохраняет короткие паузы
int SaveMinSil(CSmartSamples * aPData)
{
  long i, j, iCnt;
  long iMinSil;

  iMinSil = (L_FRAME - 1 + dMinSilF * aPData->GetSampleRate() * aPData->GetNChannels()) / L_FRAME;

  for(i=0; i<dVaSize-1; i++)
    if ((pVadis[i] == 1)&&(pVadis[i+1] == 0))
      {
        i++; iCnt = iMinSil; j=i;
        while((i<dVaSize)&&(pVadis[i] == 0)) iCnt--, i++;
        if (iCnt >= 0)
          {
            for(; j<i; j++) pVadis[j] = 1;
          }
      }

  return(0);
}

int RemoveStartTime(CSmartSamples * aPData)
{
  long i, j;

  j = (dStartTimeF * aPData->GetSampleRate() * aPData->GetNChannels()) / L_FRAME;
  for(i=0; (i<dVaSize-1)&&(i < j); i++) pVadis[i] = 0;

  return(0);
}

int RemoveEndTime(CSmartSamples * aPData)
{
  long i, j;

  j = (dEndTimeF * aPData->GetSampleRate() * aPData->GetNChannels()) / L_FRAME;
  for(i=j; (i<dVaSize-1); i++) pVadis[i] = 0;

  return(0);
}

// Вывод подсказки
void PrintHelp(void)
{
  printf("usage:\n");
  printf("SilEr <SrcWave> <OutWave> [options]\n");// <LBoundSave> <RBoundSave> <MinSil>\n");
  printf("\t<SrcWave> - input sound file;\n");
  printf("\t<OutWave> - output sound file.\n");

  //printf("\t--minsil - Set Maximal duration for saved pauses (default 0.03 sec);\n");
  //printf("\t--lbound - Set Saved time in the beginning of the voiced signal parts (default 0.03 sec);\n");
  //printf("\t--rbound - Set Saved time in the end of the voiced signal parts (default 0.03 sec);\n");
  //printf("\t--starttime - Set time of begining processed voice signal (default -1.0 - begin of signal);\n");
  //printf("\t--endtime - Set time of ending processed voice signal (default -1.0 - end of signal);\n");
  //printf("\t--silenergy - Set Vfximal energy of (default -1.0);\n");
  //printf("\t--avgecoeff - (default 0.85);\n");
  //printf("\t--vadtype - (default 2 sec);\n");
}

int ProcessOptions(int argc, char * argv[])
{
  int i;

  for(i=1; i<argc; i++)
    {
      if (strlen(argv[i]) < 3) continue;
      if ((argv[i][0] == '-')&&(argv[i][1] == '-'))
        {
          if (strcmp(argv[i], "--minsil")) 
            {
              dMinSilF = atof(argv[i + 1]);
              i++;
              continue;
            }
          else
          if (strcmp(argv[i], "--lbound")) 
            {
              dLBoundSaveF = atof(argv[i + 1]);
              i++;
              continue;
            }
          else
          if (strcmp(argv[i], "--rbound")) 
            {
              dRBoundSaveF = atof(argv[i + 1]);
              i++;
              continue;
            }
          else
          if (strcmp(argv[i], "--starttime")) 
            {
              dStartTimeF = atof(argv[i + 1]);
              i++;
              continue;
            }
          else
          if (strcmp(argv[i], "--endtime")) 
            {
              dEndTimeF = atof(argv[i + 1]);
              i++;
              continue;
            }
          else
          if (strcmp(argv[i], "--silenergy")) 
            {
              dSilEnergyF = atof(argv[i + 1]);
              i++;
              continue;
            }
          else
          if (strcmp(argv[i], "--avgecoeff")) 
            {
              dAvgECoeffF = atof(argv[i + 1]);
              i++;
              continue;
            }
          else
          if (strcmp(argv[i], "--vadtype")) 
            {
              switch(atoi(argv[i + 1]))
                {
                  case    1:
                    isVAD        =  true;
                    isTwoPassVAD = false;
                  break;

                  case    2:
                    isVAD        =  true;
                    isTwoPassVAD =  true;
                  break;

                  case    0:
                  default  :
                    isVAD        = false;
                    isTwoPassVAD = false;
                  break;
                };
              i++;
              continue;
            }
        }
    }
  return(0);
}


int main(int argc, char * argv[])
{
  CSmartSamples        iInData;

  printf("Sevana Silence Eraser - SilEr v.1.00.11.\n");
  printf("Copyright (c) 2012 by Sevana Ltd, Finland, Estonia. All rights reserved.\n");

  if (argc < 3)
    {
      PrintHelp();
      return(0);
    }

  if (iInData.ReadFromFile(argv[1], 0) != 0)
    {
      iInData.Reset();
      printf("Cannot open input file!\n");
      return(0);
    }

  if (ProcessVAD(&iInData) != 0)
    {
      iInData.Reset();
      if (pVadis) delete(pVadis);
      printf("Sound data processing error!\n");
      return(0);
    }

  // Сохраняем левые границы
  if (dLBoundSaveF > 0.0)
    {
      if (SaveLBonds(&iInData) != 0)
        {
          iInData.Reset();
          if (pVadis) delete(pVadis);
          printf("Left bounds saving error!\n");
          return(0);
        }
    }

  // Сохраняем правые границы
  if (dRBoundSaveF > 0.0)
    {
      if (SaveRBonds(&iInData) != 0)
        {
          iInData.Reset();
          if (pVadis) delete(pVadis);
          printf("Right bounds saving error!\n");
          return(0);
        }
    }

  // Сохраняем короткие паузы
  if (dMinSilF > 0.0)
    {
      if (SaveMinSil(&iInData) != 0)
        {
          iInData.Reset();
          if (pVadis) delete(pVadis);
          printf("Short pauses saving error!\n");
          return(0);
        }
    }

  // Убираем начало файла
  if (dStartTimeF > 0.0)
    {
      if (RemoveStartTime(&iInData) != 0)
        {
          iInData.Reset();
          if (pVadis) delete(pVadis);
          printf("Start time removing error!\n");
          return(0);
        }
    }

  // Убираем конец файла
  if (dEndTimeF > 0.0)
    {
      if (RemoveEndTime(&iInData) != 0)
        {
          iInData.Reset();
          if (pVadis) delete(pVadis);
          printf("End time removing error!\n");
          return(0);
        }
    }

  // Записываем звуковой - файл результат обработки
  if (WriteWave(argv[2], &iInData) != 0)
    {
      iInData.Reset();
      if (pVadis) delete(pVadis);
      printf("Cannot write output file!\n");
      return(0);
    }

  iInData.Reset();
  if (pVadis) delete(pVadis);
  return(0);
}
