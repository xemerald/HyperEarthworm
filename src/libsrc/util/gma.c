/* gma.c: Routines for the ground motion analyzer: compute PGA, PGV, PGD *
 *        and spectral response, using spectral techniques               */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "earthworm.h"
#include "fft_prep.h"
#include "fft99.h"
#include "transfer.h"
#include "gma.h"

#define MAXLINE 80

static int Debug = 0;

/* Internal Function Prototypes */
static void makeInt(long nfft, double deltat, double *fIntI);
static int makePsa(double, double, ResponseStruct *);

void cmpmax(long kug, double *ug, double w, double damp, double delta, 
            double *zd, double *zv, double *za);

/*
 * gma: converts a waveform (time series) from its original response
 *              function to a new response function. This conversion is done
 *              in the frequency domain. The frequency response of the 
 *              transfer function may be tapered. The input data will be
 *              padded in the time-domain. The amount of padding is determined
 *              automatically unless the user provides her own pad length.
 * Arguments: input: array of data for preocessing
 *             npts: number of data points to process
 *           deltat: time interval between samples, in seconds
 *           origRS: structure defining process that generated the input data
 *                   that is, the response function to be removed
 *           fTaper: array of four frequencies (f0, f1, f2, f3) defining the
 *                   taper to be applied to the frequency response function
 *                   before it is convolved with the data. Below f0 and above
 *                   f3, the taper is 0; between f2 and f3 the taper is 1;
 *                   between f0-f1 and f2-f3 is a cosine taper.
 *           tTaper: length of cosine taper in seconds to be applied at
 *                   both ends of the input data to provide a smooth
 *                   transition inot the zero-padded area.
 *           padlen: The pad length to be applied to data before transforming
 *                   to frequency domain. If padlen < 0, pad length will be
 *                   estimated here and the value chosen will be returned
 *                   in this return-value parameter.
 *             nfft: The size of the FFT chosen, based on npts + *padlen
 *                   If the returned value of nfft + padlen is less than
 *                   npts, then convertWave had to effectively truncate the 
 *                   raw trace in order to fit the processed trace in
 *                   the limit of outBufLen.
 *         spectPer: array of periods at which the Spectral Response is
 *                   to be calculated
 *              nsp; number of periods in spectPer
 *        spectDamp: array of damping values at which the Spectral Response
 *                   is to be calculated
 *              nsd; number of damping values in spectDamp
 *           output: array of the six sets of values output from the analysis.
 *                   This array must be allocated by the caller, of length
 *                   outBufLen * (3 + nsd * nsf).
 *        outBufLen: size of one of the `output' arrays.
 *             work: a work array that must be allocated by the caller.
 *                   It's size must be outBufLen * 3.
 *          workFFT: a work array for the FFT routines. It's size must be
 *                   outBufLen * (2 + nsf * nsd).
 *
 * Returns: 0 on success
 *         -1 on out-of-memory errors                   
 *         -2 on too-small impulse response
 *         -3 on invalid arguments
 */

