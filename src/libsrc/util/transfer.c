/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id:
 *
 *    Revision history:
 *     $Log:
 *
 *
 *
 */

/* transfer.c: Routines for dealing with instrument transfer functions */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fft_prep.h>
#include <fft99.h>
#include <transfer.h>

#define MAXLINE 1024
#define NANOMETERS_PER_METER 1.0e09
static int Debug = 0;

static int InputMetersInsteadOfNanometers = 0;	/* if you cannot figure this one out from the name, stop reading code */

/* Internal Function Prototypes */
static void drop(PZNum *, int *, int);

/*
 * response: compute frequency response from the pole-zero-gain information.
 *  arguments:  nfft: the number of points that will be used in the FFT
 *            deltat: the time interval between data points in the time-domain
 *               pRS: pointer to the Response Structure holding the poles,
 *                    zeros and gain information for the desired function
 *               tfr: pointer to the real part of the frequency response
 *               tfi: pointer to the imaginary part of the frequency 
 *                    response. Both tfr and tfi must be allocated
 *                    by the caller to contain at least nfft/2+1 values.
 */
void response(long nfft, double deltat, ResponseStruct *pRS, 
              double *tfr, double *tfi)
{
  double delomg, omega, mag2;
  double sr, si, srn, sin, srd, sid, sr0, si0;
  long i, j, ntr;
  
  ntr = nfft / 2 + 1;
  delomg = 2.0 * PI / (nfft * deltat);
  
  /* The (almost) zero frequency term */
  /* The zeros, in the numerator */
  srn = 1.0;
  sin = 0.0;
  omega = delomg * 0.001;
  for (j = 0; j < pRS->iNumZeros; j++)
  {
    sr = - pRS->Zeros[j].dReal;
    si = omega - pRS->Zeros[j].dImag;
    sr0 = srn * sr - sin * si;
    si0 = srn * si + sin * sr;
    srn = sr0;
    sin = si0;
  }
  
  /* The poles; in the denominator */
  srd = 1.0;
  sid = 0.0;
  
  for (j = 0; j < pRS->iNumPoles; j++)
  {
    sr = - pRS->Poles[j].dReal;
    si = omega - pRS->Poles[j].dImag;
    sr0 = srd * sr - sid * si;
    si0 = srd * si + sid * sr;
    srd = sr0;
    sid = si0;
  }
  
  /* Combine numerator, denominator and gain using complex arithemetic */
  mag2 = pRS->dGain / (srd * srd + sid * sid);
  tfr[0] = mag2 * (srn * srd + sin * sid);
  tfi[0] = 0.0; /* Actually the Nyqust part; we don't want it */
  
  /* The non-zero frequency parts */
  for (i = 1; i < ntr; i++)
  {
    /* The zeros, in the numerator */
    srn = 1.0;
    sin = 0.0;
    omega = delomg * i;
    for (j = 0; j < pRS->iNumZeros; j++)
    {
      sr = - pRS->Zeros[j].dReal;
      si = omega - pRS->Zeros[j].dImag;
      sr0 = srn * sr - sin * si;
      si0 = srn * si + sin * sr;
      srn = sr0;
      sin = si0;
    }
    
    /* The poles; in the denominator */
    srd = 1.0;
    sid = 0.0;
    
    for (j = 0; j < pRS->iNumPoles; j++)
    {
      sr = - pRS->Poles[j].dReal;
      si = omega - pRS->Poles[j].dImag;
      sr0 = srd * sr - sid * si;
      si0 = srd * si + sid * sr;
      srd = sr0;
      sid = si0;
    }
    
    /* Combine numerator, denominator and gain using complex arithemetic */
    mag2 = pRS->dGain / (srd * srd + sid * sid);
    tfr[i] = mag2 * (srn * srd + sin * sid);
    tfi[i] = mag2 * (sin * srd - srn * sid);
  }
  return;
}


