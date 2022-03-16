#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <global_amp_rw.h>
#include <earthworm.h>

#include <kom.h>  /* string parsing */


/*
** Version currently handled by this code
*/
#define GLOBAL_AMP_VERSION (short)1

#define NUMAMPTOKENS 11

/* ------------------------------------------------------------------------- */

GLOBAL_MSG_STATUS InitGlobalAmp( GLOBAL_AMP_STRUCT * p_struct )
{
   if ( p_struct == NULL )
   {
     return GLOBAL_MSG_NULL;
   }
   p_struct->version = GLOBAL_AMP_VERSION;
   p_struct->logo.instid = 0;
   p_struct->logo.mod = 0;
   p_struct->logo.type = 0;
   p_struct->sequence = 0;
   strcpy( p_struct->station, "?" );
   strcpy( p_struct->channel, "?" );
   strcpy( p_struct->network, "?" );
   strcpy( p_struct->location, "?" );
   strcpy( p_struct->amp_time, "?" );
   p_struct->amptype = MAGTYPE_UNDEFINED;
   p_struct->adcounts = 0.0;
   p_struct->period = 0.0;

   return GLOBAL_MSG_SUCCESS;
}

/* ------------------------------------------------------------------------- */

GLOBAL_MSG_STATUS WriteAmpToBuffer( GLOBAL_AMP_STRUCT * p_struct
                                  , char              * p_buffer
                                  , unsigned int        p_length
                                  )
{
   char _author[10];

   if ( p_struct == NULL )
   {
      return GLOBAL_MSG_NULL;
   }

   if ( p_buffer == NULL )
   {
      return GLOBAL_MSG_BADPARAM;
   }

   if ( p_length < GLOBAL_AMP_MAXBUFSIZE )
   {
      return GLOBAL_MSG_TOOSMALL;
   }

   if ( p_struct->version != GLOBAL_AMP_VERSION )
   {
      return GLOBAL_MSG_VERSINVALID;
   }

   EncodeAuthor( p_struct->logo, _author );

   sprintf( p_buffer
          , "%s %ld %d %s %s %s %s %s %d %1.4f %1.4f\n"
          ,  _author
          , p_struct->sequence
          , p_struct->version
          , p_struct->station
          , p_struct->channel
          , p_struct->network
          , p_struct->location
          , p_struct->amp_time
          , p_struct->amptype
          , p_struct->adcounts
          , p_struct->period
          );

   return GLOBAL_MSG_SUCCESS;
}

/* ------------------------------------------------------------------------- */