int gma(double input[], long npts, double deltat, ResponseStruct *origRS, 
        double fTaper[4], double tTaper, long *pPadLen, long *pnfft, 
        double spectPer[], int nsp, double spectDamp[], int nsd, 
        double output[], long outBufLen, double *work, double *workFFT)
{
  static ResponseStruct accRS, psaRS;
  FACT *pfact;
  double *acc, *vel, *disp, *psa;
  double *fAccR, *fAccI, *fIntI;
  double *fPsaR, *fPsaI;
  double dre, dim;   /* temporary values */
  double delfreq, f, tpr, accr, acci, minDamp, maxPeriod;
  long flen, i, ii, retval = 0;
  long nfft, nfreq, trial_nfft, psaPadLen, ntaper;
  int nz, np, isd, isp, isdp;
  
  /* Some handy pointers for all our arrays */
  acc = output;
  vel = &output[outBufLen];
  disp = &output[outBufLen * 2];
  flen = outBufLen/2;
  fAccR = work;           
  fAccI = &work[flen];    
/*fIntR = &work[flen * 2];*//* We never use this; it is treated as zero */
  fIntI = &work[flen * 3];
  fPsaR = &work[flen * 4];
  fPsaI = &work[flen * 5];
  
  /* Validate arguments */
  if (origRS == (ResponseStruct *)NULL || npts < 2 || deltat <= 0.0 || 
      fTaper == (double *)NULL || outBufLen < npts)
  {
    return -3;
  }
  if (fTaper[0] > fTaper[1] || fTaper[1] >= fTaper[2] || 
      fTaper[2] > fTaper[3] || origRS->dGain == 0.0 )
  {
    return -3;
  }
  
  /* Adjust the input response function to get ground acceleration *
   * Input units: nanometers of displacement                       *
   * Output units: acceleration in cm/sec/sec                      */

  accRS.dGain = 1.0 / (1.0e+7 * origRS->dGain);

  accRS.iNumPoles = origRS->iNumZeros; 
  accRS.iNumZeros = origRS->iNumPoles + 2; /* Differentiate to acceleration */

  if ( (accRS.Poles = (PZNum *)malloc(sizeof(PZNum) * accRS.iNumPoles)) == 
       (PZNum *)NULL ||
       (accRS.Zeros = (PZNum *)malloc(sizeof(PZNum) *accRS.iNumZeros)) ==
       (PZNum *)NULL)
  {
    retval = -1;
    goto exit;
  }


  /* Copy the poles and zeros, using structure copy */
  np = nz = 0;

  for (i = 0; i < origRS->iNumPoles; i++)
  {
    accRS.Zeros[nz].dReal = origRS->Poles[i].dReal;
    accRS.Zeros[nz].dImag = origRS->Poles[i].dImag;
    nz = nz + 1;
  }

  for (i = 0; i < origRS->iNumZeros; i++)
  {
    accRS.Poles[np].dReal = origRS->Zeros[i].dReal;
    accRS.Poles[np].dImag = origRS->Zeros[i].dImag;
    np = np + 1;
  }

  for (i = 0; i < 2; i++)
  {
    accRS.Zeros[nz].dReal = 0.0;
    accRS.Zeros[nz].dImag = 0.0;
    nz++;
  }
  
  /* Remove any cancelling poles and zeros */
  pzCancel(&accRS, 1.0e-5);
  
  /* Determine how much padding we need, unless the caller told us. */
  if ( *pPadLen < 0)
  {
    if ( (*pPadLen = respLen( &accRS, deltat, fTaper)) < 0)
    {
      /* Some error occured */
      retval = *pPadLen;
      if (Debug)
        logit ("", "\nrespLen error: %ld\n", *pPadLen);
      goto exit;
    }

    /*
     * Find the length in samples for the lightly damped PSA oscillator
     * to decay to 1% of it's maximum: a measure of the length of its
     * convolution filter. Use the longest period for this.
     */
    /* Find the minumum damping and maximum period, to get the longest decay */
    minDamp = spectDamp[0];
    for (isd = 1; isd < nsd; isd++)
      if (minDamp > spectDamp[isd]) minDamp = spectDamp[isd];
    maxPeriod = spectPer[0];
    for (isp = 1; isp < nsp; isp++)
      if (maxPeriod < spectPer[isp]) maxPeriod = spectPer[isp];
    
    psaPadLen = (long)( -1.0 * maxPeriod * log(0.01) / 
                        (minDamp * 2.0 * PI * deltat));
    if (*pPadLen < psaPadLen)
      *pPadLen = psaPadLen;
    if (Debug)
      logit ("", "estimated pad length: %ld\n", *pPadLen);
  }

  /* Find a convenient FFT size for our data plus padding that will fit in
   *  outBuf */
  trial_nfft = *pPadLen + npts;
  while ( (nfft = prepFFT(trial_nfft, &pfact)) > outBufLen - FFT_EXTRA)
  {
    if (nfft < 0)
    {
      /* Out of memory */
      retval = nfft;
      goto exit;
    }
    trial_nfft -= 100;  /* Try a liitle bit smaller */
  }

  if (nfft - *pPadLen < npts)
    npts = nfft - *pPadLen;   /* Chop some off the end if it won't fit */
  /* We aren't passing this new value of npts back to our caller; she'll
   * have to figure it out from the values of pPadLen and nfft passed back */

  nfreq = nfft / 2;

  /* Copy the input data into the acc array in preparation for FFT */
  for (i = 0; i < npts; i++)
    acc[i] = input[i];
  for (i = npts; i < nfft + FFT_EXTRA; i++)  /* Do the null-padding */
    acc[i] = 0.0;


  /* Taper the data into the zero-padded area */
  ntaper = (long)(tTaper / deltat);
  taper(acc, npts, ntaper);

  /* Transform the padded data into frequency domain */
  fft991(acc, workFFT, pfact->trigs, pfact->ifax, (long) 1, nfft, nfft, (long) 1, (long) -1);

  /* Fill in all the transfer functions */
  response(nfft, deltat, &accRS, fAccR, fAccI);
  makeInt(nfft, deltat, fIntI);

  /* Compute acceleration, velocity and displacement output data in the 
     frequency domain */
  /* rnfft = 1.0 / (2*nfft); */

  acc[0] = 0.0;   /* Remove the mean, if there is any */
  vel[0] = 0.0;
  disp[0] = 0.0;
  acc[1] = 0.0;
  vel[1] = 0.0;
  disp[1] = 0.0;
  
  delfreq = 1.0 / (nfft * deltat);
  for (i = 1; i < nfreq; i++)
  {
    ii = i+i;
    f = i * delfreq;
    tpr = ftaper(f, fTaper[1], fTaper[0]) * ftaper(f, fTaper[2], fTaper[3]);
    dre = acc[ii];   /* Real part of transformed data */
    dim = acc[ii+1]; /* Imaginary part of transformed data */
    /* The acceleration */
    /* WAS: scaled by 1 * rnfft */
    accr = (dre * fAccR[i] - dim * fAccI[i]) * tpr;
    acci = (dre * fAccI[i] + dim * fAccR[i]) * tpr;
    acc[ii] = accr;
    acc[ii+1] = acci;
    /* Integrate once to get velocity */
    vel[ii] = fIntI[i] * acci;
    vel[ii+1] = -fIntI[i] * accr;
    /* Integrate again to get displacement */
    disp[ii] = -fIntI[i] * vel[ii+1];
    disp[ii+1] = fIntI[i] * vel[ii];
  }
  /* Remove the Nyquist frequency component */
  acc[nfft] = 0.0;
  vel[nfft] = 0.0;
  disp[nfft] = 0.0;

  /* Compute the Spectral Responses for all their periods and dampings */
  isdp = -1;
  for (isp = 0; isp < nsp; isp++)
  {
    for (isd = 0; isd < nsd; isd++)
    {
      isdp++;
      psa = &output[outBufLen * (3 + isdp)];
      if ( (retval = makePsa(spectPer[isp], spectDamp[isd], &psaRS)) < 0)
        goto exit;
      response(nfft, deltat, &psaRS, fPsaR, fPsaI);
      
      psa[0] = 0.0;
      psa[1] = 0.0;

      for (i = 1; i < nfreq; i++)
      {
        ii = i+i;
        accr = acc[ii];
        acci = acc[ii+1];
        /* The spectral response accelerations */
        psa[ii] = accr * fPsaR[i] - acci * fPsaI[i] + accr;
        psa[ii+1] = accr * fPsaI[i] + acci * fPsaR[i] + acci;
      }
      /* Remove the Nyquist frequency component */
      psa[nfft] = 0.0;
      psa[nfft+1] = 0.0;

      cleanPZ( &psaRS );
    }
  }

  /* Transform the whole output array back to time domain */
  fft991(output, workFFT, pfact->trigs, pfact->ifax, (long) 1, outBufLen, nfft, 
        (long) (3 + isd * isp), (long) +1);

  /* We're done! Give the data back to the caller */
  *pnfft = nfft;
  
 exit:
  /* Clean up before we go home */
  cleanPZ( &accRS );
  cleanPZ( &psaRS );

  return retval;
}

