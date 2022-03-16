/*

   parse_usnsn.c


     Routines used to parse a text message from NEIC.
    
     A successful call to ParseNSNMsg fills the NSNStruct
     with values from the text message contained in NsnMsg.

     If debug is not 0, debug info will be written. If debug > 1,
     a file with the parsed values of the message will be 
     written in the debug_dir directory.

     ParseNSNMsg returns EW_SUCCESS if everything goes well, 
     otherwise it returns EW_FAILURE.  

     Only ParseNSNMsg() should be called from outside of this
     file. All other functions in here are utilities used
     by ParseNSNMsg()

*/
      
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "earthworm.h"
#include "parse_usnsn.h"
#include "rw_mag.h"

#define     MAX_LINES       1000
#define     MAX_LINE_SIZE   300

extern	int		epochsec17 (double *, char *);

static	int		ConvertNSNMsg (char *, int, char **, int *);
static	int		ParsePhaseMagnitude (char *, PhaseMag *);
static	int		ParsePhase (char *, Phase *, char *, int *, double *);
static	int		ParseMagnitude (char *, OriginMag *);
static	int		ParseEllipse (char *, double, Err_Elipse *);
static	int		AppendDateStr (char	*, char *);
static	int		BuildDateStr (char *, char *);
static	int 	GetNextLine (char **, int *, char *);

/************************************************************************
 * ParseNSNMsg() - Fill the msg struct from the text supplied in 
 *                 nsnbuf.
 ************************************************************************/
