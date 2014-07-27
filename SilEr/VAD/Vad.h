#ifndef __CVad_h__
#define __CVad_h__ 1

#define NP            12                  /* Increased LPC order */
#define NOISE         0
#define VOICE         1
//#define INIT_FRAME    32
//#define INIT_COUNT    20
#define INIT_FRAME    16
#define INIT_COUNT    10
#define ZC_START      120
#define ZC_END        200

#define MIN_ENER      0.1588489319   /* <=> - 8 dB      */
#define EPSI          1.0e-38        /* very small positive floating point number      */

#define PI              3.14159265358979323846f
#define PI2             6.283185307
#define FLT_MAX_G729    1.e38   /* largest floating point number             */

#define L_TOTAL         240     /* Total size of speech buffer               */
#define L_FRAME         80      /* LPC update frame size                     */
#define L_SUBFR         40      /* Sub-frame size                            */

#define L_WINDOW        240     /* LPC analysis window size                  */
#define L_NEXT          40      /* Samples of next frame needed for LPC ana. */

#define M               10      /* LPC order                                 */
#define MP1            (M+1)    /* LPC order+1                               */
#define GRID_POINTS     60      /* resolution of lsp search                  */
#define NC              (M/2)   /* LPC order / 2                            */

#define PIT_MAX         143     /* Maximum pitch lag in samples              */

#define MIN_ACTIVE_LENGTH 3
#define MAX_ACTIVE_LENGTH 200     // 2 сек
#define MAX_SPEECH_LENGTH 160000  // Максимальная длина всего рапознаваемого куска (20 сек)

#define N_WINDOW          10      // количество окошек
#define OVERLAP_WINDOW     0.5    // перекрытие окошек

#define BORDER_LENGTH1       16
#define BORDER_LENGTH0       12
#define MIN_ENERGY       100000

#define K_Coeff             1.5   // коэфф для увеличения числа кепстральных коэффициентов

#define v_21      21
#define v_21_2    30
#define v_21_4    84

class CVAD
{
  private:
    static float b140[3];
    static float a140[3];
    static float lbf_corr[NP+1];
    static float hamwindow[L_WINDOW];
    static float grid[GRID_POINTS+1];
    static float lwindow[M+2];
    static float a[14];
    static float b[14];
    static float pre_lsp_old[M];
    //
  public:
    float lsp_old[M];
    float pre_energy_buff[L_WINDOW];
    float dFEnergyBound;
    float dFEnergyBound_2;
    float dFEnergyBound_4;

  public:
    void init_vad(void);
    void init_coder_ld8c(void);
    void init_pre_process(void);
    void Init(void);
    void SetEnergyBound(float aBound);

    int GoOne(short * pInData);
    float GoPreEnergy(short * pInData);

    void vad(float rc, float *lsf, float *rxx, float *sigpp, int frm_count,
             int  prev_marker, int pprev_marker, int *marker);

    float levinsone(int m, float *r, float *A, float *rc, float *old_A, float *old_rc);

    int  coder_ld8c(int frame);
    void coder_ld8k(int *);
    void pre_process(float signal[], int lg);

    void autocorr( float *x, int m, float *r);
    void autocorr_var(short *x, int m, float *r, short l_var_window);
    void lag_window( int m, float r[]);
    float levinson (float *r, float *a, float *r_c);
    void az_lsp( float a[], float lsp[], float old_lsp[]);
    void qua_lsp( float lsp[], float lsp_q[], int ana[]);
    void lsf_lsp( float *lsf, float *lsp, int m);

    void set_zero( float  x[], int l );
    void copy( float  x[], float  y[], int L);

  private:
    int    MakeDec(float dSLE, float dSE, float SD, float dSZC);
    void   dvsub(float *in1, float *in2, float *out, short npts);
    float  dvdot(float *in1, float *in2, short npts);
    void   dvwadd(float *in1, float scalar1, float *in2, float scalar2, float *out, short npts);
    void   dvsmul(float *in, float scalar, float *out, short npts);
    float  chebyshev(float x, float *f, int n);

    float MeanLSF[M];
    float Min_buffer[16];
    float Prev_Min, Next_Min, Min;
    float MeanE, MeanSE, MeanSLE, MeanSZC;
    float prev_energy;
    int    count_sil, count_update, count_ext;
    int    flag, v_flag, less_count;
    float x0, x1;
    float y1, y2;

    float old_speech[L_TOTAL];
    float *speech, *p_window;

    float old_wsp[L_FRAME+PIT_MAX];
    float *wsp;

    int pastVad, ppastVad;

    float old_A_fwd[MP1];
    float old_rc_fwd[2];
    int    frame;
    float *new_speech;
};

#endif
