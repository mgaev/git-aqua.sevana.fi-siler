#include <stdio.h>
#include <string.h>
#include <math.h>
#include "Vad.h"

float CVAD :: pre_lsp_old[M] =
{
  0.9595f,  0.8413f,  0.6549f,  0.4154f,  0.1423f,
 -0.1423f, -0.4154f, -0.6549f, -0.8413f, -0.9595f
};

void CVAD :: init_coder_ld8c(void)
{
  new_speech = old_speech + L_TOTAL - L_FRAME;  // New speech
  speech     = new_speech - L_NEXT;             // Present frame
  p_window   = old_speech + L_TOTAL - L_WINDOW; // For LPC window

  // Initialize static pointers
  wsp    = old_wsp + PIT_MAX;

  // Static vectors to zero
  set_zero(old_speech, L_TOTAL);
  set_zero(old_wsp, L_FRAME+PIT_MAX);
  
  // For G.729B
  // Initialize VAD/DTX parameters
  pastVad  = 0;
  ppastVad = 0;

  // for G.729E
  set_zero(old_A_fwd, MP1);
  old_A_fwd[0] = 1.0;
  set_zero(old_rc_fwd, 2);
  copy(pre_lsp_old, lsp_old, M);
}

int CVAD :: coder_ld8c(int frame)
{
  float r_fwd[NP+1];          // Autocorrelations (forward)                   //
  float rc_fwd[M];            // Reflection coefficients : forward analysis   //
  float A_t_fwd[MP1*2];       // A(z) forward unquantized for the 2 subframes //
  float lsp_new[M];           // LSPs at 2th subframe                         //
  float lsf_new[M];
  
  // Other vectors
                               
  // For G.729B
  int Vad;

  autocorr(p_window, NP, r_fwd);                     // Autocorrelations 
  lag_window(NP, r_fwd);                             // Lag windowing    
  levinsone(M, r_fwd, &A_t_fwd[MP1], rc_fwd,         // Levinson Durbin  
  old_A_fwd, old_rc_fwd );
  //---------------<<>>>

  az_lsp(&A_t_fwd[MP1], lsp_new, lsp_old);           // From A(z) to lsp

  for( int i = 0; i < M; i++ )
	  lsf_new[i] = float(acos(lsp_new[i]));

  vad(rc_fwd[1], lsf_new, r_fwd, p_window, frame, pastVad, ppastVad, &Vad);

  ppastVad = pastVad;
  pastVad = Vad;

  copy(&old_speech[L_FRAME], &old_speech[0], L_TOTAL-L_FRAME);
  copy(&old_wsp[L_FRAME], &old_wsp[0], PIT_MAX);
  return(Vad);
}