int 	ParseNSNMsg (char *NsnMsg, int msgLen, 
			NSNStruct *msgStruct, int debug, char *debug_dir)
{
	int		i, j;
	char	datestr[15];
	char	DateStr[20];
	char	tmp1[256];
	char	tmp2[256];
	char	tmp3[256];
	int		lineno;
	FILE	*fp;
	char	filename[100];
    char    **nsnbuf;
    int     nlines;


	if ((NsnMsg == NULL) || (msgStruct == NULL) || (debug_dir == NULL))
	{
		logit ("e", "Invalid parameters passed in.\n");
		return EW_FAILURE;
	}	

	nsnbuf = (char **) malloc (MAX_LINES * sizeof (char *));
	for (i = 0; i < MAX_LINES; i++)
		nsnbuf[i] = (char *) malloc (MAX_LINE_SIZE * sizeof (char));


	/* Create nsnbuf (array of lines) for easier parsing
	 ****************************************************/
	if (ConvertNSNMsg (NsnMsg, msgLen, nsnbuf, &nlines) != EW_SUCCESS)
	{
		logit ("", "Call to ConvertNSNMsg failed\n");
		return EW_FAILURE;
	}


	/* skip the first two lines */
	lineno = 2;


	/* First, we have to see if this is a deletion message.
	   We don't do anything with it, so if we get it, we'll
	   just return FAILURE, and let the calling program deal 
	   with the ramifications.
	 ********************************************************/
	sscanf (nsnbuf[lineno + 3], "%s\n", tmp1);
	if (strcmp (tmp1, "EVENT") == 0)
	{
		/* Yup, it's a deletion message */

		/* extract the NSN event key */
		strncpy (msgStruct->EventKey, (nsnbuf[lineno + 3] + 21), 4);
		msgStruct->EventKey[4] = '\0';
		
		return RETURN_DELETE;
	}

	/* see if this is an automatic, or a reviewed solution */
	sscanf (nsnbuf[lineno], "%s\n", tmp1);
	if (strcmp (tmp1, "The") == 0)
		/* Yup, it's an automatic: skip four more lines */
		lineno = lineno + 4;
	/* Else, it's a reviewed solution message: we are at the first line */

/*
 * line 1; extract date, comment, event key, whether it is preliminary
 *********************************************************************/
	strncpy (datestr, (nsnbuf[lineno] + DATE_BASE), 11);
	datestr[11] = '\0';

	if (BuildDateStr (datestr, msgStruct->EventDate) != EW_SUCCESS)
	{
		logit ("e", "Call to BuildDateStr failed.\n");
		return EW_FAILURE;
	}

	strncpy (msgStruct->EventKey, (nsnbuf[lineno] + EVENT_KEY_BASE), 4);
	msgStruct->EventKey[4] = '\0';

	if (nsnbuf[lineno][PRELIM_BASE] == 'p')
	{
		msgStruct->automatic = 1;
	}
	else
	{
		msgStruct->automatic = 0;
	}

	
/*
 * line 2; extract origin time and error
 *********************************************************************/
	lineno = lineno + 1;
	if (GetNextLine (nsnbuf, &lineno, "ot") == EW_SUCCESS)
	{
		sscanf (nsnbuf[lineno], "    ot  = %s   +/-   %s\n", tmp1, tmp2);

		strcpy (DateStr, msgStruct->EventDate);
		if (AppendDateStr (DateStr, tmp1) != EW_SUCCESS)
		{
			logit ("e", "Call to AppendDateStr failed.\n");
			return EW_FAILURE;
		}

		epochsec17 (&msgStruct->ot, DateStr);
		msgStruct->ot_err = (double) atof (tmp2);
	}
	else
	{
		logit ("e", "GetNextLine returned EW_FAILURE; exitting\n");
		return EW_FAILURE;
	}

/*
 * line 3; extract latitude 
 *********************************************************************/
	lineno = lineno + 1;
	if (GetNextLine (nsnbuf, &lineno, "lat") == EW_SUCCESS)
	{
		sscanf (nsnbuf[lineno], "    lat = %s   +/-   %s\n", tmp1, tmp2);

		msgStruct->lat = (float) atof (tmp1);
		msgStruct->lat_err = (float) atof (tmp2);
	}
	else
	{
		logit ("e", "GetNextLine returned EW_FAILURE; exitting\n");
		return EW_FAILURE;
	}

/*
 * line 4; extract longitude 
 *********************************************************************/
	lineno = lineno + 1;
	if (GetNextLine (nsnbuf, &lineno, "lon") == EW_SUCCESS)
	{
		sscanf (nsnbuf[lineno], "    lon = %s   +/-   %s\n", tmp1, tmp2);

		msgStruct->lon = (float) atof (tmp1);
		msgStruct->lon_err = (float) atof (tmp2);
	}
	else
	{
		logit ("e", "GetNextLine returned EW_FAILURE; exitting\n");
		return EW_FAILURE;
	}


/*
 * line 5; extract depth 
 *********************************************************************/
	lineno = lineno + 1;
	if (GetNextLine (nsnbuf, &lineno, "dep") == EW_SUCCESS)
	{
		/* Depth could have an error */
		if (nsnbuf[lineno][DEPTH_BASE] == '+')
		{
			sscanf (nsnbuf[lineno], "    dep = %s   +/-   %s\n", tmp1, tmp2);
			msgStruct->depth = (float) atof (tmp1);
			msgStruct->depth_err = (float) atof (tmp2);

		}
		else
		{
			/* no +/- error, we have some string there */
			sscanf (nsnbuf[lineno], "    dep = %s\n", tmp1);
			msgStruct->depth = (float) atof (tmp1);
			msgStruct->depth_err = (double) 0.0;
			msgStruct->depth_fixed = 1;
		}
	}
	else
	{
		logit ("e", "GetNextLine returned EW_FAILURE; exitting\n");
		return EW_FAILURE;
	}

/*
 * line 6; extract number of phases and std error
 *********************************************************************/
	lineno = lineno + 1;
	if (GetNextLine (nsnbuf, &lineno, "nph") == EW_SUCCESS)
	{
		sscanf (nsnbuf[lineno], "    nph = %s of %s    se = %s  \n", tmp1, tmp2, tmp3);
		msgStruct->nph_used = atoi (tmp1);
		msgStruct->nph_assoc = atoi (tmp2);
		msgStruct->std_error = (double) atof (tmp3);
	}
	else
	{
		logit ("e", "GetNextLine returned P3DB_FAILURE; exitting\n");
		return EW_FAILURE;
	}


/*
 * line 7; extract the error elipse
 *********************************************************************/
	lineno = lineno + 1;
	if (GetNextLine (nsnbuf, &lineno, "error") == EW_SUCCESS)
	{
		strncpy (tmp1, (nsnbuf[lineno] + ELLIPSE_BASE), (size_t) ELLIPSE_LENGTH);
		tmp1[ELLIPSE_LENGTH] ='\0';

		if (ParseEllipse (tmp1, msgStruct->depth_err, &msgStruct->error) != EW_SUCCESS)
		{
			logit ("e", "Call to ParseEllipse failed.\n");
			return EW_FAILURE;
		}
	}
	else
	{
		logit ("e", "GetNextLine returned EW_FAILURE; exitting\n");
		return EW_FAILURE;
	}

/*
 * line 8; extract magnitudes
 *********************************************************************/
	lineno = lineno + 1;
	if (GetNextLine (nsnbuf, &lineno, "mb") == EW_SUCCESS)
	{
		msgStruct->numMags = 5;
		if (ParseMagnitude (nsnbuf[lineno], msgStruct->O_mag) != EW_SUCCESS)
		{
			logit ("e", "Call to ParseMagnitude failed.\n");
			return EW_FAILURE;
		}
	}
	else
	{
		logit ("e", "GetNextLine returned EW_FAILURE; exitting\n");
		return EW_FAILURE;
	}

/*
 * line 9; find the beginning of the picks section
 *********************************************************************/
	lineno = lineno + 1;
	if (GetNextLine (nsnbuf, &lineno, "sta") == EW_SUCCESS)
	{

		/* Now, read and parse all the phases */
		msgStruct->nph_actual = 0;
		msgStruct->Dmin = EARTH_CIRCUM;
		for (i = 0; i < msgStruct->nph_assoc; i++)
		{
			lineno = lineno + 1;
			if (ParsePhase (nsnbuf[lineno], &msgStruct->phases[msgStruct->nph_actual],
							msgStruct->EventDate, &msgStruct->nph_actual, 
							&msgStruct->Dmin) != EW_SUCCESS)
			{
				logit ("e", "Call to ParsePhase failed.\n");
				return EW_FAILURE;
			}
		}


		if (debug > 10)
		{
			/* Print the parsed version of the message
			 ********************************************/
			sprintf (filename, "%s/%s-%s", 
						debug_dir, msgStruct->EventDate, msgStruct->EventKey);
			
			fp = fopen (filename, "wt");


			/*** This is where we are done ****/
			fprintf (fp, "\n===> PRINTING SUMMARY FOR EVENT %s \n", msgStruct->EventKey);
			fprintf (fp, "Event Date: %s\n", msgStruct->EventDate);
			fprintf (fp, 
						"ot=%0.2f,%0.2f; lat=%0.2f,%0.2f; lon=%0.2f,%0.2f; dep=%0.2f,%0.2f\n", 
							msgStruct->ot, msgStruct->ot_err,
							msgStruct->lat, msgStruct->lat_err,
							msgStruct->lon, msgStruct->lon_err,
							msgStruct->depth, msgStruct->depth_err);
			fprintf (fp, "Magnitudes computed: \n");
			fprintf (fp, "\t %c (%d) = %0.2f from %d phases\n", 
										msgStruct->O_mag[0].magLabel,
										msgStruct->O_mag[0].MagType,
										msgStruct->O_mag[0].magAvg, 
										msgStruct->O_mag[0].numStas);
			fprintf (fp, "\t %c (%d) = %0.2f from %d phases\n", 
										msgStruct->O_mag[1].magLabel,
										msgStruct->O_mag[1].MagType,
										msgStruct->O_mag[1].magAvg, 
										msgStruct->O_mag[1].numStas);
			fprintf (fp, "\t %c (%d) = %0.2f from %d phases\n", 
										msgStruct->O_mag[2].magLabel,
										msgStruct->O_mag[2].MagType,
										msgStruct->O_mag[2].magAvg, 
										msgStruct->O_mag[2].numStas);
			fprintf (fp, "\t %c (%d) = %0.2f from %d phases\n", 
										msgStruct->O_mag[3].magLabel,
										msgStruct->O_mag[3].MagType,
										msgStruct->O_mag[3].magAvg, 	
										msgStruct->O_mag[3].numStas);
			fprintf (fp, "\t %c (%d) = %0.2f from %d phases\n", 
										msgStruct->O_mag[4].magLabel,
										msgStruct->O_mag[4].MagType,
										msgStruct->O_mag[4].magAvg, 
										msgStruct->O_mag[4].numStas);
			
			fprintf (fp, "\nThere were total of %d phases, of which %d were marked as used\n",
							msgStruct->nph_assoc, msgStruct->nph_used);
			fprintf (fp, "Std error (RMS) was %0.2f: \n", msgStruct->std_error);
			fprintf (fp, "Of those, we parsed the following %d phases: \n", msgStruct->nph_actual);
			for (i = 0; i < msgStruct->nph_actual; i++)
			{
			  fprintf (fp, "%s %s (%c %c) %0.2f   %0.1f,%0.1f,%d MAGS  ",
							msgStruct->phases[i].sta, msgStruct->phases[i].phase,
							msgStruct->phases[i].onset, msgStruct->phases[i].motion,
							msgStruct->phases[i].ot, msgStruct->phases[i].res,
							msgStruct->phases[i].dist, msgStruct->phases[i].azm);
			
			  for (j = 0; j < msgStruct->phases[i].num_mags; j++)
			  {
			    if (msgStruct->phases[i].mag[j].used == 1)
			      fprintf (fp, "USED %c (%d),%0.1f,%0.2f ==> %0.1f  ", 
					msgStruct->phases[i].mag[j].magLabel,
					msgStruct->phases[i].mag[j].MagType,
					msgStruct->phases[i].mag[j].value,
					msgStruct->phases[i].mag[j].period,
					msgStruct->phases[i].mag[j].mag);
			    else
			      fprintf (fp, "NOT USED %c (%d),%0.1f,%0.2f ==> %0.1f  ", 
					msgStruct->phases[i].mag[j].magLabel,
					msgStruct->phases[i].mag[j].MagType,
					msgStruct->phases[i].mag[j].value,
					msgStruct->phases[i].mag[j].period,
					msgStruct->phases[i].mag[j].mag);
			  }
			  fprintf (fp, "\n");
			
			}

			fclose (fp);
		}
		
	}
	else
	{
		logit ("", "GetNextLine returned EW_FAILURE; exitting\n");
		return EW_FAILURE;
	}


	/* free up allocated space */
	for (i = 0; i < MAX_LINES; i++)
		free ((void *) nsnbuf[i]);
	free ((void *) nsnbuf);

/*
logit ("", "BEFORE RETURN\n");
for (i = 0; i < msgStruct->nph_actual; i++)
  logit ("", "Chan %d, <%s>\n", i, msgStruct->phases[i].sta);
*/


	return EW_SUCCESS;

}


