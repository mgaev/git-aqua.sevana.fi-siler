#include <stdio.h>
#include <string.h>
#include <math.h>
#include "WavFiles.h"
#include "CSmtSamples.h"

#define EPSI          1.0e-38        /* very small positive floating point number      */

//extern int32 dFFTHalfBufferSize; // dFFTBufferSize/2

// Определение порядка FFT по частоте дискретизации сигнала
int32 CalculateFFTPower(int32 aSampleRate, int32 & aFFTPower, int32 & aFFTBufferSize, 
                        int32 & aFFTHalfBufferSize, int32 & aFFTDblBufferSize)
{
//  if (dFFTPower == -1)
    {
      aFFTPower = 0;
      while(aSampleRate)
        {
          aFFTPower++;
          aSampleRate = aSampleRate >> 1;
        }

      // Проверка показателя на разумность диапазона
      if      (aFFTPower <  7) aFFTPower =  7;
      else if (aFFTPower > 16) aFFTPower = 16;

      // Размеры буферов...
      aFFTBufferSize     = 1 << aFFTPower;
      aFFTHalfBufferSize = aFFTBufferSize/2;
      aFFTDblBufferSize  = aFFTBufferSize*2;
    }
  return(aFFTPower);
}


// Определение размера файла
// Определение размера файла
int32 m_filesize(FILE *stream)
{
  int32 curpos, length;

  curpos = ftell(stream);
  fseek(stream, 0L, SEEK_END);
  length = ftell(stream);
  fseek(stream, curpos, SEEK_SET);
  return (length);
}

CSmartSamples :: CSmartSamples()
{
  dWaveType   = -1;
  dSampleType = -1;
  //
  dCurType = esstFloat;
  //
  pSamplesByte  = NULL;
  pSamplesShort = NULL;
  pSamplesInt   = NULL;
  pSamplesFloat = NULL;
  //
  dNSamples   = 0;
  dTrueSize   = 0;
  dNChannels  = 0;
  dSampleRate = 0;
  dSampleBits = 0;
  dCFFTPower  = 0;
}

CSmartSamples :: ~CSmartSamples()
{
  if (pSamplesByte)  delete(pSamplesByte);
  if (pSamplesShort) delete(pSamplesShort);
  if (pSamplesInt)   delete(pSamplesInt);
  if (pSamplesFloat) delete(pSamplesFloat);
}

// Сбрасывает все внутренние данные в начальное состояние
void CSmartSamples :: Reset(void)
{
  if (pSamplesByte)  delete(pSamplesByte);
  if (pSamplesShort) delete(pSamplesShort);
  if (pSamplesInt)   delete(pSamplesInt);
  if (pSamplesFloat) delete(pSamplesFloat);
  //
  dCurType = esstFloat;
  //
  pSamplesByte  = NULL;
  pSamplesShort = NULL;
  pSamplesInt   = NULL;
  pSamplesFloat = NULL;
  //
  dNSamples   = 0;
  dTrueSize   = 0;
  dNChannels  = 0;
  dSampleRate = 0;
  dSampleBits = 0;
  dCFFTPower  = 0;
}

// Проверяет наличие базового массива данных
bool CSmartSamples :: Check(void)
{
  if (pSamplesFloat) return(true);
  return(false);
}

// Выполняет усекновение файла
void CSmartSamples :: TrimIt(double aTrmThresh)
{
  if (dCurType == esstShort)
    {
      // Ищем точку начала
      int32  i, j;
      double iTmpD;

      for(i=0; i<dNSamples; i++)
        {
          iTmpD = double(pSamplesShort[i]);
          iTmpD = (iTmpD*iTmpD);

          if (iTmpD < 1.0) iTmpD = 1.0;
          iTmpD = 10.0f*float(log10(iTmpD/240.0 + EPSI));
          if (iTmpD > aTrmThresh) break;
        }

      // Ищем точку конца
      for(j = dNSamples-1; j>=i; j--)
        {
          iTmpD = double(pSamplesShort[j]);
          iTmpD = (iTmpD*iTmpD);

          if (iTmpD < 1.0) iTmpD = 1.0;
          iTmpD = 10.0f*float(log10(iTmpD/240.0 + EPSI));
          if (iTmpD > aTrmThresh) break;
        }

      if ((i > 0)&&(j > i))
        {
          j++;
          memcpy(pSamplesShort, pSamplesShort+i, sizeof(short)*(j - i));
          j -= i;
          memset(pSamplesShort + j, 0, sizeof(short)*(dNSamples - j));

          if (pSamplesFloat)
            {
              memcpy(pSamplesFloat, pSamplesFloat+i, sizeof(float)*j);
              memset(pSamplesFloat + j, 0, sizeof(float)*(dNSamples - j));
            }
          dNSamples = j;
        }
    }
}

