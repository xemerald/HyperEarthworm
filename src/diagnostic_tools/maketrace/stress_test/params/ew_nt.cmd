@REM Create an Earthworm environment on Windows!

@REM This file should be run from the Command Prompt 
@REM to run or build an Earthworm system under Windows.

@ECHO OFF

@REM Set environment variables describing your Earthworm directory/version
@REM ---------------------------------------------------------------------
SET EW_HOME=C:\Users\baker\Desktop
SET EW_VERSION=earthworm-7.10-%EW_REV%
SET SYS_NAME=%COMPUTERNAME%

@REM Set environment variables used by Earthworm modules at run-time
@REM ---------------------------------------------------------------
SET EW_INSTALLATION=INST_UNKNOWN
SET EW_PARAMS=%EW_HOME%\stress_test\params
SET EW_LOG=%EW_HOME%\stress_test\logs\
SET EW_DATA_DIR=%EW_HOME%\stress_test\data\

@REM --------------------------
SET TZ=GMT

@REM Set the Path
@REM ------------------------------------------------------
SET Path=%EW_HOME%\%EW_VERSION%\bin;%Path%