/************************************************************************
 * ConvertNSNMsg () - fill the buffer with the stuff in msg.
 *   set nlines to the number of lines read.
 ************************************************************************/
static	int		ConvertNSNMsg (char *msg, int msgsize, char **buffer, int *nlines)
{

	int		i, j, done;
	char	*line;

	if ((buffer == NULL) || (msg == NULL) || (msgsize < 0))
	{
		logit ("e", "ConvertNSNMsg: Invalid parameters passed in.\n");
		return EW_FAILURE;
	}

	*nlines = 0;

	done = FALSE;
	i = 0;
	j = 0;
	while (done == FALSE)
	{
		
		line = buffer[*nlines];

		while ((msg[i] != '\n') && (i < msgsize))
		{
			line[j] = msg[i];
			i = i + 1;
			j = j + 1;
		}
		if (msg[i] == '\n')
		{
/**
			line[j] = '\0';
**/
			line[j] = '\n';
			*nlines = *nlines + 1;
			j = 0;
			i = i + 1;
		}
		else
		{
			line[j] = '\0';
			done = TRUE;
		}
			
	}

	return EW_SUCCESS;
}



/************************************************************************
 * GetNextLine () - return the next line from the message (in buffer)
 *  where the first word of the line is given by argument word.
 ************************************************************************/