/*
 * readPZ: read a SAC-format pole-zero file.
 * Arguments: pzfile: the name of the pole-zero file to read
 *               pRS: pointer to the response structure to be filled in
 *                    The calling program must allocate the ResponseStruct;
 *                    the individual pole and zero structures will be 
 *                    allocated here.
 *            
 *            Pole-zero-gain files must be for input displacement in 
 *            nanometers, output in digital counts, poles and zeros of
 *            the LaPlace transform, frequency in radians per second.
 * returns: 0 on success
 *         -1 on out-of-memory error
 *         -2 if unable to read or parse the file
 *         -3 for invalid arguments
 *         -4 error opeing file
 */
int readPZ( char *pzfile, ResponseStruct *pRS )
{
  FILE *pzFILE;
  int retval = 0, status = 0;
  int i, nz = 0, np = 0;
  char line[MAXLINE], word[21];
  enum states {Key, Pole, Zero};
  enum states state = Key;
  int alreadySet = 0;
  
  //fprintf(stdout, "** MTH: Enter readPZ(): pzfile=[%s]\n", pzfile);

  if ( pzfile == (char *)NULL || strlen(pzfile) == 0)
  {
    /* empty or missing file name */
    return -3;
  }
  
  if ( (pzFILE = fopen(pzfile, "r")) == (FILE *)NULL)
  {
    /* Error opening file */
    return -4;
  }

  while ( fgets( line, MAXLINE, pzFILE) != (char *)NULL)
  {
    /* if (line[strlen(line)-1] == '\n')
       line[strlen(line)-1] = '\0';*/
    
   restart:	/* needed for when there are no zeros after a ZERO statement (which is allowed in SAC format) */
    switch (state)
    {
    case Key:  /* Looking for next keyword */
      if (sscanf(line, "%20s", word) == 0)
        continue;
      if ( word[0] == '*' || word[0] == '#') continue;  /* a comment line */
      
      if ( strcmp(word, "CONSTANT") == 0)
      {
        if (sscanf(line, "%*s %lf", &pRS->dGain) == 0 )
        { 
          retval = -2;
          goto abort;
        }
        status |= 1;  /* Found the constant or gain */

    // 2016-12-20 MTH: For each blank line after "CONSTANT" in the polezero file, the CONSTANT block
    //                 is getting hit. Without the alreadySet flag we would be scaling the gain to nm multiple times!
    //                 Since the IRIS FetchData/FetchMetaData codes include 2 blank lines after CONSTANT,
    //                 this is a problem. This routine should be cleaned up (eg, remove the goto's!)
        if (InputMetersInsteadOfNanometers && !(alreadySet) ) 
        {
          fprintf(stdout, "** MTH: transfer.c readPZ(): pzfile=[%s] orig dGain=[%e]\n", pzfile, pRS->dGain);
          pRS->dGain = pRS->dGain/NANOMETERS_PER_METER;
          fprintf(stdout, "** MTH: transfer.c readPZ(): pzfile=[%s]  new dGain=[%e]\n", pzfile, pRS->dGain);
          alreadySet = 1;
        }
      }
      else if ( strcmp(word, "ZEROS") == 0)
      {
        if (sscanf(line, "%*s %d", &pRS->iNumZeros) == 0 || pRS->iNumZeros < 0)
        {
          /* invalid or missing number after ZEROS keyword */
          retval = -2;
          goto abort;
        }
        if (pRS->iNumZeros > 0)
        {
          if ( (pRS->Zeros = (PZNum *)malloc(pRS->iNumZeros * sizeof(PZNum))) 
               == (PZNum *)0 )
          {
            retval = -1;
            goto abort;
          }
          for (i = 0; i < pRS->iNumZeros; i++)
          {
            pRS->Zeros[i].dReal = 0.0;
            pRS->Zeros[i].dImag = 0.0;
          }
                    
          state = Zero;  /* There MAY BE  some zeros; go find them */
          continue;
        }
        status |= 2;  /* Got the number of zeros: none */
      }
      else if ( strcmp(word, "POLES") == 0)
      {
        if (sscanf(line, "%*s %d", &pRS->iNumPoles) == 0 || pRS->iNumPoles < 0)
        {
          /* invalid or missing number after POLES keyword */
          retval = -2;
          goto abort;
        }
        if (pRS->iNumPoles > 0)
        {
          if ( (pRS->Poles = (PZNum *)malloc(pRS->iNumPoles * sizeof(PZNum))) 
               == (PZNum *)0 )
          {
            retval = -1;
            goto abort;
          }
          for (i = 0; i < pRS->iNumPoles; i++)
          {
            pRS->Poles[i].dReal = 0.0;
            pRS->Poles[i].dImag = 0.0;
          }
          
          state = Pole;  /* There are some poles; go find them */
          continue;
        }
        status |= 4;  /* Got the number of poles: none */
      }
      else
      {
        /* Invalid keyword */
        retval = -2;
        goto abort;
      }
      break;
    case Zero:
      /* Looking for Zeros */
      if (nz >= pRS->iNumZeros)
      {
        /* Too many zeros! */
        retval = -2;
        goto abort;
      }
      if (sscanf(line, "%lf %lf", 
                 &pRS->Zeros[nz].dReal, &pRS->Zeros[nz].dImag) != 2)
      {
        /* Couldn't read a line of zeros */
	/* see if it was a keyword CONSTANT or POLES indicating all ZEROS were indeed zero! */
        if (sscanf(line, "%20s", word) >= 1) {
		if (strcmp(word, "CONSTANT") == 0 || strcmp(word, "POLES") == 0) {
		      nz = pRS->iNumZeros;
		      state = Key;
                      status |= 2;
		      goto restart;	 /* need to parse the key word without getting next line */
		}
        }
        retval = -2;
        goto abort;
      }
      if (++nz == pRS->iNumZeros)
      {
        /* Found all the zeros we expected */
        status |= 2;
        state = Key;
        continue;
      }
      break;
    case Pole:
      /* Looking for poles */
      if (np >= pRS->iNumPoles)
      {
        /* Too many poles! */
        retval = -2;
        goto abort;
      }
      if (sscanf(line, "%lf %lf", 
                 &pRS->Poles[np].dReal, &pRS->Poles[np].dImag) != 2)
      {
        /* Couldn't read a line of poles */
	/* see if it was a keyword CONSTANT or ZEROS key word indicating all remaining POLES  were indeed zero! */
        if (sscanf(line, "%20s", word) >= 1) {
		if (strcmp(word, "CONSTANT") == 0 || strcmp(word, "ZEROS") == 0) {
		      np = pRS->iNumPoles;
		      state = Key;
                      status |= 4;
		      goto restart;	 /* need to parse the key word without getting next line */
		}
        }
        retval = -2;
        goto abort;
      }
      if (++np == pRS->iNumPoles)
      {
        /* Found all the poles we expected */
        status |= 4;
        state = Key;
        continue;
      }
      break;
    }
  }

  if (status != 7)
  {
    /* One of the keywords was missing */
    retval = -2;
  }
  
 abort:
  if (retval != 0)
  {
    /* Something went wrong; clean up the mess */
    cleanPZ(pRS);
  }
  fclose(pzFILE);
  
  return retval;
}


