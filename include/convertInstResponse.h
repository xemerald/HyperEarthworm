/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: convertInstResponse.h 6803 2016-09-09 06:06:39Z et $
 *
 *	  convertInstResponse.h
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2006/01/30 19:41:10  friberg
 *     added in hydra includes for raypicker module
 *
 *     Revision 1.1.1.1  2005/06/22 19:30:36  michelle
 *     new directory tree built from files in HYDRA_NEWDIR_2005-06-20 tagged hydra and earthworm projects
 *
 *     Revision 1.5  2004/08/25 17:57:09  cjbryan
 *     revised prototypes to match changes to convertInstResponse.c
 *
 *     Revision 1.4  2004/08/10 22:21:17  michelle
 *     correct function name createAtoDfilter to be createAtoDFilter as this is
 *     the function implementation name and name used in other calling code.
 *     this fixes compile warnings
 *
 *     Revision 1.3  2004/05/26 19:05:05  cjbryan
 *     changed ResponseStruct to ResponseFnStruct
 *
 *     Revision 1.2  2004/04/22 14:52:34  patton
 *     filters.h should be ioc_filters.h, it is now
 *
 *     Revision 1.1  2004/04/21 20:00:15  cjbryan
 *     initial version
 *
 *
 */

#ifndef CONVERTINSTRESPONSE_H
#define CONVERTINSTRESPONSE_H

#include <transferFunction.h>
#include <ioc_filter.h>

int createAtoDFilter(ResponseFnStruct *fromInstResp, FILTER *pFilter, 
                     ResponseFnStruct *toInstResp, ResponseFnStruct *convertedResp,
                     int traceAmplitude);
int convertInstResponse(ResponseFnStruct *fromInstResp, FILTER *rFilter, 
                        ResponseFnStruct *toInstResp, ResponseFnStruct *convertedResp,
                        int traceAmplitude);
int constructTransferFunctionPZ(TransferFnStruct *fromInstResp, FILTER *pFilter, 
                                TransferFnStruct *toInstResp, 
                                TransferFnStruct *convertedResp);
int evaluateTransferFn(TransferFnStruct *transferFn, Complex *transFnValue);
int pz_overflow(Complex *response, double *compValue);
int digitalFilterCoeffs(TransferFnStruct tf, FILTER *rFilter);
int trapFilterZeroes(ResponseFnStruct *convertedResp, FILTER *pFilter);


#endif /*  CONVERTINSTRESPONSE_H  */