void CSmartSamples :: SetFromShort(short * aPSamples, int32 aNSamples)
{
  int32 iFFTPower = -1;
  int32 iFFTBufferSize;
  int32 iFFTHalfBufferSize;
  int32 iFFTDblBufferSize;

  Reset();
  if (aPSamples&&(aNSamples > 0))
    {
      pSamplesFloat = new float[aNSamples];
      for(int i=0; i<aNSamples; i++) pSamplesFloat[i] = float(aPSamples[i])/32768.0f;
      dNSamples   = aNSamples;
      dTrueSize   = aNSamples;
      dNChannels  = 1;
      dSampleRate = 8000;
      dSampleBits = 16;
      //
      dCFFTPower  = CalculateFFTPower(dSampleRate, iFFTPower, iFFTBufferSize, iFFTHalfBufferSize, iFFTDblBufferSize);
      dFFTHalfSize = iFFTHalfBufferSize;
    }
}

void CSmartSamples :: SetFromFloat(float * aPSamples, int32 aNSamples)
{
  int32 iFFTPower = -1;
  int32 iFFTBufferSize;
  int32 iFFTHalfBufferSize;
  int32 iFFTDblBufferSize;

  Reset();
  if (aPSamples&&(aNSamples > 0))
    {
      pSamplesFloat = new float[aNSamples];
      memcpy(pSamplesFloat, aPSamples, sizeof(float)*aNSamples);
      dNSamples   = aNSamples;
      dTrueSize   = aNSamples;
      dNChannels  = 1;
      dSampleRate = 8000;
      dSampleBits = 16;
      //
      dCFFTPower  = CalculateFFTPower(dSampleRate, iFFTPower, iFFTBufferSize, iFFTHalfBufferSize, iFFTDblBufferSize);
      dFFTHalfSize = iFFTHalfBufferSize;
    }
}

bool CSmartSamples :: ApplayFromShort(void)
{
  if (pSamplesFloat&&pSamplesShort&&(dTrueSize > 0))
    {
      for(int i=0; i<dTrueSize; i++)
        pSamplesFloat[i] = float(pSamplesShort[i])/32768.0f;
      return(true);
    }
  return(false);
}

// Возвращает количество отсчетов
int32 CSmartSamples :: GetNSamples(void)
{
  return(dNSamples);
}

// Возвращает количество каналов
int32 CSmartSamples :: GetNChannels(void)
{
  return(dNChannels);
}

// Возвращает частоту дискретизации
int32 CSmartSamples :: GetSampleRate(void)
{
  return(dSampleRate);
}

// Возвращает разрядность отсчетов
int32 CSmartSamples :: GetSampleBits(void)
{
  return(dSampleBits);
}

// Возвращает размер половины буфера FFT (ширина спектра)
int32 CSmartSamples :: GetFFTHalfBufferSize(void)
{
  return(dFFTHalfSize);
}

// Возвращает показатель степени FFT-анализа
int32 CSmartSamples :: GetFFTPower(void)
{
  return(dCFFTPower);
}

// Возвращает тип отсчетов в вав-файле
int32 CSmartSamples :: GetSampleType(void)
{
  return(dSampleType);
}

// Возвращает длительность сигнала в секундах
float CSmartSamples :: GetSoundTime(void)
{
  float iRet;
  if (dSampleRate <= 0) iRet = 0;
  else                  iRet = float(dNSamples)/float(dSampleRate);
  return(iRet);
}