/*
 * ftaper: produce a cosine taper between unity (beyond fon) and zero
 *        (beyond foff). The cosine taper is between fon and foff.
 * Arguments: freq: the frequency at which the taper value is desired
 *             fon: the unity end of the taper
 *            foff: the zero end of the taper
 *    if fon and foff are equal, then taper returns 1.0, the all-pass filter.
 * returns: the value of the taper
 */
double ftaper(double freq, double fon, double foff)
{
  double t, pi = PI;
  
  if (fon > foff)
  {   /* high-pass taper */
    if (freq < foff)
      t = 0.0;
    else if (freq > fon)
      t = 1.0;
    else
      t = 0.5 * (1.0 - cos(pi * (freq - foff) / (fon - foff)));
  }
  else if (fon < foff)
  {   /* low-pass case */
    if (freq < fon)
      t = 1.0;
    else if (freq > foff)
      t = 0.0;
    else
      t = 0.5 * (1.0 + cos(pi * (freq - fon) / (foff - fon)));
  }
  else
    t = 1.0;
  
  return t;
}

/*
 * convertWave: converts a waveform (time series) from its original response
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
 *          finalRS: structure defining desired response function
 *             freq: array of four frequencies (f0, f1, f2, f3) defining the
 *                   taper to be applied to the frequency response function
 *                   before it is convolved with the data. Below f0 and above
 *                   f3, the taper is 0; between f2 and f3 the taper is 1;
 *                   between f0-f1 and f2-f3 is a cosine taper.
 *            retFD: flag to return result in frequency-domain (if retFD == 1)
 *                   or in time-domain (if retFD == 0)
 *                   If the output is to stay in the frequency domain,
 *                   be sure you understand how the results are laid out. 
 *                   See the comments in the FFT package: currently sing.c
 *           padlen: The pad length to be applied to data before transforming
 *                   to frequency domain. If padlen < 0, pad length will be
 *                   estimated here and the value chosen will be returned
 *                   in this return-value parameter.
 *             nfft: The size of the FFT chosen, based on npts + *padlen
 *                   If the returned value of nfft + padlen is less than
 *                   npts, then convertWave had to effectively truncate the 
 *                   raw trace in order to fit the processed trace in
 *                   the limit of outBufLen.
 *           output: array of values output from the conversion
 *                   This array must be allocated by the caller. 
 *        outBufLen: size of `output' array.
 *             work: a work array that must be allocated by the caller.
 *                   Its size must be outBufLen+2
 *          workFFT: a work array needed by fft99. 
 *                   Its size must be outBufLen+1
 *
 * Returns: 0 on success
 *         -1 on out-of-memory errors                   
 *         -2 on too-small impulse response
 *         -3 on invalid arguments
 *         -4 on FFT error
 */