static	int 	GetNextLine (char **buffer, int *lineno, char *word)
{

	char	tmp[256];
	int		done;

	if ((buffer == NULL) || (word == NULL))
	{
		logit ("", "Invalid arguments passed in.\n");
		return EW_FAILURE;
	}

	done = FALSE;
	while (done == FALSE)
	{
		if (strlen (buffer[*lineno]) == 1)
		{}
		else
		{
			/* Check the first word */

			sscanf (buffer[*lineno], "%s\n", tmp);

			if (strcmp (tmp, word) == 0)
			{
				done = TRUE;
			}
			else
			{}
		}

		if (done != TRUE)
			*lineno = *lineno + 1;
	}

	return EW_SUCCESS;
}


/************************************************************************
 * BuildDateStr () - Create a date string suitable for a call to 
 *   epochsec17, from an NSN format date string.
 ************************************************************************/
static	int		BuildDateStr (char *in, char *out)
{

	char	year[6];
	char	month[6];
	char	day[6];

	if ((in == NULL) || (out == NULL))
	{
		logit ("", "Invalid arguments passed in.\n");
		return EW_FAILURE;
	}

	strncpy (day, in, 2);
	day[2] = '\0';

	if (day[0] == ' ')
		day[0] ='0';

	/* figure out the month */
	if (in[3] == 'J')
	{
		if (in[4] == 'A')
			sprintf (month, "01");
		else if (in[4] == 'U')
		{
			if (in[5] == 'N')
				sprintf (month, "06");
			else if (in[5] == 'L')
				sprintf (month, "07");
			else
			{
				logit ("", "Unknown month %s!\n", month);
				return EW_FAILURE;
			}
		}
		else
		{
			logit ("", "Unknown month %s!\n", month);
			return EW_FAILURE;
		}
	}
	else if (in[3] == 'M')
	{
		if (in[5] == 'R')
			sprintf (month, "03");
		else if (in[5] == 'Y')
			sprintf (month, "05");
		else
		{
			logit ("", "Unknown month %s!\n", month);
			return EW_FAILURE;
		}
	}
	else if (in[3] == 'A')
	{
		if (in[4] == 'P')
			sprintf (month, "04");
		else if (in[4] == 'U')
			sprintf (month, "08");
		else
		{
			logit ("", "Unknown month %s!\n", month);
			return EW_FAILURE;
		}
	}
	else if (in[3] == 'F')
		sprintf (month, "02");
	else if (in[3] == 'S')
		sprintf (month, "09");
	else if (in[3] == 'O')
		sprintf (month, "10");
	else if (in[3] == 'N')
		sprintf (month, "11");
	else if (in[3] == 'D')
		sprintf (month, "12");
	else
	{
		logit ("", "Unknown month %s!\n", month);
		return EW_FAILURE;
	}

	strncpy (year, (in + 7), 4);
	year[4] = '\0';

	sprintf (out, "%s%s%s", year, month, day);


	return EW_SUCCESS;

}


