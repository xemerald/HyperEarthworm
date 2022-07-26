#
#                  wsv_test's Configuration File
#
MyModuleId    MOD_WSV_TEST  # module id for this program,
RingName        HYPO_RING   # transport ring to write to,
LogFile                 1   # If 0, don't write logfile at all,
LogAllTests             1   # If 0, only log when waveserver dies or comes alive,
                            # Otherwise, log all waveserver tests.
HeartBeatInterval      30   # Send heartbeats to statmgr this often (sec)
TestInterval          300   # Test all waveservers this often (sec)
wsTimeout           10000   # Msec to wait for reply from waveserver

#
# Test the following waveservers:

#           IP Address    Port   Host Note
#           ------------  -----  ---- -----
Waveserver  130.118.43.3  16000  wsv1 dummy  
Waveserver  130.118.43.3  16032  wsv1 ad2
Waveserver  130.118.43.3  16033  wsv1 ad3
Waveserver  130.118.43.3  16025  wsv1 nano
Waveserver  130.118.43.3  16026  wsv1 dst
Waveserver  130.118.43.3  16027  wsv1 bdsn
Waveserver  130.118.43.4  16031  wsv2 ad1
Waveserver  130.118.43.4  16032  wsv2 ad2
Waveserver  130.118.43.4  16033  wsv2 ad3
Waveserver  130.118.43.4  16034  wsv2 ad4
Waveserver  130.118.43.4  16035  wsv2 ad5
Waveserver  130.118.43.4  16036  wsv2 ad6
Waveserver  130.118.43.4  16037  wsv2 ad7
Waveserver  130.118.43.4  16038  wsv2 ad8
Waveserver  130.118.43.4  16039  wsv2 ad9
Waveserver  130.118.43.4  16025  wsv2 nano
Waveserver  130.118.43.4  16026  wsv2 dst
Waveserver  130.118.43.4  16027  wsv2 bdsn