/*
 * makeInt: make a `response' function for integrating once with respect
 *          to time. This function is simply `i' times the frequency;
 *          or the response function for a single zero at the origin.
 */
static void makeInt(long nfft, double deltat, double *fIntI)
{
  double delomg, omega;
  long i, ntr;
  
  ntr = nfft / 2;
  delomg = 2.0 * PI / (nfft * deltat);

  fIntI[0] = 0.0;
  for (i = 1; i < ntr; i++)
  {
    omega = delomg * i;
    fIntI[i] = 1.0 / omega;
  }
  return;
}

/*
 * makePsa: Generate the acceleration response function in pole-zero-gain
 *          form for a simple oscillator with given free period and damping.
 */
static int makePsa(double period, double damp, ResponseStruct *rs)
{
  double mu, omega;
  
  omega = 2.0 * PI / period;
  mu = sqrt(1.0 - damp * damp);

  rs->iNumZeros = 2;
  rs->iNumPoles = 2;
  if ( (rs->Zeros = (PZNum *)malloc(2 * sizeof(PZNum))) == (PZNum *)0 ||
       (rs->Poles = (PZNum *)malloc(2 * sizeof(PZNum))) == (PZNum *)0 )
    return -1;
  
  rs->Zeros[0].dReal = rs->Zeros[0].dImag = 0.0;
  rs->Zeros[1].dReal = rs->Zeros[1].dImag = 0.0;
  
  rs->dGain = -1.0;

  rs->Poles[0].dReal = - omega * damp;
  rs->Poles[0].dImag = omega * mu;
  rs->Poles[1].dReal = - omega * damp;
  rs->Poles[1].dImag = - omega * mu;
  
  return 0;
}
  
void gmaDebug( int level )
{
  Debug = level;
  return;
}
