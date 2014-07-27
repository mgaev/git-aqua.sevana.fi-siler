#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Wave/WavFiles.h"
#include "../Wave/CSmtSamples.h"
#include "../VAD/Vad.h"

// Значения VAD
short * pVadis      = NULL;
long    dVaSize     =    0;
long    dLBoundSave =    3;
long    dRBoundSave =    3;
long    dMinSil     =    3;

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
  if (iFramesCount > 1.0) iAvgEnergy /= iFramesCount;

  // Настраиваем ВАД
  mVAD.Init();
  mVAD.SetEnergyBound(iAvgEnergy*0.85);

  dVaSize = (dEOFData / L_FRAME) + 1;
  pVadis  = new short[dVaSize];
  memset(pVadis, 0, dVaSize*sizeof(short));

  // заполняем масив флагов
  for(i=j=0; i<dEOFData; i+= L_FRAME, j++) pVadis[j] = mVAD.GoOne(iPDataShort + i);

  return(0);
}

// Сохраняет левые границы
int SaveLBonds(void)
{
  long i, iCnt;

  for(i=dVaSize-1; i>0; i--)
    if ((pVadis[i] == 1)&&(pVadis[i-1] == 0))
      {
        iCnt = dLBoundSave;
        for(i--; (i>0)&&(iCnt>0); iCnt--, i--) pVadis[i] = 1;
      }

  return(0);
}

// Сохраняет правые границы
int SaveRBonds(void)
{
  long i, iCnt;

  for(i=0; i<dVaSize-1; i++)
    if ((pVadis[i] == 1)&&(pVadis[i+1] == 0))
      {
        iCnt = dRBoundSave;
        for(i++; (i<dVaSize-1)&&(iCnt>0); iCnt--, i++) pVadis[i] = 1;
      }

  return(0);
}

// Сохраняет короткие паузы
int SaveMinSil(void)
{
  long i, j, iCnt;

  for(i=0; i<dVaSize-1; i++)
    if ((pVadis[i] == 1)&&(pVadis[i+1] == 0))
      {
        i++; iCnt = dMinSil; j=i;
        while((i<dVaSize)&&(pVadis[i] == 0)) iCnt--, i++;
        if (iCnt >= 0)
          {
            for(; j<i; j++) pVadis[j] = 1;
          }
      }

  return(0);
}

// Вывод подсказки
void PrintHelp(void)
{
  printf("usage:\n");
  printf("SilEr <SrcWave> <OutWave>\n");// <LBoundSave> <RBoundSave> <MinSil>\n");
  printf("\t<SrcWave> - input sound file;\n");
  printf("\t<OutWave> - output sound file.\n");
  //printf("\t<LBoundSave> - number of frames saved in the beginning of the voiced signal parts;\n");
  //printf("\t<RBoundSave> - number of frames saved in the end of the voiced signal parts;\n");
  //printf("\t<MinSil> - Maximal duration for saved pauses in frames.\n");
}

int main(int argc, char * argv[])
{
  CSmartSamples        iInData;

  printf("Sevana Silence Eraser - SilEr v.1.00.11.\n");
  printf("Copyright (c) 2012 by Sevana Ltd, Finland, Estonia. All rights reserved.\n");

  if ((argc != 3)&&(argc != 6))
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

  if (argc == 6)
    {
      dLBoundSave = strtol(argv[3], NULL, 10);
      dRBoundSave = strtol(argv[4], NULL, 10);
      dMinSil     = strtol(argv[5], NULL, 10);
    }

  if (ProcessVAD(&iInData) != 0)
    {
      iInData.Reset();
      if (pVadis) delete(pVadis);
      printf("Sound data processing error!\n");
      return(0);
    }

  // Сохраняем левые границы
  if (dLBoundSave >= 1)
    {
      if (SaveLBonds() != 0)
        {
          iInData.Reset();
          if (pVadis) delete(pVadis);
          printf("Left bounds saving error!\n");
          return(0);
        }
    }

  // Сохраняем правые границы
  if (dRBoundSave >= 1)
    {
      if (SaveRBonds() != 0)
        {
          iInData.Reset();
          if (pVadis) delete(pVadis);
          printf("Right bounds saving error!\n");
          return(0);
        }
    }

  // Сохраняем короткие паузы
  if (dMinSil >= 1)
    {
      if (SaveMinSil() != 0)
        {
          iInData.Reset();
          if (pVadis) delete(pVadis);
          printf("Short pauses saving error!\n");
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