// Возвращает указатель на массив отсчетов в заданном формате
void * CSmartSamples :: GetPSamplesArray(CSmtSmplesTypes aType)
{
  void * iRet;
  int    i;

  switch(aType)
    {
      // отсчеты типа байт
      case esstByte:
        if (pSamplesByte) delete(pSamplesByte);
        pSamplesByte  = new char[dTrueSize];
        memset(pSamplesByte, 0, sizeof(char)*dTrueSize);
        for(i=0; i<dNSamples; i++) pSamplesByte[i] = char(127.0*pSamplesFloat[i]);
        dCurType = esstByte;
        iRet = pSamplesByte;
      break;

      // расширенный целочисленный формат
      case esstInt:
        if (pSamplesInt) delete(pSamplesInt);
        pSamplesInt  = new int[dTrueSize];
        memset(pSamplesByte, 0, sizeof(int)*dTrueSize);
        for(i=0; i<dNSamples; i++) pSamplesByte[i] = int(127.0*pSamplesFloat[i]);
        dCurType = esstInt;
        iRet = pSamplesInt;
      break;

      // отсчеты с плавающей точкой
      case esstFloat:
        dCurType = esstFloat;
        iRet = pSamplesFloat;
      break;

      // обычный формат
      case esstShort:
      default:
        if (pSamplesShort&&(dCurType == esstShort))
          {
            dCurType = esstShort;
            iRet = pSamplesShort;
          }
        else
          {
            if (pSamplesShort) delete(pSamplesShort);
            pSamplesShort  = new short[dTrueSize];
            memset(pSamplesShort, 0, sizeof(short)*dTrueSize);
            for(i=0; i<dNSamples; i++) pSamplesShort[i] = short(32767.0*pSamplesFloat[i]);
            dCurType = esstShort;
            iRet = pSamplesShort;
          }
      break;
    }

  return(iRet);
}

// Рушит временные рабочие структуры после использования
void CSmartSamples :: ResetSamplesArray(void)
{
  if (pSamplesByte)  delete(pSamplesByte);  pSamplesByte  = NULL;
  if (pSamplesShort) delete(pSamplesShort); pSamplesShort = NULL;
  if (pSamplesInt)   delete(pSamplesInt);   pSamplesInt   = NULL;
}

// Принудительно настраивает параметры FFT
void CSmartSamples :: SetFFTPower(int32 aPower)
{
  dCFFTPower   = aPower;
  dFFTHalfSize = 1 << (dCFFTPower - 1);
}

// Читает отсчеты из файла
//float * ReadBothFile(char * aPFileName, int32 & aNSamples, int32 & aSampleRate, int32 & aNCannels)
int CSmartSamples :: ReadFromFile(char * aPFileName, long aDelay)
{
  unsigned char * iPTmpData;
  FILE          * iPFile;
  TWAV_HEADER     iWavHeader;
  int32           iFFTPower = -1;
  int32           iFFTBufferSize;
  int32           iFFTHalfBufferSize;
  int32           iFFTDblBufferSize;

  dNSamples = 0;

  // Открываем с проверкой
  iPFile = fopen(aPFileName, "rb");
  if (!iPFile) return(-1);

  // Читаем заголовок и проверяем тип файла
  if (fread(&iWavHeader, sizeof(TWAV_HEADER), 1, iPFile) != 1)
    {
      fclose(iPFile);
      return(-1);
    }
  fclose(iPFile);

  // Исходный файл - вава
  if ((iWavHeader.RIFF == isRIFF)&&(iWavHeader.WAVE == isWAVE))
       iPTmpData = ReadWaveFile(aPFileName, dNSamples, aDelay), dSampleRate = iWavHeader.Diskret, dNChannels = iWavHeader.NChannel;
  else // Не вава
       iPTmpData = ReadPCMFile(aPFileName, dNSamples, aDelay),  dSampleRate = 8000,  dNChannels = 1;

  if (!iPTmpData) return(-2);

  dCFFTPower  = CalculateFFTPower(dSampleRate, iFFTPower, iFFTBufferSize, iFFTHalfBufferSize, iFFTDblBufferSize);
  dFFTHalfSize = iFFTHalfBufferSize;

  // Переконвертим формату файла
  if (!iPTmpData) return(-3);
  pSamplesFloat = TranslateToNormalFormat(iPTmpData, dNSamples, dTrueSize);
  delete(iPTmpData);
  return(0);
}

int CSmartSamples :: ReadFileHeader(char * aPFileName)
{
  FILE          * iPFile;
  TWAV_HEADER     iWavHeader;

  dNSamples = 0;

  // Открываем с проверкой
  iPFile = fopen(aPFileName, "rb");
  if (!iPFile) return(-1);

  // Читаем заголовок и проверяем тип файла
  if (fread(&iWavHeader, sizeof(TWAV_HEADER), 1, iPFile) != 1)
    {
      fclose(iPFile);
      return(-1);
    }
  fclose(iPFile);

  // Исходный файл - вава
  if ((iWavHeader.RIFF == isRIFF)&&(iWavHeader.WAVE == isWAVE))
    {
      if (!ReadWaveHeader(aPFileName)) return(-2);
      dSampleRate = iWavHeader.Diskret;
      dNChannels  = iWavHeader.NChannel;
    }
  else // Не вава
    {
      dSampleRate = 8000;
      dNChannels  =    1;
      dSampleType =    0;
    }

  return(0);
}