int convertWave(double input[], long npts, double deltat, 
                ResponseStruct *origRS, ResponseStruct *finalRS, 
                double freq[4], int retFD, long *pPadLen, long *pnfft, 
                double output[], long outBufLen, double *work, double *workFFT)
{
  ResponseStruct rs;  /* the combined response function */
  FACT *pfact;
  long i, ii, retval = 0;
  int nz = 0, np = 0;
  long nfft, nfreq, trial_nfft;
  double delfreq, tpr, dre, dim, f, delomg, omega;
  double *fre, *fim;
  
  /* Validate arguments */
  if (origRS == (ResponseStruct *)NULL || finalRS == (ResponseStruct *)NULL ||
      npts < 2 || deltat <= 0.0 || freq == (double *)NULL || outBufLen < npts)
  {
    return -3;
  }
  if (freq[0] > freq[1] || freq[1] >= freq[2] || freq[2] > freq[3] ||
      origRS->dGain == 0.0 || finalRS->dGain == 0.0)
  {
    return -3;
  }
  

  /* Combine the response functions into one: finalRS / origRS */
  rs.dGain = finalRS->dGain / origRS->dGain;
  rs.iNumPoles = finalRS->iNumPoles + origRS->iNumZeros;
  rs.iNumZeros = finalRS->iNumZeros + origRS->iNumPoles;
  if ( (rs.Poles = (PZNum *)malloc(sizeof(PZNum) * rs.iNumPoles)) == 
       (PZNum *)NULL ||
       (rs.Zeros = (PZNum *)malloc(sizeof(PZNum) *rs.iNumZeros)) ==
       (PZNum *)NULL)
  {
    retval = -1;
    goto exit;
  }
  /* Copy the poles and zeros, using structure copy */
  for (i = 0; i < origRS->iNumPoles; i++)
    rs.Zeros[nz++] = origRS->Poles[i];
  for (i = 0; i < origRS->iNumZeros; i++)
    rs.Poles[np++] = origRS->Zeros[i];
  for (i = 0; i < finalRS->iNumPoles; i++)
    rs.Poles[np++] = finalRS->Poles[i];
  for (i = 0; i < finalRS->iNumZeros; i++)
    rs.Zeros[nz++] = finalRS->Zeros[i];
  
  if (Debug & TR_DBG_PZG)
  {
    printf("Input response function: gain %10.3e\n", origRS->dGain);
    printf("Poles: %d\n", origRS->iNumPoles);
    for (i = 0; i < origRS->iNumPoles; i++)
      printf("%10.3e   %10.3e\n", origRS->Poles[i].dReal, 
             origRS->Poles[i].dImag);
    printf("\nZeros: %d\n", origRS->iNumZeros);
    for (i = 0; i < origRS->iNumZeros; i++)
      printf("%10.3e   %10.3e\n", origRS->Zeros[i].dReal, 
             origRS->Zeros[i].dImag);
    printf("\nOutput response function: gain %10.3e\n", finalRS->dGain);
    printf("Poles: %d\n", finalRS->iNumPoles);
    for (i = 0; i < finalRS->iNumPoles; i++)
      printf("%10.3e   %10.3e\n", finalRS->Poles[i].dReal, 
             finalRS->Poles[i].dImag);
    printf("\nZeros: %d\n", finalRS->iNumZeros);
    for (i = 0; i < finalRS->iNumZeros; i++)
      printf("%10.3e   %10.3e\n", finalRS->Zeros[i].dReal, 
             finalRS->Zeros[i].dImag);
  }

  /* Determine how much padding we need, unless the caller told us. */
  if ( *pPadLen < 0)
  {
    if ( (*pPadLen = respLen( &rs, deltat, freq)) < 0)
    {
      /* Some error occured */
      retval = *pPadLen;
      if (Debug)
        printf("\nrespLen error: %ld\n", *pPadLen);
      goto exit;
    }
    if (Debug)
      printf("estimated pad length: %ld\n", *pPadLen);
  }
  
  /* Find a convenient FFT size for our data plus padding that will fit in
   *  outBuf */
  trial_nfft = *pPadLen + npts;
  while ( (nfft = prepFFT(trial_nfft, &pfact)) > outBufLen)
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

  nfreq = nfft / 2 + 1;
  fre = work;
  fim = work + 1 + outBufLen/2;

  for (i = 0; i < npts; i++)
    output[i] = input[i];
  /* Fill the remainder of output buffer with zeros.                 *
   * For fft99, we must fill two slots past the normal end of output *
   * buffer; this space must be allocated in initBufs().             */
  for (i = outBufLen + FFT_EXTRA; i < nfft; i++)
    output[i] = 0.0;
  
  /* Transform the padded data into frequency domain */
  fft991(output, workFFT, pfact->trigs, pfact->ifax, (long) 1, nfft, nfft, (long) 1, (long) -1);

  response(nfft, deltat, &rs, fre, fim);
  
  /* Convolve the tapered frequency response with the data. Since we  *
   * are in the frequency domain, convolution becomes `multiply'.     *
   * We skip the zero-frequency part; this only affects the mean      *
   * of the data, which should have been removed long ago.            */
  delfreq = 1.0 / (nfft * deltat);
  output[0] = 0.0;   /* Remove the mean, if there is any */
  for (i = 1; i < nfreq - 1; i++)
  {
    ii = i+i;
    f = i * delfreq;
    tpr = ftaper(f, freq[1], freq[0]) * ftaper(f, freq[2], freq[3]);
    dre = output[ii];   /* Real part of transformed data */
    dim = output[ii+1]; /* Imaginary part of transformed data */
    output[ii] = (dre * fre[i] - dim * fim[i]) * tpr;
    output[ii+1] = (dre * fim[i] + dim * fre[i]) * tpr;
  }
  f = i * delfreq;
  tpr = ftaper(f, freq[1], freq[0]) * ftaper(f, freq[2], freq[3]);
  dre = output[nfft];  /* Real part of transformed data; imaginary part is 0 */
  output[nfft] = dre * fre[i] * tpr;

  if (Debug & TR_DBG_ARS)
  {
    delomg = 2.0 * PI / (nfft * deltat);
    printf("    omega        tapered\n");
    printf("%4ld  %10.3e  %10.3e  %10.3e\n", 0L, 0.0, 0.0, 0.0);
    for (i = 1; i < nfreq - 1; i++ )
    {
      omega = i * delomg;
      printf("%4ld  %10.3e  %10.3e  %10.3e\n", i, omega, fre[i], fim[i]);
    }
  }
  
  /* If the user wants data in the time domain, transform it now */
  if (!retFD)
    fft991(output, workFFT, pfact->trigs, pfact->ifax, (long) 1, nfft, nfft, (long) 1, (long) +1);

  /* We're done! Give the data back to the caller */
  *pnfft = nfft;

  exit:
    /* Clean up before we quit: free any memory that was allocated */
  cleanPZ( &rs );
  return retval;
}

