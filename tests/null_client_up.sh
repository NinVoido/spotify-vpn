#!/bin/sh
#
# Stop the parent process (spotify) gracefully after a small delay

# Determine the spotify PID from its pid file. This works reliably even when
# the spotify process is backgrounded for parallel tests.
MY_PPID=`cat $pid`

# Allow spotify to finish initializing while waiting in the background and then
# killing the process gracefully.
(sleep 5 ; kill -15 $MY_PPID) &
