/*       usbrhead -    Structure for USBR-style UW1 demultiplexed files. */
struct usbrhead {
	short nchan;	/* Number of demultiplexed channels */
	int lrate;	/* Sampling rate in sample per 1000 seconds */
	int lmin;	/* Reference time for Carl's date routine (grgmin) */
	int lsec;	/* Reference second from above min in microseconds */
	int length;	/* Number of samples per channel in total record */
	int tapenum;	/* Original 11/34 tape number - short in true UW-1 */
	int eventnum;	/* Event number on original tape - short in UW-1 */
	short flg[10];	/* Extra flags for expansion.  

			flg[0] is -3 when lmin is not set but the digitization
			       rate and seconds modulo 10 have been set.
			flg[0] is -2 when lmin and lsec have not been set.
			flg[0] is -1 when lmin is set to within plus or minus 3.
			flg[0] is what you get from the online demultiplexer.
			       it usually means  that the time is set
			       to within 3 seconds and the digrate is good
			       to about 3 places, but this may be in error:
				a) if the online clock was not set at reboot 
			           time then the year will not be 198?. 
				b) if the digrate is exactly 100 in old 64 chan 
				   stuff then this is not even close.
				   The digrate was constant throughout the
				   64-chan configuration.
			flg[0] is  1 when ping is through with it.  This 
			   	generally means the time is set to within
				.2 second and the digrate has not been reset.
			flg[0] is  2  when the digrate has been set to 4
				significant figures and the starting time
				has been set to within one sampling interval.
			flg[1] is the logical OR of:
				low bit: 0 -> Squash Lock On
				low bit: 1 -> Squash Lock Off
				2nd bit: 0 -> Not Squashed
				2nd bit: 1 -> Squashed

			flg[2] is number of times the file has been merged.
			flg[3] is 0 usually and 1 if the station names have
				been modified.
			flg[4] is decimation factor if slashed
			flg[5] is the channel number (1 - hm.nchan) of the
				time code from which the time was set.
			flg[6] is used by the 5-day merge package.  It is
				set to 1 after the station names have been
				corrected.
			flg[7] is used by 'stack' to tell how many files have
				been stacked onto this one.

			*/
	char extra[10];	/* codes for extra structures for data added later*/
	char comment[80]; /* 80 optional comment characters */
};