/* NFFT_TEST is the size of the iFFT used for estimated the pad length. *
 * The value of NFFT_TEST MUST be a multiple of 2, 3, and/or 5.         */
#define NFFT_TEST 1024
/*
 * ALIAS_CUTOFF is the fraction of the maximum of the impulse response
 * function used for testing pad length. The length (in points) of the
 * impulse response that is above this threshold is the pad length
 */
#define ALIAS_CUTOFF 0.01

/*
 * respLen: estimate the length of the impulse response (the number of
 *          points where it is greater than some throshold) so we know
 *          how much padding we need for the convolution.      
 *          This is a trial algorithm that may or may not work.
 *          We assume the the impulse response looks something like a
 *          broadened and shifted impulse. We asssume that the width of
 *          its peak is independent of the number of points used in
 *          this trial FFT, as long as the peak isn't too broad.  
 *  Returns: the length of the peak (in data points) on success
 *           NFFT_TEST when impulse response never drops below threshold
 *          -1 when out of memory
 *          -2 when impulse response is too small to analyze
 *          -4 on FFT failure
 *          -NFFT_TEST when search tp left of peak finds drop-off
 *           but search to right doesn't find drop-off: logic error.
 *    Exits if NFFT_TEST is not a multiple of powers of 2, 3, 5
 *          That would be a coding error only.
 */
