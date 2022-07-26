
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: localoci.h 15 2000-02-14 20:06:34Z lucky $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2000/02/14 20:05:54  lucky
 *     Initial revision
 *
 *
 */

/*
 *  localoci.h   
 *  
 *  Does all the basic Oracle OCI includes required
 *
 *  Also contains some useful definitions to use with OCI 7.3
 *  (Not everything is nicely defined in the Oracle header files)
 *                                       -Lynn Dietz
 *  
 * NOTE:  When you need to test the VALUE of a return code from an
 *        OCI function, always use the rc field of the Cda_Def structure
 *        instead of the actual returned value. The rc value will  
 *        always be positive (it's an unsigned 2-byte int in OCI 7.3).
 *
 *        The actual return value (which is the same as the
 *        outdated v2_rc field of the Cda_Def structure) may have 
 *        a different sign or value than the rc field and could 
 *        cause one to become very confused.  LDD
 *
 */

#ifndef LOCALOCI_H
#define LOCALOCI_H

/* Basic Oracle OCI includes: 
 ****************************/
#include <oratypes.h>
#include <ocidfn.h>
#ifdef __STDC__       /* if ANSI C compiler */
#include <ociapr.h>
#else
#include <ocikpr.h>
#endif

/* Useful local definitions start here: */

/* Oracle DataType Codes
 * internal/external datatype codes 
 **********************************/
#define VARCHAR2_TYPE         1   /* char[n], n<=2000                  */
#define NUMBER_TYPE           2   /* unsigned char[21]                 */
#define INT_TYPE              3   /* signed char,short,int, or long    */
#define FLOAT_TYPE            4   /* float, double                     */
#define STRING_TYPE           5   /* char[n+1] (null-terminated-string)*/
#define ROWID_TYPE           11   /* char[n]                           */
#define DATE_TYPE            12   /* char[7]                           */
#define LONGVARRAW_TYPE      24   /* unsigned char[n+ilen]             */
#define UNSIGNED_TYPE        68   /* unsigned integers                 */
#define CHAR_TYPE            96   /* fixed-length char[n] n<=255       */

/* Oracle OCI oparse flags 
 *************************/
#define  NATIVE                     1
#define  VERSION_7                  2
#define  OCI_NTS                   -1   /* null-terminated string */
#define  OCI_IGNORED_FIELD         -1
#define  OCI_MORE_FETCH_PIECES   3130
#define  OCI_MORE_INSERT_PIECES  3129
#define  OCI_NO_MORE_DATA        1403   /* got all data */
#define  OCI_NULL_COLUMN         1405   /* fetched a null column */
#define  OCI_TRUNCATED_COLUMN    1406   /* truncated a column */
#define  OCI_DEFER_FULL_PARSE       1
#define  OCI_NODEFER_FULL_PARSE     0
#define  OCI_NATIVE_BEHAVIOR        1  /* expect native behavior of the version you're connected to */
#define  OCI_VERSION_7              2  /* expect behavior of Oracle version7 */

/* Possible "indicator variable" values 
   ("indp" argument of obndrv,obndrn)
 **************************************/
#define  OCI_VALID_DATA             0
#define  OCI_NULL_DATA             -1

/* Arguments for oexfet
 **********************/
#define  CANCEL_CURSOR    1  /* cancel cursor (same as ocan) after fetch   */
#define  KEEP_CURSOR      0  /* keep cursor active, do more fetches w/ofen */  

/* Oracle return codes to check for on ofetch,oexfet,ofen
 ********************************************************/
#define  OCI_SUCCESS           0   /* no errors                            */
#define  OCI_NO_DATA_FOUND  1403   /* no more rows were selected           */
#define  OCI_NULL_COLUMN    1405   /* null item retrieved for a column     */
#define  OCI_TRUNCATED      1406   /* item fetched was truncated           */
#define  OCI_INVALID_CONV   1454   /* invalid conversion requested         */
#define  OCI_REALOVERFLOW   1456   /* item would overflow a floating-point */
                                   /* number on this machine               */
#define  OCI_UNSUPPORTED    3115   /* unsupported datatype                 */

/*  OCI function code labels,
 *  corresponding to the fc numbers
 *  in the cursor data area.
 **********************************/
CONST text  *oci_func_tab[] =  {(text *) "not used",
/* 1-2 */       (text *) "not used", (text *) "OSQL",
/* 3-4 */       (text *) "not used", (text *) "OEXEC, OEXN",
/* 5-6 */       (text *) "not used", (text *) "OBIND",
/* 7-8 */       (text *) "not used", (text *) "ODEFIN",
/* 9-10 */      (text *) "not used", (text *) "ODSRBN",
/* 11-12 */     (text *) "not used", (text *) "OFETCH, OFEN",
/* 13-14 */     (text *) "not used", (text *) "OOPEN",
/* 15-16 */     (text *) "not used", (text *) "OCLOSE",
/* 17-18 */     (text *) "not used", (text *) "not used",
/* 19-20 */     (text *) "not used", (text *) "not used",
/* 21-22 */     (text *) "not used", (text *) "ODSC",
/* 23-24 */     (text *) "not used", (text *) "ONAME",
/* 25-26 */     (text *) "not used", (text *) "OSQL3",
/* 27-28 */     (text *) "not used", (text *) "OBNDRV",
/* 29-30 */     (text *) "not used", (text *) "OBNDRN",
/* 31-32 */     (text *) "not used", (text *) "not used",
/* 33-34 */     (text *) "not used", (text *) "OOPT",
/* 35-36 */     (text *) "not used", (text *) "not used",
/* 37-38 */     (text *) "not used", (text *) "not used",
/* 39-40 */     (text *) "not used", (text *) "not used",
/* 41-42 */     (text *) "not used", (text *) "not used",
/* 43-44 */     (text *) "not used", (text *) "not used",
/* 45-46 */     (text *) "not used", (text *) "not used",
/* 47-48 */     (text *) "not used", (text *) "not used",
/* 49-50 */     (text *) "not used", (text *) "not used",
/* 51-52 */     (text *) "not used", (text *) "OCAN",
/* 53-54 */     (text *) "not used", (text *) "OPARSE",
/* 55-56 */     (text *) "not used", (text *) "OEXFET",
/* 57-58 */     (text *) "not used", (text *) "OFLNG",
/* 59-60 */     (text *) "not used", (text *) "ODESCR",
/* 61-62 */     (text *) "not used", (text *) "OBNDRA"
};

#endif  /* LOCALOCI_H */