/************************************************************************
 * AppendDateStr () - Append hours, minutes, and seconds to the 
 *   date, to create a string suitable for a call to epochsec17
 ************************************************************************/
static	int		AppendDateStr (char	*orig, char *append)
{

	int		i, j;
	char	tmp[10];

	/* strip off ':'s and append to the old date string */

	if ((orig == NULL) || (append == NULL))
	{
		logit ("", "Invalid arguments passed in.\n");
		return EW_FAILURE;
	}

	i = 0;
	j = 0;

	for (i = 0; i < 11; i++)
	{
		if (append[i] == ':')
			;
		else
		{
			tmp[j] = append[i];
			j = j + 1;
		}
	}
	tmp[j] = '\0';

	strcat (orig, tmp);

	return EW_SUCCESS;
			

}

/************************************************************************
 * ParseEllipse () - fill the Ellipse structure from the NSN formatted
 *   error elipse string
 ************************************************************************/
static	int		ParseEllipse (char *str, double depth_err, Err_Elipse *error)
{

	int		i, j, k, l, index;
	double	err[9];
	char	tmp[10];

	if ((str == NULL) || (error == NULL))
	{
		logit ("", "Invalid arguments passed in.\n");
		return EW_FAILURE;
	}

	/* Extract numbers */
	i = 0;
	index = 0;
	/* Loop over 3 sets of numbers */
	for (k = 0; k < 3; k++)
	{
		/* Loop over 3 numbers in each set */
		for (l = 0; l < 3; l++)
		{
			j = 0;
			while ((i < ELLIPSE_LENGTH) && (str[i] != ',') && (str[i] != ';'))
			{
				tmp[j] = str[i];
				j = j + 1;
				i = i + 1;
			}
			
			/* Got a number, convert it */
			tmp[j] = '\0';
			err[index] = atof (tmp);
			index = index + 1;
			i = i + 1;
		}
	}


	/* For some reason, the meaning of the ellipse changes
	 * if depth is fixed:
	 *
	 *   ind 0  =  Semi-major strike
	 *   ind 1  =  Semi-minor strike
	 *   ind 2  =  Intermediate strike
	 *
	 *   ind 3  =  Semi-major dip
	 *   ind 4  =  Semi-minor dip
	 *   ind 5  =  Intermediate dip
	 *
	 *   ind 6  =  Semi-major length
	 *   ind 7  =  Semi-minor length
	 *   ind 8  =  Intermediate length
	 *
	 *  For non-fixed depth, the Semi-minor and intermediate axes
	 *  are swapped.
	 *
	 */

	if (depth_err != 0.0)
	{
		error->maj_s = err[0];
		error->maj_d = err[3];
		error->maj_l = err[6];

		error->int_s = err[1];
		error->int_d = err[4];
		error->int_l = err[7];

		error->min_s = err[2];
		error->min_d = err[5];
		error->min_l = err[8];
	}
	else
	{
		error->maj_s = err[0];
		error->maj_d = err[3];
		error->maj_l = err[6];

		error->min_s = err[1];
		error->min_d = err[4];
		error->min_l = err[7];

		error->int_s = err[2];
		error->int_d = err[5];
		error->int_l = err[8];
	}

	return EW_SUCCESS;

}

