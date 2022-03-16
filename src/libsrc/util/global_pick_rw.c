

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <global_pick_rw.h>
#include <earthworm_simple_funcs.h>
#include <kom.h>  /* string parsing */


/*
** Version currently handled by this code
*/
#define GLOBAL_PICK_VERSION (short)1

#define NUMPICKTOKENS 11

/* ------------------------------------------------------------------------- */

GLOBAL_MSG_STATUS InitGlobalPick( GLOBAL_PICK_STRUCT * p_struct )
{
   if ( p_struct == NULL )
   {
     return GLOBAL_MSG_NULL;
   }
   p_struct->version = GLOBAL_PICK_VERSION;
   p_struct->logo.instid = 0;
   p_struct->logo.mod = 0;
   p_struct->logo.type = 0;
   p_struct->sequence = 0;
   strcpy( p_struct->station, "?" );
   strcpy( p_struct->channel, "?" );
   strcpy( p_struct->network, "?" );
   strcpy( p_struct->location, "?" );
   strcpy( p_struct->pick_time, "?" );
   strcpy( p_struct->phase_name, "?" );
   p_struct->quality = 0.0;
   p_struct->polarity = '?';

   return GLOBAL_MSG_SUCCESS;
}

/* ------------------------------------------------------------------------- */

GLOBAL_MSG_STATUS WritePickToBuffer( GLOBAL_PICK_STRUCT * p_struct
                                   , char               * p_buffer
                                   , unsigned int         p_length
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

   if ( p_length < GLOBAL_PICK_MAXBUFSIZE )
   {
      return GLOBAL_MSG_TOOSMALL;
   }

   if ( p_struct->version != GLOBAL_PICK_VERSION )
   {
      return GLOBAL_MSG_VERSINVALID;
   }

   EncodeAuthor( p_struct->logo, _author );

   sprintf( p_buffer
          , "%s %ld %d %s %s %s %s %s %s %1.4f %c\n"
          ,  _author
          , p_struct->sequence
          , p_struct->version
          , p_struct->station
          , p_struct->channel
          , p_struct->network
          , p_struct->location
          , p_struct->pick_time
          , p_struct->phase_name
		  , p_struct->quality
		  , p_struct->polarity
          );

   return GLOBAL_MSG_SUCCESS;
}

/* ------------------------------------------------------------------------- */



GLOBAL_MSG_STATUS StringToPick( GLOBAL_PICK_STRUCT * p_struct, char * p_string )
{
	char _str[80];
	char PickString[256];
	int  tokencount;
	int  count;
	int  temp;
	int  found;
	char * NextToken;

/*	logit ("e", "\tStringToPick: Startup\n");*/

	/* Check incoming parameters for sanity
	 **************************************/
	if ( p_struct == NULL )
	{
		logit ("e", "StringToPick: NULL GLOBAL_PICK_STRUCT\n");
		return GLOBAL_MSG_NULL;
	}

	if ( p_struct->version != GLOBAL_PICK_VERSION )
	{
		logit ("e", "StringToPick: Invalid GLOBAL_PICK_STRUCT version\n");
		return GLOBAL_MSG_VERSINVALID;
	}

	if ( p_string == NULL )
	{
		logit ("e", "StringToPick: NULL Incoming message\n");
		return GLOBAL_MSG_BADPARAM;
	}

	/* Initilize varibles
	 ********************/
	InitGlobalPick(p_struct);
	tokencount = 0;
	strcpy(PickString, p_string);

/*	logit ("e", "\tStringToPick: Initalization\n");*/

	/* First count the number of tokens
	 * in the message, if we don't = the
	 * expected number, return with error
	 ************************************/
	NextToken = strtok(PickString, " \n");
	while (NextToken != NULL)
	{
		NextToken = strtok(NULL, " \n");
		tokencount++;
	}

	if (tokencount != NUMPICKTOKENS)
	{
		/* Wrong number of tokens, something's
	     * messed up, return with error
		 *************************************/
		logit ("e", "StringToPick: Wrong number of pick tokens\n");
		return GLOBAL_MSG_FORMATERROR;
	}

/*	logit ("e", "\tStringToPick: Token Count\n");*/

	/* This message has at the right number
	 * of tokens at least, reinitilize PickString
	 * and move on
	 ********************************************/
	strcpy(PickString, p_string);

	/* Try to parse out a pick message from this string
	 **************************************************/
 
	/* Get first token */
	NextToken = strtok(PickString, " \n");
	strcpy( _str , NextToken );

/*	logit ("e", "\tStringToPick: Author - ");*/

	/* Author:  
	 * This should appear as a 9 digit long integer
	 **********************************************/

	/* check length */
	if ( strlen(_str) != 9 )
	{
		logit ("e", "StringToPick: Bad Logo Length\n");
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
			logit ("e", "StringToPick: Logo does not only contain digits\n");
			return GLOBAL_MSG_FORMATERROR;
		}
	}

	/* attept to decode author */
	if ( DecodeAuthor( &(p_struct->logo), _str ) != GLOBAL_MSG_SUCCESS )
	{
		logit ("e", "StringToPick: Failed to decode author\n");
		return GLOBAL_MSG_FORMATERROR;
	}


	/* Get next token */
	NextToken = strtok(NULL, " \n");
	strcpy( _str , NextToken );

/*	logit ("e", "Sequence - ");*/

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
			logit ("e", "StringToPick: Sequence number does not only contain digits\n");
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
		logit ("e", "StringToPick: Invalid value 0 for sequence number\n");
		return GLOBAL_MSG_FORMATERROR;
	}


	/* Get next token */
	NextToken = strtok(NULL, " \n");
	strcpy( _str , NextToken );