GLOBAL_MSG_STATUS StringToAmp( GLOBAL_AMP_STRUCT * p_struct, char * p_string )
{
	char _str[80];
	char AmpString[256];
	int  tokencount;
	int  count;
	int  temp;
	int  found;
	char * NextToken;

	/* Check incoming parameters for sanity
	 **************************************/
	if ( p_struct == NULL )
	{
		return GLOBAL_MSG_NULL;
	}

	if ( p_struct->version != GLOBAL_AMP_VERSION )
	{
		 return GLOBAL_MSG_VERSINVALID;
	}

	if ( p_string == NULL )
	{
		return GLOBAL_MSG_BADPARAM;
	}

	/* Initilize varibles
	 ********************/
	InitGlobalAmp(p_struct);
	tokencount = 0;
	strcpy(AmpString, p_string);

	/* First count the number of tokens
	 * in the message, if we don't = the
	 * expected number, return with error
	 ************************************/
	NextToken = strtok(AmpString, " \n");
	while (NextToken != NULL)
	{
		NextToken = strtok(NULL, " \n");
		tokencount++;
	}

	if (tokencount != NUMAMPTOKENS)
	{
		/* Wrong number of tokens, something's
	     * messed up, return with error
		 *************************************/
		logit ("e", "StringToAmp: Wrong number of amp tokens\n");
		return GLOBAL_MSG_FORMATERROR;
	}

	/* This message has at the right number
	 * of tokens at least, reinitilize PickString
	 * and move on
	 ********************************************/
	strcpy(AmpString, p_string);

	/* Try to parse out a pick message from this string
	 **************************************************/
 
	/* Get first token */
	NextToken = strtok(AmpString, " \n");
	strcpy( _str , NextToken );

	/* Author:  
	 * This should appear as a 9 digit long integer
	 ************************************************************************************/

	/* check length */
	if ( strlen(_str) != 9 )
	{
		logit ("e", "StringToAmp: Bad Logo Length, length is %zu, token is <%s>\n",strlen(_str), _str);
		return GLOBAL_MSG_FORMATERROR;
	}

	/* check to see that only digits are present */
	for (count = 0; count < 9; count++)
	{
		if (isdigit(_str[count]) == 0)
		{
			/* This character is not a digit, author must have
			 * only digits in it, return with error
			 *************************************************/
			logit ("e", "StringToAmp: Logo does not only contain digits\n");
			return GLOBAL_MSG_FORMATERROR;
		}
	}

	/* attept to decode author */
	if ( DecodeAuthor( &(p_struct->logo), _str ) != GLOBAL_MSG_SUCCESS )
	{
		logit ("e", "StringToAmp: Failed to decode author\n");
		return GLOBAL_MSG_FORMATERROR;
	}


	/* Get next token */
	NextToken = strtok(NULL, " \n");
	strcpy( _str , NextToken );


	/* Sequence number: 
	 * This should be a long integer
	 ************************************************************************************/

	/* get length of sequence number */
	temp = strlen(_str);

	/* check to see that only digits are present */
	for (count = 0; count < temp; count++)
	{
		if (isdigit(_str[count]) == 0)
		{
			/* This character is not a digit, sequence number 
			 * must have only digits in it, return with error
			 ************************************************/
			logit ("e", "StringToAmp: Sequence number does not only contain digits\n");
			return GLOBAL_MSG_FORMATERROR;
		}
	}

	/* attempt to decode sequence number */
	p_struct->sequence = atol(_str);

	/* 0 is not a valid sequence number, plus
	 * atol returns 0 if it failed
	 ****************************************/
	if (p_struct->sequence == 0)
	{
		logit ("e", "StringToAmp: Invalid value 0 for sequence number\n");
		return GLOBAL_MSG_FORMATERROR;
	}


	/* Get next token */
	NextToken = strtok(NULL, " \n");
	strcpy( _str , NextToken );


	/* Version number:
	 * This should be a 1 digit short integer 
	 ************************************************************************************/

	/* check length */
	if ( strlen(_str) != 1 )
	{
		logit ("e", "StringToAmp: Version is too long\n");
		return GLOBAL_MSG_FORMATERROR;
	}

	/* check to see that only a digit is present */
	if (isdigit(_str[0]) == 0)
	{
		/* This character is not a digit, version must have
		 * only digits in it, return with error
		 **************************************************/
		logit ("e", "StringToAmp: Version number does not only contain digits\n");
		return GLOBAL_MSG_FORMATERROR;
	}

	/* attempt to decode version number */
	p_struct->version = (short)atoi(_str);

	/* 0 is not a valid version number, plus
	 * atoi returns 0 if it failed
	 ***************************************/
	if (p_struct->version == 0)
	{
		logit ("e", "StringToAmp: Invalid value 0 for version number\n");
		return GLOBAL_MSG_FORMATERROR;
	}

	/* check to see if this message has the right version */
	if ( p_struct->version != GLOBAL_AMP_VERSION )
	{
		return GLOBAL_MSG_VERSINVALID;
	}


	/* Get next token */
	NextToken = strtok(NULL, " \n");
	strcpy( _str , NextToken );


   /* Station:
	* This is a alphanumeric string
	************************************************************************************/

	/* get length of station */
	temp = strlen(_str);

	/* check to see that only letters or digits are present */
	for (count = 0; count < temp; count++)
	{
		if (isalnum(_str[count]) == 0)
		{
			/* This character is not a letter or digit, station 
			 * must have only letters or digits in it, return 
			 * with error
			 **************************************************/
			logit ("e", "StringToAmp: Station name does not only contain letters or digits\n");
			return GLOBAL_MSG_FORMATERROR;
		}
	}
	
	/* attempt to decode station */
	strcpy( p_struct->station , _str);


	/* Get next token */
	NextToken = strtok(NULL, " \n");
	strcpy( _str , NextToken );


	/* Channel:
	 * This is a alphanumeric string
	 ************************************************************************************/

	/* get length of channel */
	temp = strlen(_str);

	/* check to see that only letters or digits are present */
	for (count = 0; count < temp; count++)
	{
		if (isalnum(_str[count]) == 0)
		{
			/* This character is not a letter or digit, channel 
			 * must have only letters or digits in it, return 
			 * with error
			 **************************************************/
			logit ("e", "StringToAmp: Channel name does not contain only letters or digits\n");
			return GLOBAL_MSG_FORMATERROR;
		}
	}
	
	/* attempt to decode channel */
	strcpy( p_struct->channel , _str);


	/* Get next token */
	NextToken = strtok(NULL, " \n");
	strcpy( _str , NextToken );	
	

	

	/* Network:
	 * This can be just about any string
	 ************************************************************************************/

	/* get length of network */
	temp = strlen(_str);

	/* check to see that only valid characters are present */
	for (count = 0; count < temp; count++)
	{
		if (isgraph(_str[count]) == 0)
		{
			/* This character is not a valid character, network 
			 * must have only valid characters, return with error
			 ****************************************************/
			logit ("e", "StringToPick: Network name does not contain a valid character\n");
			return GLOBAL_MSG_FORMATERROR;
		}
	}
	
	/* attempt to decode network */
	strcpy( p_struct->network , _str);


	/* Get next token */
	NextToken = strtok(NULL, " \n");
	strcpy( _str , NextToken );	


	/* Location:
	 * This can be just about any string
	 ************************************************************************************/

	/* get length of location */
	temp = strlen(_str);

	/* check to see that only valid characters are present */
	for (count = 0; count < temp; count++)
	{
		if (isgraph(_str[count]) == 0)
		{
			/* This character is not a valid character, location 
			 * must have only valid characters, return with error
			 ****************************************************/
			logit ("e", "StringToPick: Location name does not contain a valid character\n");
			return GLOBAL_MSG_FORMATERROR;
		}
	}
	
	/* attempt to decode location */
	strcpy( p_struct->location , _str);


	/* Get next token */
	NextToken = strtok(NULL, " \n");
	strcpy( _str , NextToken );	

 
	/* Amp Time:
	 * This should be a numaric string with a decimal point 
	 ************************************************************************************/

	/* check length */
	if ( strlen(_str) > 20 )
	{
		/* This allows for 5 decimal points, should be enough */
		logit ("e", "StringToAmp: Amp time is too long\n");
		return GLOBAL_MSG_FORMATERROR;
	}

	if (strlen(_str) < 16)
	{
		/* This allows for 5 decimal points, should be enough */
		logit ("e", "StringToAmp: Amp time is too short\n");
		return GLOBAL_MSG_FORMATERROR;
	}

	/* check to see that only a digit or decimal point is present */
	for (count = 0; count < temp; count++)
	{
		if (isdigit(_str[0]) == 0)
		{
			/* This character is not a digit, pick time check
			 * to see if this is the right place for the decimal
			 * point
			 **************************************************/

			if (count == 13)
			{
				if (_str[count] != '.')
				{
					/* This character is not the decimal point, or is in
					 * the wrong spot, return with error
					 ***************************************************/
					logit ("e", "StringToAmp: Amp time doesn't have the decimal point in the right spot\n");
					return GLOBAL_MSG_FORMATERROR;
				}
			}
			else
			{
				logit ("e", "StringToAmp: Amp time contains a non-numaric character\n");
				return GLOBAL_MSG_FORMATERROR;
			}
		}

	}

	/* attempt to decode pick time */
	strcpy( p_struct->amp_time , _str);


	/* Get next token */
	NextToken = strtok(NULL, " \n");
	strcpy( _str , NextToken );	


	/* Amp Type:
	 * This should be a numaric string 
	 ************************************************************************************/
 
	/* get length of Amp Type */
	temp = strlen(_str);

	/* check to see that only valid characters are present */
	for (count = 0; count < temp; count++)
	{
		if (isdigit(_str[count]) == 0)
		{
			/* This character is not a valid character, location 
			 * must have only valid characters, return with error
			 ****************************************************/
			logit ("e", "StringToAmp: Amp Type does not contain only numbers\n");
			return GLOBAL_MSG_FORMATERROR;
		}
	}
	
	/* attempt to decode Amp Type */
	p_struct->amptype = (MAGNITUDE_TYPE) atoi(_str);

	if ( p_struct->amptype < 0)
	{
		logit ("e", "StringToAmp: Amp Type cannot be less than 0\n");
		return GLOBAL_MSG_FORMATERROR;
	}


	/* Get next token */
	NextToken = strtok(NULL, " \n");
	strcpy( _str , NextToken );	

	/* Amplitude:
	 * This should be a numaric string with a decimal point 
	 ************************************************************************************/

	/* get length of Amplitude */
	temp = strlen(_str);

	found = 0;

	/* check to see that only valid characters are present */
	for (count = 0; count < temp; count++)
	{
		if (isdigit(_str[count]) == 0)
		{
			/* This character is not a valid character, check to 
			 * see if it is the single decimal point
			 ***************************************************/
			if (_str[count] != '.')
			{
				/* This character is not a valid character, phase name 
				 * must have only valid characters, return with error
				 ****************************************************/
				logit ("e", "StringToAmp: Amplitude does not contain a valid character\n");
				return GLOBAL_MSG_FORMATERROR;
			}

			if ((_str[count] == '.') && (found == 1))
			{
				/* We already have one decimal point, we don't need
				 * another
				 ***************************************************/
				logit ("e", "StringToAmp: Amplitude has two decimal points\n");
				return GLOBAL_MSG_FORMATERROR;
			}

			if ((_str[count] == '.') && (found == 0))
			{
				/* This is the first decimal point we've found 
				 *********************************************/
				found = 1;
			}


		}
	}
	
	/* attempt to decode amplitude */
	p_struct->adcounts = atof(_str);


	/* Get last token */
	NextToken = strtok(NULL, " \n");
	strcpy( _str , NextToken );	

 
	/* Amp Period:
	 * This should be a numaric string with a decimal point 
	 ************************************************************************************/

	/* get length of Amp Period */
	temp = strlen(_str);

	found = 0;

	/* check to see that only valid characters are present */
	for (count = 0; count < temp; count++)
	{
		if (isdigit(_str[count]) == 0)
		{
			/* This character is not a valid character, check to 
			 * see if it is the single decimal point
			 ***************************************************/
			if (_str[count] != '.')
			{
				/* This character is not a valid character, phase name 
				 * must have only valid characters, return with error
				 ****************************************************/
				logit ("e", "StringToAmp: Amp Period does not contain a valid character\n");
				return GLOBAL_MSG_FORMATERROR;
			}

			if ((_str[count] == '.') && (found == 1))
			{
				/* We already have one decimal point, we don't need
				 * another
				 ***************************************************/
				logit ("e", "StringToAmp: Amp Period has two decimal points\n");
				return GLOBAL_MSG_FORMATERROR;
			}

			if ((_str[count] == '.') && (found == 0))
			{
				/* This is the first decimal point we've found 
				 *********************************************/
				found = 1;
			}


		}
	}
	
	/* attempt to decode amp period */
	p_struct->period = atof(_str);
 
	/* DONE!!*/
	return GLOBAL_MSG_SUCCESS;
}

