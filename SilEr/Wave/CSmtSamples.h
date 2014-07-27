#ifndef __C_Smt_Samples_h__
#define __C_Smt_Samples_h__

// Множество типов отсчетов
enum CSmtSmplesTypes
{
  esstByte,    // отсчеты типа байт
  esstShort,   // обычный формат
  esstInt,     // расширенный целочисленный формат
  esstFloat,   // отсчеты с плавающей точкой
};

// Виртуальный класс для работы с отсчетами
class CSamplesVirt
{
  public:
    // Возвращает количество отсчетов
    virtual int32 GetNSamples(void) = 0;
    // Возвращает количество каналов
    virtual int32 GetNChannels(void) = 0;
    // Возвращает частоту дискретизации
    virtual int32 GetSampleRate(void) = 0;
    // Возвращает разрядность отсчетов
    virtual int32 GetSampleBits(void) = 0;
    // Возвращает размер половины буфера FFT (ширина спектра)
    virtual int32 GetFFTHalfBufferSize(void)=0;
    // Возвращает показатель степени FFT-анализа
    virtual int32 GetFFTPower(void)=0;
    // Возвращает тип отсчетов в вав-файле
    virtual int32 GetSampleType(void)=0;

    // Возвращает длительность сигнала в секундах
    virtual float GetSoundTime(void)=0;

    // Возвращает указатель на массив отсчетов в заданном формате
    virtual void * GetPSamplesArray(CSmtSmplesTypes aType) = 0;
    // Рушит временные рабочие структуры после использования
    virtual void   ResetSamplesArray(void) = 0;
};

// Реализация класса для работы с отсчетами
class CSmartSamples : public CSamplesVirt
{
  private:
    int32             dWaveType;
    int32             dSampleType;

  public:
    CSmtSmplesTypes   dCurType;
    char            * pSamplesByte;
    short           * pSamplesShort;
    int             * pSamplesInt;
    float           * pSamplesFloat;
    //
    int32             dNSamples;
    int32             dTrueSize;
    int32             dNChannels;
    int32             dSampleRate;
    int32             dSampleBits;
    int32             dFFTHalfSize;
    int32             dCFFTPower;

  public:
    CSmartSamples();
   ~CSmartSamples();

    // Сбрасывает все внутренние данные в начальное состояние
    void Reset(void);
    // Проверяет наличие базового массива данных
    bool Check(void);

    // Выполняет усекновение файла
    void TrimIt(double aTrmThresh);

    void SetFromShort(short * aPSamples, int32 aNSamples);
    void SetFromFloat(float * aPSamples, int32 aNSamples);

    bool ApplayFromShort(void);

    // Возвращает количество отсчетов
    virtual int32 GetNSamples(void);
    // Возвращает количество каналов
    virtual int32 GetNChannels(void);
    // Возвращает частоту дискретизации
    virtual int32 GetSampleRate(void);
    // Возвращает разрядность отсчетов
    virtual int32 GetSampleBits(void);
    // Возвращает размер половины буфера FFT (ширина спектра)
    virtual int32 GetFFTHalfBufferSize(void);
    // Возвращает показатель степени FFT-анализа
    virtual int32 GetFFTPower(void);
    // Возвращает тип отсчетов в вав-файле
    virtual int32 GetSampleType(void);

    // Возвращает длительность сигнала в секундах
    virtual float GetSoundTime(void);

    // Возвращает указатель на массив отсчетов в заданном формате
    virtual void * GetPSamplesArray(CSmtSmplesTypes aType);
    // Рушит временные рабочие структуры после использования
    virtual void   ResetSamplesArray(void);

    // --------------------------------
    // Принудительно настраивает параметры FFT
    void SetFFTPower(int32 aPower);

    // --------------------------------
    // --------------------------------
    // Читает отсчеты из файла
    int ReadFromFile(char * aPFileName, long aDelay);

    // Читает инфу из заголовка
    int ReadFileHeader(char * aPFileName);

    // --------------------------------
    // Математика
    // --------------------------------
    // Считает энергию
    float CalculateEnergy(void);
    // Умножает все отсчеты на коэффициент
    void  AplayCoefficient(float aCoeff);

  private:
    // Читаем звуковой файл с WAVE-структурой 
    unsigned char * ReadWaveFile(char * aPSrcFileName, int32 & aNSamples, long aDelay);

    // Читаем заголовок файла с WAVE-структурой 
    bool ReadWaveHeader(char * aPSrcFileName);

    // Читаем просто отсчеты
    unsigned char * ReadPCMFile(char * aPSrcFileName, int32 & aNSamples, long aDelay);
    // Приводим к отсчетам в формате "флоат"
    float * TranslateToNormalFormat(unsigned char * aPFileBytes, 
                                    int32 & aNSamples, int32 & aTrueSize);
};

#endif // __C_Smt_Samples_h__
