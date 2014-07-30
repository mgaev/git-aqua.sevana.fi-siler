#include <stdio.h>
#include <string.h>
#include <math.h>
#include "Vad.h"

// Voice Activity Detector functions
float CVAD :: a[14] = 
{
  1.750000e-03, -4.545455e-03, -2.500000e+01, 2.000000e+01,
  0.000000e+00,  8.800000e+03,  0.000000e+00, 2.5e+01,
 -2.909091e+01,  0.000000e+00,  1.400000e+04, 0.928571,
 -1.500000e+00,  0.714285
};

float CVAD :: b[14] = 
{
  0.00085f,  0.001159091f, -5.0f,  -6.0f,    -4.7f, -12.2f,     0.0009f,
 -7.0f,     -4.8182f,      -5.3f, -15.5f, 1.14285f,  -9.0f, -2.1428571f
};

void CVAD :: init_vad(void)
{
  // Static vectors to zero
  set_zero(MeanLSF, M);
  set_zero(old_speech, L_TOTAL);
    
  // Initialize VAD parameters
  MeanSE = 0.0;
  MeanSLE = 0.0;
  MeanE = 0.0;
  MeanSZC = 0.0;
  count_sil = 0;
  count_update = 0;
  count_ext = 0;
  less_count = 0;
  flag = 1;
  Min = FLT_MAX_G729;
  Prev_Min = Min;
  set_zero(Min_buffer, 16);
}