/*	logit ("e", "Version - ");*/

	/* Version number:
	 * This should be a 1 digit short integer 
	 ************************************************************************************/

	/* check length */
	if ( strlen(_str) != 1 )
	{
		logit ("e", "StringToPick: Version is too long\n");
		return GLOBAL_MSG_FORMATERROR;
	}

	/* check to see that only a digit is present */
	if (isdigit(_str[0]) == 0)
	{
		/* This character is not a digit, version must have
		 * only digits in it, return with error
		 **************************************************/
		logit ("e", "StringToPick: Version number does not only contain digits\n");
		return GLOBAL_MSG_FORMATERROR;
	}

	/* attempt to decode version number */
	p_struct->version = (short)atoi(_str);

	/* 0 is not a valid version number, plus
	 * atoi returns 0 if it failed
	 ***************************************/
	if (p_struct->version == 0)
	{
		logit ("e", "StringToPick: Invalid value 0 for version number\n");
		return GLOBAL_MSG_FORMATERROR;
	}

	/* check to see if this message has the right version */
	if ( p_struct->version != GLOBAL_PICK_VERSION )
	{
		return GLOBAL_MSG_VERSINVALID;
	}


	/* Get next token */
	NextToken = strtok(NULL, " \n");
	strcpy( _str , NextToken );

/*	logit ("e", "Station - ");*/

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
			logit ("e", "StringToPick: Station name does not only contain letters or digits\n");
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
			logit ("e", "StringToPick: Channel name does not contain only letters or digits\n");
			return GLOBAL_MSG_FORMATERROR;
		}
	}
	
	/* attempt to decode channel */
	strcpy( p_struct->channel , _str);


	/* Get next token */
	NextToken = strtok(NULL, " \n");
	strcpy( _str , NextToken );	
	
/*	logit ("e", "Network - ");*/

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

/*	logit ("e", "Location - ");*/

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

/*	logit ("e", "Pick Time - ");*/

	/* Pick Time:
	 * This should be a numaric string with a decimal point 
	 ************************************************************************************/

	/* check length */
	if ( strlen(_str) > 20 )
	{
		/* This allows for 5 decimal points, should be enough */
		logit ("e", "StringToPick: Pick time is too long\n");
		return GLOBAL_MSG_FORMATERROR;
	}

	if (strlen(_str) < 16)
	{
		/* This allows for 5 decimal points, should be enough */
		logit ("e", "StringToPick: Pick time is too short\n");
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
					logit ("e", "StringToPick: Pick time doesn't have the decimal point in the right spot\n");
					return GLOBAL_MSG_FORMATERROR;
				}
			}
			else
			{
				logit ("e", "StringToPick: Pick time contains a non-numaric character\n");
				return GLOBAL_MSG_FORMATERROR;
			}
		}

	}

	/* attempt to decode pick time */
	strcpy( p_struct->pick_time , _str);


	/* Get next token */
	NextToken = strtok(NULL, " \n");
	strcpy( _str , NextToken );	

/*	logit ("e", "Phase Name - ");*/

   /* Phase Name:
	* This should be a alpha string, or a question mark 
	************************************************************************************/

	/* get length of phase name */
	temp = strlen(_str);

	/* check to see that only valid characters are present */
	for (count = 0; count < temp; count++)
	{
		if (isalpha(_str[count]) == 0)
		{
			/* This character is not a valid character, check to 
			 * see if it is a question mark
			 ***************************************************/
			if (_str[count] != '?')
			{
				/* This character is not a valid character, phase name 
				 * must have only valid characters, return with error
				 ****************************************************/
				logit ("e", "StringToPick: Phase name does not contain a valid character\n");
				return GLOBAL_MSG_FORMATERROR;
			}
		}
	}
	
	/* attempt to decode phase name */
	strcpy( p_struct->phase_name , _str);


	/* Get next token */
	NextToken = strtok(NULL, " \n");
	strcpy( _str , NextToken );	

	/* Quality:
	 * This should be a numaric string with a decimal point 
	 ************************************************************************************/

	/* get length of Quality */
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
				logit ("e", "StringToPick: Quality does not contain a valid character\n");
				return GLOBAL_MSG_FORMATERROR;
			}

			if ((_str[count] == '.') && (found == 1))
			{
				/* We already have one decimal point, we don't need
				 * another
				 ***************************************************/
				logit ("e", "StringToPick: Quality has two decimal points\n");
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
	
	/* attempt to decode quality */
	p_struct->quality = atof(_str);


	/* Get last token */
	NextToken = strtok(NULL, " \n");
	strcpy( _str , NextToken );	

/*	logit ("e", "Polarity\n");*/

	/* Polarity:
	 * This should be a single alpha character or ? 
	 ************************************************************************************/

	/* check length */
	if ( strlen(_str) != 1 )
	{
		/* Polarity is only 1 char*/
		logit ("e", "StringToPick: Polarity is too long, length is %zu, token is <%s>\n",  strlen(_str), _str);
		return GLOBAL_MSG_FORMATERROR;
	}

	/* check to see that only valid characters are present */
	if (isalpha(_str[0]) == 0)
	{
		if (_str[0] != '?')
		{
			/* Polarity can only be a character or ?
			 ***************************************/
			logit ("e", "StringToPick: Polarity does not contain a valid character\n");
			return GLOBAL_MSG_FORMATERROR;
		}
	}

	/* attempt to decode polarity */
	p_struct->polarity = _str[0];

/*	logit ("e", "\tStringToPick: Done\n");*/
	/* DONE!!*/
	return GLOBAL_MSG_SUCCESS;
}