int respLen( ResponseStruct *rs, double deltat, double freq[4])
{
  FACT *pfact;
  double fre[NFFT_TEST/2+1], fim[NFFT_TEST/2+1], data[NFFT_TEST + 2];
  double work[NFFT_TEST+1];
  long i, ii, nf_test, nfreq;
  double imp_max, thresh, delfreq, f, tpr, delomg, omega;
  long imax, left_lim, right_lim;
  
  /*
   * Determine how much padding we need.
   * We do this by computing the frequency response for a small number of
   * points, getting the impulse response (iFFT of freq. response),
   * and measuring the length the non-zero part of the response.
   */
  if ( (nf_test = prepFFT( (long) NFFT_TEST, &pfact)) < 0)
  {
    /* Out of memory */
    return -1;
  }
  else if (nf_test != NFFT_TEST)
  {
    fprintf(stderr, "respLen fatal error: NFFT_TEST (%d) not factorable by 2, 3, 5\n",
            NFFT_TEST);
    return -1;
  }
  nfreq = nf_test/2+1;
  
  response(nf_test, deltat, rs, fre, fim);
  
  delfreq = 1.0 / (nf_test * deltat);

  if (Debug & TR_DBG_TRS)
  {   /* Print the values while they are available */
    delomg = 2.0 * PI / (nf_test * deltat);
    printf("\nTest response function in Frequency Domain\n");
    printf("  i      omega             raw                   tapered\n");
    f = 0.0;
    tpr = ftaper(f, freq[1], freq[0]);
    data[0] = fre[0] * tpr;
    data[1] = 0.0;
    omega = 0.001 * delomg;
    printf("%4ld  %10.3e  %10.3e  %10.3e  %10.3e  %10.3e\n", 0L, omega, fre[0],
           0.0, data[0], 0.0);
    for (i = 1; i < nfreq - 1; i++ )
    {
      ii = i+i;
      f = i * delfreq;
      tpr = ftaper(f, freq[1], freq[0]) * ftaper(f, freq[2], freq[3]);
      data[ii] = fre[i] * tpr;
      data[ii+1] = fim[i] * tpr;
      omega = i * delomg;
      printf("%4ld  %10.3e  %10.3e  %10.3e  %10.3e  %10.3e\n", i, omega, 
             fre[i], fim[i], data[ii], data[ii+1]);
    }
    data[nf_test] = 0.0;
    data[nf_test+1] = 0.0;
  }
  else
  {  /* Don't print, just calculate quickly with no /if/ inside the loops */
    f = 0.0;
    tpr = ftaper(f, freq[1], freq[0]);
    data[0] = fre[0] * tpr;
    data[1] = 0.0;
    for (i = 1; i < nfreq - 1; i++ )
    {
      ii = i+i;
      f = i * delfreq;
      tpr = ftaper(f, freq[1], freq[0]) * ftaper(f, freq[2], freq[3]);
      data[ii] = fre[i] * tpr;
      data[ii+1] = fim[i] * tpr;
    }
    data[nf_test] = 0.0;
    data[nf_test+1] = 0.0;
  }
  
  /* Transform test response function into time domain */
  fft991(data, work, pfact->trigs, pfact->ifax, (long) 1, nf_test, nf_test, (long) 1, (long) +1);

  
  if (Debug & TR_DBG_TRS)
  {
    printf("\nTest response function in TD\n");
    for (i = 0; i < nf_test; i++)
    {
      printf("%5ld  %10.3e\n", i, data[i]);
    }
  }
 
  /*
   * We assume that the impulse response has a peak somewhere and 
   * falls off on both sides of that peak, allowing for wrap-around.
   * So we locate the peak and set a threshold of ALIAS_CUTOFF (currently 1%)
   * of the peak. If the response does not drop off below this threshold
   * for any values of i, then we're screwed. Some other measure will have
   * to be dreamed for preventing time-domain aliasing in our convolution.
   */
  imp_max = 0.0;
  for (i = 0; i < nf_test; i++)
  {
    if (fabs(data[i]) > imp_max)
    {
      imp_max = fabs(data[i]);  /* the extremal value */
      imax = i;                 /* the location of the extremum */
    }
  }
  
  if (imp_max > 0.001 * rs->dGain)
  {   /* make sure the response isn't too small to measure */
    thresh = imp_max * ALIAS_CUTOFF;
    /* Start searching to the left from the peak to find where we drop   *
     * below the threshold. This point will be left_lim; we may have to  *
     * wrap the search around to the right end. If this earch continues  *
     * all the way back to the peak, then the peak is too broad and this *
     * algorithm fails.                                                  */
    right_lim = left_lim = imax;
    for (i = imax; i >= 0; i--)
    {
      if ( fabs(data[i]) < thresh)
      {
        left_lim = i;
        break;
      }
    }
    if (left_lim == imax) /* Didn't find it; wrap around */
    {
      for (i = nf_test - 1; i > imax; i--)
      {
        if ( fabs(data[i]) < thresh)
        {
          left_lim = i;
          break;
        }
      }
      if (left_lim == imax)
      {   /* Still didn't find it; peak doesn't drop of anywhere */
        return NFFT_TEST;
      }
    }
    /* Now search to the right for right_lim; may have to wrap around */
    for (i = imax; i < nf_test; i++)
    {
      if ( fabs(data[i]) < thresh)
      {
        right_lim = i;
        break;
      }
    }
    if (right_lim == imax)
    {  /* Didn't find it; wrap around to the left side */
      for (i = 0; i < imax; i++)
      {
        if ( fabs(data[i]) < thresh)
        {
          right_lim = i;
          break;
        }
      }
      if (right_lim == imax)
      {   /* How come we found left_lim but not right_lim: shouldn't happen! */
        return -NFFT_TEST;
      }
    }
    if (left_lim < right_lim)
      return (right_lim - left_lim + 1);
    else
      return (right_lim - left_lim + nf_test + 1);
  }
  /* Impulse response is too small to analyze */
  return -2;
}

