#!/bin/bash
# vim:textwidth=80:tabstop=4:shiftwidth=4:smartindent:autoindent
########################################################################
#	
#	humbug-collector-keeper.sh 
#	Copyright (C) 2011-2012, Humbug Analytics Labs
#
#	http://www.humbuglabs.org or support@humbuglabs.org
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#       
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#       
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#   MA 02110-1301, USA.
#
#	Igor Ratgauzer <igor@humbuglabs.org>
#
########################################################################

MY_PATH=/usr/sbin/humbug-collector-keeper.sh
BIN_PATH=/usr/bin
COLLECTOR=humbug-collector
CONFIG_FILE=/etc/humbug/humbug.conf
PID_FILE=/var/run/humbug/humbug-collector.pid
LOG_FILE=`grep 'log_file' $CONFIG_FILE | sed -e 's/^\s*log_file\s*=\s*//g'`

# Write message to log file
log_message() {
	if [ -n "$LOG_FILE" ] 
	then
		DATE=`date "+%Y-%m-%d %H:%M:%S"`
		echo $DATE KEEPER     $1 >> $LOG_FILE
	fi
}

# Starts new 'collector' process
start_collector() {
	/etc/init.d/$COLLECTOR start > /dev/null 2>&1
}

# if PID file exists
if [ -e $PID_FILE ]
then
	# get PID from file
	PID_F=`cat $PID_FILE`
	# get PID from system
	PID_S=`ps aux | grep $BIN_PATH/$COLLECTOR | awk '{ print $8$2 }' | grep Ss | sed -e 's/^Ss//g'`
	if [ -n "$PID_S" ]
	then
		# exit if PID_F eq PID_S
		if [ $PID_F -eq $PID_S ]
		then
			# OK
			log_message "'collector' looks good"
			exit 0
		fi
	else
		# PID file exists, but 'collector' process not found
		# remove old PID file
		rm -f $PID_FILE
		# start new 'collector' process
		log_message "PID file found, but process is dead"
		log_message "Try to start new process"
		start_collector
	fi
else
	log_message "PID file is not exists"
	# Check if 'collector' turned into a zombie
	PID_Z=`ps aux | grep $BIN_PATH/$COLLECTOR | awk '{ print $8$2 }' | grep Z | sed -e 's/^Z//g'`
	if [ -n "$PID_Z" ]
	then
		log_message "'collector' is zombie!!!"
		log_message "Try to kill"
		kill -9 $PID_Z
	fi
	log_message "Try to start new process"
	start_collector
fi
exit 0