/************************************************************************
 * ParseMagnitude () - fill the Mag structure from the NSN formatted
 *   magnitudes string
 ************************************************************************/
static	int		ParseMagnitude (char *str, OriginMag *mag)
{

	char	tmp[10];

	if ((str == NULL) || (mag == NULL))
	{
		logit ("", "Invalid arguments passed in.\n");
		return EW_FAILURE;
	}


	/* HACK: we only know about duration and local mags */
	
	/* Mb */
	mag[0].magLabel = 'b';
	mag[0].MagType = MAGTYPE_LOCAL_ZERO2PEAK;
	strncpy (tmp, (str + MB_BASE), MAG_LEN);
	tmp[MAG_LEN] = '\0';
	mag[0].magAvg = (double) atof (tmp);

	strncpy (tmp, (str + MB_DATUM), DATUM_LEN);
	tmp[DATUM_LEN] = '\0';
	mag[0].numStas = (int) atoi (tmp);


	/* ML */
	mag[1].magLabel = 'L';
	mag[1].MagType = MAGTYPE_LOCAL_ZERO2PEAK;
	strncpy (tmp, (str + ML_BASE), MAG_LEN);
	tmp[MAG_LEN] = '\0';
	mag[1].magAvg = (double) atof (tmp);

	strncpy (tmp, (str + ML_DATUM), DATUM_LEN);
	tmp[DATUM_LEN] = '\0';
	mag[1].numStas = (int) atoi (tmp);


	/* Mblg */
	mag[2].magLabel = 'g';
	mag[2].MagType = MAGTYPE_LOCAL_ZERO2PEAK;
	strncpy (tmp, (str + MBLG_BASE), MAG_LEN);
	tmp[MAG_LEN] = '\0';
	mag[2].magAvg = (double) atof (tmp);

	strncpy (tmp, (str + MBLG_DATUM), DATUM_LEN);
	tmp[DATUM_LEN] = '\0';
	mag[2].numStas = (int) atoi (tmp);


	/* Md */
	mag[3].magLabel = 'd';
	mag[3].MagType = MAGTYPE_DURATION;
	strncpy (tmp, (str + MD_BASE), MAG_LEN);
	tmp[MAG_LEN] = '\0';
	mag[3].magAvg = (double) atof (tmp);

	strncpy (tmp, (str + MD_DATUM), DATUM_LEN);
	tmp[DATUM_LEN] = '\0';
	mag[3].numStas = (int) atoi (tmp);


	/* MS */
	mag[4].magLabel = 'S';
	mag[4].MagType = MAGTYPE_LOCAL_ZERO2PEAK;
	strncpy (tmp, (str + MS_BASE), MAG_LEN);
	tmp[MAG_LEN] = '\0';
	mag[4].magAvg = (double) atof (tmp);

	strncpy (tmp, (str + MS_DATUM), DATUM_LEN);
	tmp[DATUM_LEN] = '\0';
	mag[4].numStas = (int) atoi (tmp);

	return EW_SUCCESS;
}

