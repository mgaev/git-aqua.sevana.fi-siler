#ifndef __Wav_Files_h__
#define __Wav_Files_h__ 1

#ifndef int32
#define int32 int
#endif

// —амый заголовочек
struct TVRIFFHeader
{
           int32  RIFF;
           int32  FileLen; // -8
           int32  WAVE;
};

struct TVFMTHeader
{
           int32 FMT;
           int32 Maker;
  unsigned short ModulationType;
  unsigned short NChannel;
           int32 Diskret;
           int32 Speed;
  unsigned short Bytes;
  unsigned short Value;
};

struct TVSomethingHeader
{
           int32 dDataId;
           int32 dHedSize;
};

// wav header format
struct TWAV_HEADER
{
           int32  RIFF;
           int32  FileLen; // -7
           int32  WAVE;
           int32  FMT;
           int32  Maker;
  unsigned short ModulationType;
  unsigned short NChannel;
           int32  Diskret;
           int32  Speed;
  unsigned short Bytes;
  unsigned short Value;
           int32  Data;
           int32  InfLen;
};

// ‘орматы звука
#define cmPCMVoice  0
#define cmALawVoice 1

//////////////////////////
// ƒл€ 8к√ц и 16к√ц
#define isRIFF                  0x46464952
#define isWAVE                  0x45564157
#define isFMT                   0x20746d66
#define isMaker     	        0x00000010
#define isModulationTypePCM     0x00000001
#define isModulationTypeCODEC   0x00000002
#define isNChannel              0x00000001
#define isBytesPCM              0x00000002
#define isValuePCM              0x00000010
#define isData                  0x61746164
#define isFact                  0x74636166
#define isAUST                  0x00000004

#define cmWaveType              1
#define cmCodecType             0

#endif 