typedef void TputEnergyFunc(float dEnergy);
void * pPutEnergy = NULL;
void CVAD :: vad(float rc, float *lsf, float *rxx, float *sigpp,
                 int frm_count, int prev_marker, int pprev_marker, int *marker)
{
  float tmp[M];
  float SD;
  float E_low;
  float dtemp;
  float dSE;
  float dSLE;
  float ZC;
  float COEF;
  float COEFZC;
  float COEFSD;
  float dSZC;
  float norm_energy;
  int   i;
    
  // compute the frame energy 
  norm_energy = 10.0f*float(log10(rxx[0]/240.0 +EPSI));
  if (pPutEnergy) ((TputEnergyFunc *)pPutEnergy)(norm_energy);

  // compute the low band energy 
  E_low = 0.0;
  for( i=1; i<= NP; i++) E_low = E_low + rxx[i]*lbf_corr[i];

  E_low= rxx[0]*lbf_corr[0] + 2.0f*E_low;
  if (E_low < 0.0) E_low = 0.0;
  E_low= 10.0f*float(log10(E_low/240.0+EPSI));

  // compute SD 
  // Normalize lsfs
  for(i=0; i<M; i++) lsf[i] /= 2.0f*PI;
  dvsub(lsf,MeanLSF,tmp,M);
  SD = dvdot(tmp,tmp,M);
    
  // compute # zero crossing
  ZC = 0.0f;
  dtemp = sigpp[ZC_START];
  for (i=ZC_START+1 ; i <= ZC_END ; i++) 
    {
      if (dtemp*sigpp[i] < 0.0) ZC= ZC + 1.0f;
      dtemp = sigpp[i];
    }
  ZC = ZC/80.0f;
    
  // Initialize and update Mins 
  if (frm_count < 129) 
    {
      if (norm_energy < Min)
        {
          Min = norm_energy;
          Prev_Min = norm_energy;
        }
      if ((frm_count % 8) == 0)
        {
          Min_buffer[(int)frm_count/8 -1] = Min;
          Min = FLT_MAX_G729;
        }
    }
  if ((frm_count % 8) == 0)
    {
      Prev_Min = Min_buffer[0];
      for (i = 1; i < 15; i++)
        if (Min_buffer[i] < Prev_Min) Prev_Min = Min_buffer[i];
    }
    
  if (frm_count >= 129) 
    {
      if ((frm_count % 8 ) == 1) 
        {
          Min = Prev_Min;
          Next_Min = FLT_MAX_G729;
        }
      if (norm_energy <      Min) Min      = norm_energy;
      if (norm_energy < Next_Min) Next_Min = norm_energy;
      if ((frm_count % 8) == 0)
        {
          for (i = 0; i < 15; i++) Min_buffer[i] = Min_buffer[i+1];
          Min_buffer[15]  = Next_Min;
          Prev_Min = Min_buffer[0];
          for (i = 1; i < 16; i++)
            if (Min_buffer[i] <  Prev_Min) Prev_Min = Min_buffer[i];
        }
    }
    
  if (frm_count <= INIT_FRAME)
    {
      if (norm_energy < v_21)
      //if (norm_energy < dFEnergyBound)
        {
          less_count++;
          *marker = NOISE;
        }
      else
        {
          //if (norm_energy < v_21_2) *marker = NOISE;
          if (norm_energy < dFEnergyBound) *marker = NOISE;
          //if (norm_energy < dFEnergyBound_4) *marker = NOISE;
          else                             *marker = VOICE;
          //
          MeanE = (MeanE*( (float)(frm_count-less_count -1)) + norm_energy)/(float) (frm_count-less_count);
          MeanSZC = (MeanSZC*( (float)(frm_count-less_count -1)) + ZC)/(float) (frm_count-less_count);
          dvwadd(MeanLSF,(float) (frm_count-less_count -1),lsf,1.0,MeanLSF,M);
          dvsmul(MeanLSF,1.0f/(float) (frm_count-less_count ),MeanLSF,M);
        }
    }

  if (frm_count >= INIT_FRAME)
    {
      if (frm_count == INIT_FRAME)
        {
          MeanSE  = MeanE - 10.0f;
          MeanSLE = MeanE - 12.0f;
        }

      dSE  = MeanSE - norm_energy;
      dSLE = MeanSLE - E_low;
      dSZC = MeanSZC - ZC;

      //if (norm_energy < v_21_2) *marker = NOISE;
      if (norm_energy < dFEnergyBound*0.75) *marker = NOISE;
      //if (norm_energy < dFEnergyBound_2) *marker = NOISE;
      else                             *marker = MakeDec(dSLE, dSE, SD, dSZC );
        
      v_flag = 0;
        if ((prev_marker == VOICE)&&(*marker == NOISE)&&(norm_energy > MeanSE + 2.0)&&(norm_energy > v_21))
          {
           *marker = VOICE;
            v_flag =     1;
          }
        
      if ((flag == 1))
        {
          if ((pprev_marker == VOICE)&&(prev_marker == VOICE)&&(*marker == NOISE)&&(fabs(prev_energy - norm_energy) <= 3.0))
            {
              count_ext++;
             *marker = VOICE;
              v_flag =     1;
              if (count_ext <= 4) flag = 1;
              else
                {
                  flag      = 0;
                  count_ext = 0;
                }
            }
        }
      else flag =1;
        
      if(*marker == NOISE) count_sil++;
        
      if ((*marker == VOICE) && (count_sil > 10)&&((norm_energy - prev_energy) <= 3.0))
        {
         *marker = NOISE;
          count_sil=0;
        }
        
        
      if (*marker == VOICE) count_sil=0;
      if ((norm_energy < MeanSE+ 3.0)&&(frm_count > 128)&&(!v_flag)&&(rc <0.6)) *marker = NOISE;
      if ((norm_energy < MeanSE+ 3.0)&&(rc <0.75)&&(SD<0.002532959))
        {
          count_update++;
          if (count_update < INIT_COUNT)
            {
              COEF   = 0.75f;
              COEFZC = 0.8f;
              COEFSD = 0.6f;
            }
          else
            if (count_update < INIT_COUNT+10)
              {
                COEF   = 0.95f;
                COEFZC = 0.92f;
                COEFSD = 0.65f;
              }
            else
              if (count_update < INIT_COUNT+20)
                {
                  COEF   = 0.97f;
                  COEFZC = 0.94f;
                  COEFSD = 0.70f;
                }
              else
                if (count_update < INIT_COUNT+30)
                  {
                    COEF   = 0.99f;
                    COEFZC = 0.96f;
                    COEFSD = 0.75f;
                  }
                else
                  if (count_update < INIT_COUNT+40)
                    {
                      COEF   = 0.995f;
                      COEFZC = 0.99f;
                      COEFSD = 0.75f;
                    }
                  else
                    {
                      COEF   = 0.995f;
                      COEFZC = 0.998f;
                      COEFSD = 0.75f;
                    }
          //
          dvwadd(MeanLSF, COEFSD, lsf, 1.0f - COEFSD, MeanLSF, M);
          MeanSE  = COEF*MeanSE    + (1.0f -   COEF)*norm_energy;
          MeanSLE = COEF*MeanSLE   + (1.0f -   COEF)*E_low;
          MeanSZC = COEFZC*MeanSZC + (1.0f - COEFZC)*ZC;
        }
        
      if ((frm_count > 128)&&((MeanSE < Min )&&(SD < 0.002532959))||(MeanSE > Min+10.0))
        {
          MeanSE = Min;
          count_update = 0;
        }
    }
  
  prev_energy = norm_energy;
}