/************************************************************************
 * ParsePhase () - fill the Phase structure from the NSN formatted
 *   phase string
 ************************************************************************/
static	int		ParsePhase (char *str, Phase *phase, char *EventDate, 
									int *num_phases, double *Dmin)
{
	char	tmp[256];
	char	datestr[256];
	int		i, j;

	if ((str == NULL) || (phase == NULL))
	{
		logit ("", "Invalid arguments passed in\n");
		return EW_FAILURE;
	}


	/* ingore phases that don't start with the station name */
	if (str[1] == ' ')
	{
		return EW_SUCCESS;
	}


	strncpy (phase->sta, (str + STA_BASE), STA_LEN);
	phase->sta[STA_LEN] = '\0';

	strncpy (tmp, (str + PHA_BASE), PHA_LEN);
	tmp[PHA_LEN] = '\0';

	j = 0;
	phase->onset = ' ';
	phase->motion = ' ';
	for (i = 0; i < PHA_LEN; i++)
	{
		if (tmp[i] == '?')
			phase->automatic = 1;
		/* onset codes */
		else if (tmp[i] == 'i')
			phase->onset = 'i';
		else if (tmp[i] == 'e')
			phase->onset = 'e';
		else if (tmp[i] == 'q')
			phase->onset = 'q';

		/* first motion codes */
		else if (tmp[i] == 'c')
			phase->motion = 'c';
		else if (tmp[i] == 'd')
			phase->motion = 'd';
		else if (tmp[i] == 'u')
			phase->motion = 'u';
		else if (tmp[i] == 'r')
			phase->motion = 'r';
		else if (tmp[i] == ' ')
			;

		/* part of the phase code */
		else
		{
			phase->phase[j] = tmp[i];
			j = j + 1;
		}
	}

	if (j >= PHA_LEN)
	{
		logit ("e", "MAX Phase length (%d) exceeded!\n", PHA_LEN);
		return EW_FAILURE;
	}
	phase->phase[j] = '\0';

	strncpy (tmp, (str + OT_BASE), OT_LEN);
	tmp[OT_LEN] = '\0';

	strcpy (datestr, EventDate);
	if (AppendDateStr (datestr, tmp) != EW_SUCCESS)
	{
		logit ("", "Call to AppendDateStr failed.\n");
		return EW_FAILURE;
	}
	epochsec17 (&phase->ot, datestr);


	strncpy (tmp, (str + RES_BASE), RES_LEN);
	tmp[RES_LEN] = '\0';
	phase->res = (double) atof (tmp);

	if (str[RES_USED] == 'X')
		phase->res_used = 1;
	else
		phase->res_used = 0;

	strncpy (tmp, (str + DIST_BASE), DIST_LEN);
	tmp[DIST_LEN] = '\0';
	phase->dist = (double) atof (tmp);

	/* convert from degrees to km */
	phase->dist = (phase->dist * EARTH_CIRCUM)/360.0;

	if (phase->dist <= *Dmin)
	{
		*Dmin = phase->dist;
	}

	strncpy (tmp, (str + AZM_BASE), AZM_LEN);
	tmp[AZM_LEN] = '\0';
	phase->azm = (int) atoi (tmp);


	/* magnitudes */
	phase->num_mags = 0;
	if (str[MAG1_BASE] != ' ')
	{
		if (ParsePhaseMagnitude ((str+MAG1_BASE), &phase->mag[0]) != EW_SUCCESS)
		{
			logit ("", "Call to ParsePhaseMagnitude failed\n");
			return EW_FAILURE;
		}
		phase->num_mags = phase->num_mags + 1;

	}
	if (str[MAG2_BASE] != ' ')
	{

		if (ParsePhaseMagnitude ((str+MAG2_BASE), &phase->mag[1]) != EW_SUCCESS)
		{
			logit ("", "Call to ParsePhaseMagnitude failed\n");
			return EW_FAILURE;
		}
		phase->num_mags = phase->num_mags + 1;
	}

	*num_phases = *num_phases + 1;
	return EW_SUCCESS;
}

