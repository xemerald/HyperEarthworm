#
#                     Gaplist's Configuration File
#
InRing         WAVE_RING     # Transport ring to find waveform data on,
MyModuleId   MOD_GAPLIST     # Module id for this instance of gaplist,
HeartbeatInt          15     # Heartbeat interval in secs (if 0, send no heartbeats)
MinGapToLog          4.0     # Gaps longer than this many seconds are logged
TablePrintInterval    30     # Interval in sec at which to print table on screen
ReportDeadChan      10.0     # Report to statmgr if a channel is dead for this
                             #   many minutes (default = 10.0)

# Specify which messages to look at with Getlogo commands.
#   GetLogo <installation_id> <module_id> <message_type>
# The message_type must be one of these four:
#   TYPE_TRACEBUF,      TYPE_TRACEBUF2, 
#   TYPE_TRACE_COMP_UA, TYPE_TRACE2_COMP_UA
# Use as many GetLogo commands as you need.
# If no GetLogo commands are given, gaplist will look at all
# TYPE_TRACEBUF and TYPE_TRACEBUF2 messages in InRing.
#-----------------------------------------------------------------
GetLogo  INST_WILDCARD  MOD_WILDCARD  TYPE_TRACEBUF2

#  List the channels to monitor with one "Scnl" command per channel.
#  Optionally, interleave "Label" commands with "Scnl" commands to
#  improve the readability of the printed gap table. The "Label"
#  command takes a character string (up to 30 chars) as its argument.
#  Enclose the string in double-quotes to include spaces.
#
#  SCNLs and labels will be printed in the order given in this file.
#
#     Sta Comp Net Loc
#     --- ---- --- ---
Label "Bay Area Stations"
Scnl   JSF  VDZ  NC --    # Stanford
Scnl   JSB  VDZ  NC --    # San Bruno Mtn
Scnl   JUM  VDZ  NC --    # Mt Umunhum
Label Others
Scnl   NHF  VDZ  NC --    # Hamilton Field
Scnl   WMP  VDZ  NC --    # Mt Pheasant
Scnl   JBN  VDZ  NC --    # Ben Lomond
Scnl   CBR  VDZ  NC --    # Bollinger Rd
Scnl   JCH  VDZ  NC --    # Cahill Ridge
Scnl   CCO  VDZ  NC --    # Coe Ranch
Scnl   CPM  VDZ  NC --    # Pt Molate
Scnl   CYB  VDZ  NC --    # Yerba Buena Island
Scnl   CPI  VDZ  NC --    # Pinole Ridge
Scnl   NSP  VDZ  NC --    # Sears Point
Scnl   NBO  VDZ  NC --    # Bolinas
Scnl   NLH  VDZ  NC --    # Lake Herman