/*
 * pzCancel: Remove cancelling pole-zero pairs from a response structure.
 *           Search for pairs of poles and zeros whose real and imaginary
 *           parts are within `tol' of each other. Remove any such pairs.
 *           This will remove useless computations from the calculation
 *           of the frequency response function in response().
 */
void pzCancel(ResponseStruct *rs, double tol)
{
  int ip, iz, mz;
  
  if (rs->iNumZeros == 0 || rs->iNumPoles == 0)
    return;
  
  for (ip = 0; ip < rs->iNumPoles; ip++)
  {
    mz = -1;
    for (iz = 0; iz < rs->iNumZeros; iz++)
    {
      if (fabs(rs->Poles[ip].dReal - rs->Zeros[iz].dReal) < tol &&
          fabs(rs->Poles[ip].dImag - rs->Zeros[iz].dImag) < tol)
      {
        mz = iz;
        break;
      }
    }
    if (mz != -1)
    {
      /* Found a match; remove the pole and the zero; move the other *
       * poles and zeros into those empty slots.                     */
      drop(rs->Poles, &rs->iNumPoles, ip);
      drop(rs->Zeros, &rs->iNumZeros, mz);
      ip--;  /* We have to look at this pole slot again */
      iz--;  /* and also this zero slot */
    }
  }
  return;
}

