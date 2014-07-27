#ifndef __C_Smt_Samples_h__
#define __C_Smt_Samples_h__

// ��������� ����� ��������
enum CSmtSmplesTypes
{
  esstByte,    // ������� ���� ����
  esstShort,   // ������� ������
  esstInt,     // ����������� ������������� ������
  esstFloat,   // ������� � ��������� ������
};

// ����������� ����� ��� ������ � ���������
class CSamplesVirt
{
  public:
    // ���������� ���������� ��������
    virtual int32 GetNSamples(void) = 0;
    // ���������� ���������� �������
    virtual int32 GetNChannels(void) = 0;
    // ���������� ������� �������������
    virtual int32 GetSampleRate(void) = 0;
    // ���������� ����������� ��������
    virtual int32 GetSampleBits(void) = 0;
    // ���������� ������ �������� ������ FFT (������ �������)
    virtual int32 GetFFTHalfBufferSize(void)=0;
    // ���������� ���������� ������� FFT-�������
    virtual int32 GetFFTPower(void)=0;
    // ���������� ��� �������� � ���-�����
    virtual int32 GetSampleType(void)=0;

    // ���������� ������������ ������� � ��������
    virtual float GetSoundTime(void)=0;

    // ���������� ��������� �� ������ �������� � �������� �������
    virtual void * GetPSamplesArray(CSmtSmplesTypes aType) = 0;
    // ����� ��������� ������� ��������� ����� �������������
    virtual void   ResetSamplesArray(void) = 0;
};

// ���������� ������ ��� ������ � ���������
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

    // ���������� ��� ���������� ������ � ��������� ���������
    void Reset(void);
    // ��������� ������� �������� ������� ������
    bool Check(void);

    // ��������� ����������� �����
    void TrimIt(double aTrmThresh);

    void SetFromShort(short * aPSamples, int32 aNSamples);
    void SetFromFloat(float * aPSamples, int32 aNSamples);

    bool ApplayFromShort(void);

    // ���������� ���������� ��������
    virtual int32 GetNSamples(void);
    // ���������� ���������� �������
    virtual int32 GetNChannels(void);
    // ���������� ������� �������������
    virtual int32 GetSampleRate(void);
    // ���������� ����������� ��������
    virtual int32 GetSampleBits(void);
    // ���������� ������ �������� ������ FFT (������ �������)
    virtual int32 GetFFTHalfBufferSize(void);
    // ���������� ���������� ������� FFT-�������
    virtual int32 GetFFTPower(void);
    // ���������� ��� �������� � ���-�����
    virtual int32 GetSampleType(void);

    // ���������� ������������ ������� � ��������
    virtual float GetSoundTime(void);

    // ���������� ��������� �� ������ �������� � �������� �������
    virtual void * GetPSamplesArray(CSmtSmplesTypes aType);
    // ����� ��������� ������� ��������� ����� �������������
    virtual void   ResetSamplesArray(void);

    // --------------------------------
    // ������������� ����������� ��������� FFT
    void SetFFTPower(int32 aPower);

    // --------------------------------
    // --------------------------------
    // ������ ������� �� �����
    int ReadFromFile(char * aPFileName, long aDelay);

    // ������ ���� �� ���������
    int ReadFileHeader(char * aPFileName);

    // --------------------------------
    // ����������
    // --------------------------------
    // ������� �������
    float CalculateEnergy(void);
    // �������� ��� ������� �� �����������
    void  AplayCoefficient(float aCoeff);

  private:
    // ������ �������� ���� � WAVE-���������� 
    unsigned char * ReadWaveFile(char * aPSrcFileName, int32 & aNSamples, long aDelay);

    // ������ ��������� ����� � WAVE-���������� 
    bool ReadWaveHeader(char * aPSrcFileName);

    // ������ ������ �������
    unsigned char * ReadPCMFile(char * aPSrcFileName, int32 & aNSamples, long aDelay);
    // �������� � �������� � ������� "�����"
    float * TranslateToNormalFormat(unsigned char * aPFileBytes, 
                                    int32 & aNSamples, int32 & aTrueSize);
};

#endif // __C_Smt_Samples_h__
