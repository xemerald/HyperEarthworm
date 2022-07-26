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

/*
 * fft_prep.c: routines for setting up the structures needed for the 
 * Temperton FFT99 package.
 */

#include <stdlib.h>
#include <stdio.h>
#include <fft_prep.h>
#include <fft99.h>
#include <earthworm.h>

static long last_nfft = 1;
static FACT *fft_list = (FACT *)NULL;
static int prime[3] = {2, 3, 5};
static int Debug = 0;

static int makeTrigs( FACT *this );

/*
 * fftPrepDebug: set the debug level for all functions in fft_prep.c
 *        Debug levels: 0 no debug output
 *                      1 dump FAClist on logic errors.
 */
void fftPrepDebug( int level )
{
  Debug = level;
  return;
}


/* buildFacList: build a linked list of FFT factor structures. This list is
 * used to decide the size of FFT to use. The FFT routines can handle 
 * sizes that are multiples of powers of 2, 3, and 5. This provides greater
 * flexibility than sizes that are only powers of 2.
 *
 * argument: new_nfft: the largest fft size required for this list.
 *   This function may be called again to extend the list further.
 * returns: 0 on success
 *         -1 on failure (out of memory; not logged here)
 *         -2 on logic errors (logged here)
 */

long buildFacList( long new_nfft )
{
  long i, j, next_nfft;
  FACT *this, *next, *ins_before;

  /*
   * The idea is to start at `last_nfft' and multiply it by 2, 3, and 5, 
   * creating a new FACT element on the list for each one. After this is
   * done, then we advance on the list ONE element (not the three that we 
   * just created. From this next list element, we set `last_nfft' to the
   * nfft value of the element. In this way, the list contains an ordered
   * set of all the multiples of powers of 2, 3, and 5.
   */

  /* First time through is a special case: there are no elements on the 
   * list, so we have to create one. We'll assume that new_nfft > 5
   * though it really doesn't matter */
  if (last_nfft == 1)
  {
    if ( (fft_list = (FACT *)calloc((size_t) 1, sizeof(FACT))) == (FACT *)NULL)
      return -1;
    this = fft_list;
    this->nfft = prime[0];
    this->fact_power[0] = 1;
    next = this;
    for (i = 1; i < N_RADIX; i++)
    {
      /* Get the next factor structure and fill it in */
      if ( (next->next = (FACT *)calloc((size_t) 1, sizeof(FACT))) == (FACT *)NULL)
        return -1;
      next = next->next;
      next->nfft = last_nfft * prime[i];
      next->fact_power[i]++;
    }
  } 
  else
  {
    /* Walk the list to the last (newest, largest) entry */
    this = fft_list;
    while (this->nfft < last_nfft )
    {
      /* DEBUG; shouldn't happen if logic is correct */
      if (this->next == (FACT *) NULL)
      {
        logit("et", "buildFacList: Walked off end of list this %ld last %ld\n",
              this->nfft, last_nfft);
        if (Debug > 0)
          printFacList();
        return -2;
      }
      this = this->next;
    }
    if (this->nfft != last_nfft)
    {
      logit("et", "buildFacList: last %ld does not match this %ld\n", 
            last_nfft, this->nfft);
      if (Debug > 0)
        printFacList();
      return -2;
    }
  }
  last_nfft = this->nfft;
  
  /* Add new entries to the list up to the requested "new_nfft" */
  while (this->nfft <= new_nfft)
  {
    ins_before = this;
    for (i = 0; i < N_RADIX; i++)
    {
      next_nfft = last_nfft * prime[i];
      /* Walk the list to see where the next element will go, or if it 
       * is a dupe */
      while( ins_before->next != (FACT *)NULL && 
             ins_before->next->nfft < next_nfft)
        ins_before = ins_before->next;
      if (ins_before->next != (FACT *)NULL && 
          ins_before->next->nfft == next_nfft)
      {  /* Found a dupe; skip it */
        continue;
      }
      /* Get the next factor structure and fill it in */
      if ( ( next = (FACT *)calloc((size_t) 1, sizeof(FACT))) == (FACT *)NULL)
        return -1;
      next->nfft = next_nfft;
      for (j = 0; j < N_RADIX; j++)
        next->fact_power[j] = this->fact_power[j];
      next->fact_power[i]++;
      /* Insert in the list */
      next->next = ins_before->next;
      ins_before->next = next;
    }
    /* don't need current factor anymore, move to next one */
    this = this->next;
    last_nfft = this->nfft;
  }
  return 0;
}


/*
 * prepFFT: find a `suitable' FFT size for proposed size n, and return
 * a pointer to the FACT structure that has been prepared for this FFT.
 * Currently `suitable' is defined as the next even nfft value larger than n.
 * In the future, a more intelligent sizing algorithm may be employed.
 *
 * Arguments: n   the proposed FFT size
 *           *pf  pointer to the FACT structure that will be filled in
 *                by prepFFT for this FFT.
 * returns:  size of FFT on success, 
 *           -1 on failure (out of memory; not logged)
 *           -2 on other errors, logged here
 */

long prepFFT( long n, FACT **pf )
{
  FACT *this;
  int rc;
  
  /* Make sure we have enough entries in the list of FACT structures */
  if ( n > last_nfft)
  {
    if ( (rc = buildFacList( n )) < 0 ) 
      return rc;
  }
  
  /* Walk the list, looking for `suitable', even nfft value */
  this = fft_list;
  while( (this->nfft < n || this->nfft % 2 == 1) && 
         this->next != (FACT *)NULL )
    this = this->next;

  if ( this->trigs == (double *)NULL)
  {
    if (makeTrigs( this ) < 0)
      return -1;
  }
  
  *pf = this;
  return this->nfft;
}

  
void printFacList( )
{
  FACT *this;

  this = fft_list;
  
  while( this != (FACT *)NULL)
  {
    logit("", "%6ld %2ld %2ld %2ld\n", this->nfft, this->fact_power[0], 
           this->fact_power[1], this->fact_power[2]);
    this = this->next;
  }
  return;
}

/* trimFacList: remove unwanted entries from the beginning of the FFT list. */
void trimFacList( long n )
{
  FACT *this;
  
  if (n >= last_nfft)
    return;
  
  this = fft_list;
  
  while(this->nfft < n)
  {
    fft_list = this->next;
    free( this );
    this = fft_list;
  }
  return;
}

/*
 * makeTrigs: Set up the trigs and ifax arrays for fft99. 
 *    Returns: 0 on success
 *            -1 when out of memory
 */
static int makeTrigs( FACT *this )
{
  long ntrigs;
  
  ntrigs = 3 * this->nfft / 2 + 1;
  if ( (this->trigs = (double *)calloc((size_t) ntrigs, sizeof(double))) ==
       (double *)NULL)
    return -1;
  if ( (this->ifax = (long *)calloc((size_t) 13, sizeof(long))) == (long *)NULL)
  {
    free(this->trigs);
    this->trigs = (double *)NULL;
    return -1;
  }
  fftfax(this->nfft, this->ifax, this->trigs);

  return 0;
} 
