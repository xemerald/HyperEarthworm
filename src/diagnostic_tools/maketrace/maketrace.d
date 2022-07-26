#
#                    Configuration file for maketrace
#                    --------------------------------
#
#          Produce dummy trace data messages. Cloned from adsend.
#

ModuleId         MOD_IMPORT_GENERIC  # Module id of this instance of maketrace
OutRing          WAVE_RING      # Transport ring to write waveforms to
HeartbeatInt     15             # Heartbeat interval in seconds

# Required parameters
ChanRate         100.0          # Sampling rate in samples/second
ChanMsgSize      100            # TraceBuf message size, in samples
nChan            500            # Number of channels to produce

# The SCNL location code is incremented for each channel.  The default SCNL
# template is <STA.CHA.NN.00>.  The channels will have location codes 00, 01,
# etc., up to 99.  By defaut, nChan cannot be more than 100.  You can supply
# your own SCNL template.  If the last character is a digit, the location
# codes will be digits.  If the last character is upper case alpha, the
# location codes will be upper case alpha.  The sequence will "carry" into
# the next character, as long as the template for that character is from the
# same character set.  If you specify AA for the template SCNL location code,
# nChan can be up to 676 (26^2).  Use -- for the blank location code.  For
# that case, nChan must be 1.

# Optional parameters
#SCNL             STA CHA NN --  # SCNL template, default=<STA.CHA.NN.00>
#StartTime        1906-04-18T13:12:27 # Start time of first packet in
                                 # FDSN time format YYYY-MM-DDTHH:MM:SS,
                                 # default is current UTC time
                                 # StartTime is read, but ignored, on Windows
#Interval         10             # Interval between data rate statistics,
                                 #   in seconds, 0=none, default=10
#Debug            0              # 0=none, >0 yes, >1 more, default=0