/*
 * drop: remove the `ipz' pole or zero from the PZNum array;
 *       update the pNumPZ counter.
 */
static void drop(PZNum *pPZ, int *pNumPZ, int ipz)
{
  int i;
  
  (*pNumPZ)--;
  for (i = ipz; i < *pNumPZ; i++)
  {
    pPZ[i].dReal = pPZ[i+1].dReal;
    pPZ[i].dImag = pPZ[i+1].dImag;
  }
  return;
}


/* 
 * taper: Apply a cosine taper to a data series.
 * Arguments:  data: array of data to be tapered
 *             npts: number of points in data array
 *             tLen: width of taper (at each end) in number of points
 *      The end of the taper (where it has 0 value) is assumed to occur 
 *      at the first point before the start and after the end of the data;
 *      these data points are not modified here. There are `tLen' points
 *      that have taper values between 0 and 1 exclusive at each end of
 *      the data.
 *      If tLen is less then half of npts, this function returns
 *      silently without applying the taper.
 */
void taper(double *data, long npts, long tLen)
{
  long i, jb, je;
  double tap, omega;
  
  tLen++;
  if (tLen < 2 || tLen > npts / 2)
    return;
  
  omega = PI / tLen;
  
  for (i = 1; i < tLen; i++)
  {
    jb = i - 1;
    je = npts - i;
    tap = 0.5 * (1.0 - cos(omega * i));
    data[jb] *= tap;
    data[je] *= tap;
  }
  return;
}


void deMean( double *data, long npts, double *mean)
{
  long i;
  double sum = 0.0;
  
  if (npts < 1)
  {
    *mean = 0.0;
    return;
  }
  
  for (i = 0; i < npts; i++)
    sum += data[i];
  
  sum /= (double)npts;
  for (i = 0; i < npts; i++)
    data[i] -= sum;
  
  *mean = sum;
  return;
}

  
void cleanPZ( ResponseStruct *pRS)
{
  if (pRS->Zeros != (PZNum *)NULL)
  {
    free( pRS->Zeros );
    pRS->Zeros = (PZNum *)NULL;
  }
  if (pRS->Poles != (PZNum *)NULL)
  {
    free( pRS->Poles );
    pRS->Poles = (PZNum *)NULL;
  }
  pRS->iNumPoles = 0;
  pRS->iNumZeros = 0;
  return;
}

void transferDebug( int level )
{
  Debug = level;
  return;
}

void setResponseInMeters( int flag ) 
{
  InputMetersInsteadOfNanometers = flag;
  return;
}