// Читаем звуковой файл со структурой 
unsigned char * CSmartSamples :: ReadWaveFile(char * aPSrcFileName, int32 & aNSamples, long aDelay)
{
  TWAV_HEADER         iWavHeader;
  TVRIFFHeader        iRIFF;
  TVFMTHeader         iFMT;
  TVSomethingHeader   iSometh;
  char                iData[4] = { 0, 0, 0, 0 };
  int32               iSizePos = 0;
  unsigned char     * iPOutData;
  FILE              * iPFile;

  aNSamples = 0;

  // Привязывается к структуре файла
  dWaveType = 1;

  // Открываем с проверкой
  iPFile = fopen(aPSrcFileName, "rb");
  if (!iPFile) return(NULL);

  // Читаем и проверяем заголовок
  // Чтение заголовка
  fseek(iPFile, 0L, SEEK_SET);
  if (fread(&iRIFF, sizeof(TVRIFFHeader), 1, iPFile) != 1) 
    {
      fclose(iPFile);
      return(NULL);
    }
  if (fread(&iFMT, sizeof(TVFMTHeader), 1, iPFile) != 1) 
    {
      fclose(iPFile);
      return(NULL);
    }
  fseek(iPFile, iFMT.Maker - isMaker, SEEK_CUR);

  memset(&iSometh, 0, sizeof(TVSomethingHeader));
  while(iSometh.dDataId != isData)
    {
      if (fread(&iSometh, sizeof(TVSomethingHeader), 1, iPFile) != 1) 
        {
          fclose(iPFile);
          return(NULL);
        }
      if (iSometh.dDataId != isData)
        fseek(iPFile, iSometh.dHedSize, SEEK_CUR);
    }

  iWavHeader.RIFF           = iRIFF.RIFF;
  iWavHeader.FileLen        = iRIFF.FileLen;
  iWavHeader.WAVE           = iRIFF.WAVE;
  iWavHeader.FMT            = iFMT.FMT;
  iWavHeader.Maker          = iFMT.Maker;
  iWavHeader.ModulationType = iFMT.ModulationType;
  iWavHeader.NChannel       = iFMT.NChannel;
  iWavHeader.Diskret        = iFMT.Diskret;
  iWavHeader.Speed          = iFMT.Speed;
  iWavHeader.Bytes          = iFMT.Bytes;
  iWavHeader.Value          = iFMT.Value;
  iWavHeader.Data           = iSometh.dDataId;
  iWavHeader.InfLen         = iSometh.dHedSize;

  if ((iWavHeader.RIFF != isRIFF)||(iWavHeader.WAVE != isWAVE))
    {
      fclose(iPFile);
      return(NULL);
    }

  // Определяем размер
  dNSamples = m_filesize(iPFile);
  if (dNSamples <= 0)
    {
      dNSamples = 0;
      fclose(iPFile);
      return(NULL);
    }

  // Ищем начало данных
  fseek(iPFile, 0, SEEK_SET);
  iSizePos = 0;
  while(!feof(iPFile))
    {
      iData[0] = iData[1]; iData[1] = iData[2]; iData[2] = iData[3];
      if (fread(iData + 3, 1, 1, iPFile) != 1) break;
      iSizePos++;
      if ((iData[0] == 'd')&&(iData[1] == 'a')&&(iData[2] == 't')&&(iData[3] == 'a'))
      break;
    }

  // Проверяем структуру
  if ((iData[0] != 'd')||(iData[1] != 'a')||(iData[2] != 't')||(iData[3] != 'a'))
    {
      aNSamples = 0;
      fclose(iPFile);
      return(NULL);
    }

  // читаем размер данных
  if (fread(&aNSamples, sizeof(int32), 1, iPFile) != 1) 
    {
      aNSamples = 0;
      fclose(iPFile);
      return(NULL);
    }
  if (aNSamples <= 0)
    {
      aNSamples = 0;
      fclose(iPFile);
      return(NULL);
    }

  // Хватаем параметры исходного файла
  if (iWavHeader.NChannel < 1) iWavHeader.NChannel = 1;
  aDelay *= iWavHeader.Bytes;
  iWavHeader.Bytes /= iWavHeader.NChannel;

  // Учитываем смещение
  aNSamples -= aDelay;
  if (aNSamples <= 0)
    {
      aNSamples = 0;
      fclose(iPFile);
      return(NULL);
    }
  if (aDelay > 0) fseek(iPFile, aDelay, SEEK_CUR);

  // Выделяем память
  iPOutData = new unsigned char[aNSamples];
  if (!iPOutData)
    {
      aNSamples = 0;
      fclose(iPFile);
      return(NULL);
    }

  // Читаем звуковые данные
  aNSamples = fread(iPOutData, 1, aNSamples, iPFile);
  fclose(iPFile);
  if (aNSamples <= 0)
    {
      aNSamples = 0;
      delete(iPOutData);
      return(NULL);
    }

  dSampleType = -1;
  if (iWavHeader.ModulationType ==     1)
    {
      if      (iWavHeader.Bytes ==     2) dSampleType = 0;
      else if (iWavHeader.Bytes ==     4) dSampleType = 4;
      else                                dSampleType = 1;
    }
  else
  if (iWavHeader.ModulationType ==     6) dSampleType = 2;
  else
  if (iWavHeader.ModulationType ==     7) dSampleType = 3;
  else
  if (iWavHeader.ModulationType ==     3) dSampleType = 4;
  else
  if (iWavHeader.ModulationType == 65534) dSampleType = 5;

  if (dSampleType == -1)
    {
      aNSamples = 0;
      delete(iPOutData);
      return(NULL);
    }

  return(iPOutData);
}

