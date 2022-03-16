# Create an Earthworm environment on Linux!

# This file should be sourced by a Bourne shell wanting 
# to run or build an Earthworm system under Linux.

# Set environment variables describing your Earthworm directory/version
export EW_HOME="/home/baker/Software/Earthworm"
export EW_VERSION=earthworm-7.10-${EW_REV}
export SYS_NAME=`hostname`

# Set environment variables used by Earthworm modules at run-time
# Path names must end with the slash "/"
export EW_INSTALLATION=INST_UNKNOWN
export EW_PARAMS="${EW_HOME}/stress_test/params/"
export EW_LOG="${EW_HOME}/stress_test/logs/"
export EW_DATA_DIR="${EW_HOME}/stress_test/data/"

# Set the PATH
export PATH="${EW_HOME}/$EW_VERSION/bin:${PATH}"