/************************************************************************
 * ParsePhaseMagnitude () - fill the PhaseMag structure from the NSN 
 *   formatted phase string
 ************************************************************************/
static	int		ParsePhaseMagnitude (char *str, PhaseMag *mag)
{
	char	*ptr;
	char	tmp[5];
	int		expn, mult;
        mult = 0;

	if ((str == NULL) || (mag == NULL))
	{
		logit ("", "Invalid arguments passed in\n");
		return EW_FAILURE;
	}


	/* Hack: we only know duration and local mags */
	mag->magLabel = str[0];
	if (mag->magLabel == 'b')
		mag->MagType = MAGTYPE_LOCAL_ZERO2PEAK;
	else if (mag->magLabel == 'L')
		mag->MagType = MAGTYPE_LOCAL_ZERO2PEAK;
	else if (mag->magLabel == 'g') /* HACK -- we don't have Mblg type yet */
		mag->MagType = MAGTYPE_LOCAL_ZERO2PEAK;
	else if (mag->magLabel == 'd')
		mag->MagType = MAGTYPE_DURATION;
	else if (mag->magLabel == 'S')
		mag->MagType = MAGTYPE_LOCAL_ZERO2PEAK;
	else
	{
		logit ("", "Invalid magnitude type: %c\n", mag->magLabel);
		return EW_FAILURE;
	}

	ptr = str + 2;
	strncpy (tmp, ptr, 3);
	tmp[3] = '\0';
	mag->value = (double) atof (tmp);

	ptr = ptr + 3;
	if (ptr[0] == '+')
		mult = 1;
	else if (ptr[0] == '-')
		mult = -1;

	ptr = ptr + 1;
	strncpy (tmp, ptr, 1);
	tmp[1] = '\0';
	expn = mult * ((int) atoi (tmp));
	mag->value = (double) (mag->value * pow (10.0, (double) expn));

	ptr = ptr + 1;
	strncpy (tmp, ptr, 4);
	tmp[4] = '\0';
	mag->period = (double) atof (tmp);

	ptr = ptr + 5;
	strncpy (tmp, ptr, 3);
	tmp[3] = '\0';
	mag->mag = (double) atof (tmp);

	ptr = ptr + 3;

	if (ptr[0] != 'X')
		mag->used = 1;
	else
		mag->used = 0;


	return EW_SUCCESS;

}