bool CSmartSamples :: ReadWaveHeader(char * aPSrcFileName)
{
  TWAV_HEADER         iWavHeader;
  TVRIFFHeader        iRIFF;
  TVFMTHeader         iFMT;
  TVSomethingHeader   iSometh;
  char                iData[4] = { 0, 0, 0, 0 };
  int32               iSizePos = 0;
  FILE              * iPFile;

  int32               iNSamples = 0;

  // Привязывается к структуре файла
  dWaveType = 1;

  // Открываем с проверкой
  iPFile = fopen(aPSrcFileName, "rb");
  if (!iPFile) return(false);

  // Читаем и проверяем заголовок
  // Чтение заголовка
  fseek(iPFile, 0L, SEEK_SET);
  if (fread(&iRIFF, sizeof(TVRIFFHeader), 1, iPFile) != 1) 
    {
      fclose(iPFile);
      return(false);
    }
  if (fread(&iFMT, sizeof(TVFMTHeader), 1, iPFile) != 1) 
    {
      fclose(iPFile);
      return(false);
    }
  fseek(iPFile, iFMT.Maker - isMaker, SEEK_CUR);

  memset(&iSometh, 0, sizeof(TVSomethingHeader));
  while(iSometh.dDataId != isData)
    {
      if (fread(&iSometh, sizeof(TVSomethingHeader), 1, iPFile) != 1) 
        {
          fclose(iPFile);
          return(false);
        }
      if (iSometh.dDataId != isData)
        fseek(iPFile, iSometh.dHedSize, SEEK_CUR);
    }

  iWavHeader.RIFF           = iRIFF.RIFF;
  iWavHeader.FileLen        = iRIFF.FileLen;
  iWavHeader.WAVE           = iRIFF.WAVE;
  iWavHeader.FMT            = iFMT.FMT;
  iWavHeader.Maker          = iFMT.Maker;
  iWavHeader.ModulationType = iFMT.ModulationType;
  iWavHeader.NChannel       = iFMT.NChannel;
  iWavHeader.Diskret        = iFMT.Diskret;
  iWavHeader.Speed          = iFMT.Speed;
  iWavHeader.Bytes          = iFMT.Bytes;
  iWavHeader.Value          = iFMT.Value;
  iWavHeader.Data           = iSometh.dDataId;
  iWavHeader.InfLen         = iSometh.dHedSize;

  if ((iWavHeader.RIFF != isRIFF)||(iWavHeader.WAVE != isWAVE))
    {
      fclose(iPFile);
      return(false);
    }

  // Определяем размер
  dNSamples = m_filesize(iPFile);
  if (dNSamples <= 0)
    {
      dNSamples = 0;
      fclose(iPFile);
      return(false);
    }

  // Ищем начало данных
  fseek(iPFile, 0, SEEK_SET);
  iSizePos = 0;
  while(!feof(iPFile))
    {
      iData[0] = iData[1]; iData[1] = iData[2]; iData[2] = iData[3];
      if (fread(iData + 3, 1, 1, iPFile) != 1) break;
      iSizePos++;
      if ((iData[0] == 'd')&&(iData[1] == 'a')&&(iData[2] == 't')&&(iData[3] == 'a'))
      break;
    }

  // Проверяем структуру
  if ((iData[0] != 'd')||(iData[1] != 'a')||(iData[2] != 't')||(iData[3] != 'a'))
    {
      iNSamples = 0;
      fclose(iPFile);
      return(false);
    }

  // читаем размер данных
  if (fread(&iNSamples, sizeof(int32), 1, iPFile) != 1) 
    {
      fclose(iPFile);
      return(false);
    }

  fclose(iPFile);
  if (iNSamples <= 0) return(false);

  // Хватаем параметры исходного файла
  if (iWavHeader.NChannel < 1) iWavHeader.NChannel = 1;
  iWavHeader.Bytes /= iWavHeader.NChannel;

  dSampleType = -1;
  if (iWavHeader.ModulationType ==     1)
    {
      if      (iWavHeader.Bytes ==     2) dSampleType = 0;
      else if (iWavHeader.Bytes ==     4) dSampleType = 4;
      else                                dSampleType = 1;
    }
  else
  if (iWavHeader.ModulationType ==     6) dSampleType = 2;
  else
  if (iWavHeader.ModulationType ==     7) dSampleType = 3;
  else
  if (iWavHeader.ModulationType ==     3) dSampleType = 4;
  else
  if (iWavHeader.ModulationType == 65534) dSampleType = 5;

  if (dSampleType == -1) return(false);

  return(true);
}

