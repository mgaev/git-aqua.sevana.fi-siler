#include <stdio.h>
#include <string.h>
#include <math.h>
#include "Vad.h"

void CVAD :: autocorr(float *x, int m, float *r)
{
  float y[L_WINDOW];  
  float sum;
  int i, j;
  
  for (i = 0; i < L_WINDOW; i++) y[i] = x[i]*hamwindow[i];
  
  for (i = 0; i <= m; i++)
    {
      sum = 0.0;
      for (j = 0; j < L_WINDOW-i; j++) sum += y[j]*y[j+i];
      r[i] = sum;
    }
  if (r[0]<1.0) r[0]=1.0;
}

void CVAD :: lag_window(int m, float r[])
{
  for (int i=1; i<= m; i++) r[i] *= lwindow[i-1];
}

float CVAD :: levinsone(int m, float *r, float *a, float *rc, float *old_A, float *old_rc)
{
  float s, at, err;
  int i, j, l;
  
  if(r[1] > r[0]) r[1] = r[0];
  rc[0] = (-r[1])/r[0];
  a[0] = 1.0;
  a[1] = rc[0];
  err = r[0] + r[1]*rc[0];
  for (i = 2; i <= m; i++)
    {
      s = 0.0;
      for (j = 0; j < i; j++) s += r[i-j]*a[j];
      if (err != 0.0) rc[i-1] = (-s)/(err);
      else            rc[i-1] = 1.0;
      
      // Test for unstable filter. If unstable keep old A(z)
      if (fabs(rc[i-1]) > 0.999451) 
        {
          for(j=0; j<=m; j++) a[j] = old_A[j];
          rc[0] = old_rc[0];
          rc[1] = old_rc[1];
          return (0.001f);
        }
      
      // -----------------------
      //  Compute new LPC coeff 
      // -----------------------
      for (j = 1; j <= (i/2); j++)
        {
          l = i-j;
          at = a[j] + rc[i-1]*a[l];
          a[l] += rc[i-1]*a[j];
          a[j] = at;
        }
      a[i] = rc[i-1];
      err += rc[i-1]*s;
      if (err <= 0.0) err = 0.001f;
    }
  
  copy(a, old_A, (m+1));
  old_rc[0] = rc[0];
  old_rc[1] = rc[1];
  
  return (err);
}

void CVAD :: az_lsp(float *a, float *lsp, float *old_lsp)
{
  int i, j, nf, ip;
  float xlow,ylow,xhigh,yhigh,xmid,ymid,xint;
  float *coef;
  float f1[NC+1], f2[NC+1];
    
  f1[0] = 1.0;
  f2[0] = 1.0;
  for (i=1, j=M; i<=NC; i++, j--)
    {
      f1[i] = a[i]+a[j]-f1[i-1];
      f2[i] = a[i]-a[j]+f2[i-1];
    }
    
  nf   =  0; // number of found frequencies
  ip   =  0; // flag to first polynomial
  coef = f1; // start with F1(z)
    
  xlow=grid[0];
  ylow = chebyshev(xlow, coef, NC);
    
  j = 0;
  while ((nf < M)&&(j < GRID_POINTS))
    {
      j++;
      xhigh = xlow;
      yhigh = ylow;
      xlow  = grid[j];
      ylow  = chebyshev(xlow,coef,NC);
        
      // if sign change new root exists
      if (ylow*yhigh <= 0.0) 
        {
          j--;
           
          // divide the interval of sign change by 4
          for (i = 0; i < 4; i++)
            {
              xmid = 0.5f*(xlow + xhigh);
              ymid = chebyshev(xmid,coef,NC);
              if (ylow*ymid <= 0.0)
                {
                  yhigh = ymid;
                  xhigh = xmid;
                }
              else
                {
                  ylow = ymid;
                  xlow = xmid;
                }
            }
            
          xint    = xlow - ylow*(xhigh-xlow)/(yhigh-ylow);
          lsp[nf] = xint; // new root
          nf++;
            
          ip   = 1 - ip;        // flag to other polynomial
          coef = ip ? f2 : f1;  // pointer to other polynomial
            
          xlow = xint;
          ylow = chebyshev(xlow,coef,NC);
        }
    }
    
  if (nf < M)
    for(i=0; i<M; i++)  lsp[i] = old_lsp[i];
}

float CVAD :: chebyshev(float x, float *f, int n)
{
  float        b1, b2, b0, x2;
  register int i;                       // for the special case of 10th order //
                                        //       filter (n=5)                 //
  x2 = 2.0f*x;                           // x2 = 2.0*x;                        //
  b2 = 1.0;                // f[0] //   //                                    //
  b1 = x2 + f[1];                       // b1 = x2 + f[1];                    //
  for (i=2; i<n; i++)                   //                                    //
    {                                   //                                    //
      b0 = x2*b1 - b2 + f[i];           // b0 = x2 * b1 - 1. + f[2];          //
      b2 = b1;                          // b2 = x2 * b0 - b1 + f[3];          //
      b1 = b0;                          // b1 = x2 * b2 - b0 + f[4];          //
    }                                   //                                    //
  return (x*b1 - b2 + 0.5f*f[n]);        // return (x*b1 - b2 + 0.5*f[5]);     //
}