int CVAD :: MakeDec(float dSLE, float dSE, float SD, float dSZC)
{
  float pars[4];
  float iCount;
    
  pars[0] = dSLE;
  pars[1] = dSE;
  pars[2] = SD;
  pars[3] = dSZC;

  iCount = 0;
    
  // SD vs dSZC
  if (pars[2] > a[0]*pars[3]+b[0]) iCount += 1.0;//return(VOICE);
  if (pars[2] > a[1]*pars[3]+b[1]) iCount += 1.0;//return(VOICE);
        
  // dE vs dSZC
  if (pars[1] < a[2]*pars[3]+b[2]) iCount += 1.0;//return(VOICE);
  if (pars[1] < a[3]*pars[3]+b[3]) iCount += 1.0;//return(VOICE);
  if (pars[1] < b[4])              iCount += 1.0;//return(VOICE);
        
  // dE vs SD
  if (pars[1] < a[5]*pars[2]+b[5]) iCount += 1.0;//return(VOICE);
  if (pars[2] > b[6])              iCount += 1.0;//return(VOICE);
        
  // dEL vs dSZC
  if (pars[1] < a[7]*pars[3]+b[7]) iCount += 1.0;//return(VOICE);
  if (pars[1] < a[8]*pars[3]+b[8]) iCount += 1.0;//return(VOICE);
  if (pars[1] < b[9])              iCount += 1.0;//return(VOICE);
    
  // dEL vs SD 
  if (pars[0] < a[10]*pars[2]+b[10]) iCount += 1.0;//return(VOICE);
    
  // dEL vs dE 
  if (pars[0] > a[11]*pars[1]+b[11]) iCount += 1.0;//return(VOICE);
  if (pars[0] < a[12]*pars[1]+b[12]) iCount += 1.0;//return(VOICE);
  if (pars[0] < a[13]*pars[1]+b[13]) iCount += 1.0;//return(VOICE);

  iCount /= 14.0;
  if (iCount > 0.65) return(VOICE);
    
  return(NOISE);
}

void CVAD :: dvsub(float *in1, float *in2, float *out, short npts)
{
  while (npts--)  *(out++) = *(in1++) - *(in2++);
}

float CVAD :: dvdot(float *in1, float *in2, short npts)
{
  float accum;
    
  accum = 0.0;
  while (npts--)  accum += *(in1++) * *(in2++);
  return(accum);
}

void CVAD :: dvwadd(float *in1, float scalar1, float *in2, float scalar2,
                        float *out, short npts)
{
  while (npts--) *(out++) = *(in1++) * scalar1 + *(in2++) * scalar2;
}

void CVAD :: dvsmul(float *in, float scalar, float *out, short npts)
{
  while (npts--)  *(out++) = *(in++) * scalar;
}

void CVAD :: set_zero(float x[], int L)
{
  //for (int i = 0; i < L; i++) x[i] = 0.0;
  memset(x, 0, sizeof(float)*L);
}

void CVAD :: copy( float  x[], float  y[], int L)
{
  //for (int i = 0; i < L; i++) y[i] = x[i];
  memcpy(y, x, sizeof(float)*L);
}

void CVAD :: Init(void)
{
  init_pre_process();
  init_coder_ld8c();
  init_vad();
  frame = 0;
  dFEnergyBound   = v_21;
  dFEnergyBound_2 = v_21_2;
  dFEnergyBound_4 = v_21_4;
}

void CVAD :: SetEnergyBound(float aBound)
{
  if (aBound < v_21)
    {
      dFEnergyBound   = v_21;
      dFEnergyBound_2 = v_21_2;
      dFEnergyBound_4 = v_21_4;
    }
  else
    {
      dFEnergyBound   =      aBound;
      dFEnergyBound_2 = 1.5f*aBound;
      dFEnergyBound_4 = 4.0f*aBound;
    }
}

int CVAD :: GoOne(short * pInData)
{
  for(int i=0; i < L_FRAME; i++) new_speech[i] = pInData[i];

  if (frame == 32767) frame = 256;
  else frame++;

  pre_process(new_speech, L_FRAME);
  if (coder_ld8c(frame) == 1) return(1);
  else                        return(0);
}

int CVAD :: GoOneBack(short * pInData)
{
  for(int i=0, j=L_FRAME-1; i < L_FRAME; i++, j--) new_speech[i] = pInData[j];

  if (frame == 32767) frame = 256;
  else frame++;

  pre_process(new_speech, L_FRAME);
  if (coder_ld8c(frame) == 1) return(1);
  else                        return(0);
}

float CVAD :: GoPreEnergy(short * pInData)
{
  float sum, iTmpD;
  int    i;

  for(i=0; i < L_WINDOW; i++) pre_energy_buff[i] = pInData[i];
  pre_process(pre_energy_buff, L_WINDOW);

  sum = 0.0;
  for(i = 0; i < L_WINDOW; i++) 
    {
      iTmpD = pre_energy_buff[i]*hamwindow[i];
      sum += (iTmpD*iTmpD);
    }
  if (sum < 1.0) sum = 1.0;
  iTmpD = 10.0f*float(log10(sum/240.0 + EPSI));
  return(iTmpD);
}