unsigned char * CSmartSamples :: ReadPCMFile(char * aPSrcFileName, int32 & aNSamples, long aDelay)
{
  unsigned char * iPOutData;
  FILE          * iPFile;

  dSampleType = 0;
  dWaveType   = 0;
  aNSamples   = 0;

  // Открываем файл с проверкой
  iPFile = fopen(aPSrcFileName, "rb");
  if (!iPFile) return(NULL);

  // Проверяем размер
  aNSamples = m_filesize(iPFile);
  if (aNSamples <= 0)
    {
      aNSamples = 0;
      fclose(iPFile);
      return(NULL);
    }

  // Учитывапем задержку
  aDelay *= 16;
  aNSamples -= aDelay;
  if (aNSamples <= 0)
    {
      aNSamples = 0;
      fclose(iPFile);
      return(NULL);
    }
  if (aDelay > 0) fseek(iPFile, aDelay, SEEK_SET);

  // Выделяем память
  iPOutData = new unsigned char[aNSamples];
  if (!iPOutData)
    {
      aNSamples = 0;
      fclose(iPFile);
      return(NULL);
    }

  // Читаем звуковые данные
  aNSamples = fread(iPOutData, 1, aNSamples, iPFile);
  fclose(iPFile);
  if (aNSamples <= 0)
    {
      aNSamples = 0;
      delete(iPOutData);
      return(NULL);
    }

  return(iPOutData);
}

// Считает энергию
float CSmartSamples :: CalculateEnergy(void)
{
/*
  float iEnergy = 1.0;

  for(int i=0; i<dNSamples; i++)
    iEnergy += (pSamplesFloat[i] * pSamplesFloat[i]);

  iEnergy /= float(dNSamples);
  return(iEnergy);
*/
  float iEnergy = 1.0;
  float iMaxEnergy = 1.0;
  long  iCntr = 0;
  int   i;

  for(i=0; i<dNSamples; i++)
    iEnergy += fabs(pSamplesFloat[i]);

  iEnergy /= float(dNSamples);


  for(i=1; i<dNSamples-1; i++)
    if ((pSamplesFloat[i] > iEnergy)&&(pSamplesFloat[i] > pSamplesFloat[i-1])&&(pSamplesFloat[i] > pSamplesFloat[i+1]))
      {
        iMaxEnergy += fabs(pSamplesFloat[i]);
        iCntr++;
      }

  if (iCntr > 0) iMaxEnergy /= float(iCntr);
  else           iMaxEnergy  = 1.0;
  return(iMaxEnergy);
}

// Умножает все отсчеты на коэффициент
void CSmartSamples :: AplayCoefficient(float aCoeff)
{
  for(int i=0; i<dNSamples; i++) pSamplesFloat[i] *= aCoeff;
}

