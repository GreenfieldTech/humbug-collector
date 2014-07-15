#!/bin/bash
########################################################################
#	
#	humbug-config.sh - create config`s for humbug-collector
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

MY_USER=root
MY_GROUP=root

# Conffig files
HUMBUG_CONF_FILE=/etc/humbug/humbug.conf
MANAGER_HUMBUG_FILE=/etc/asterisk/manager_humbug.conf
CDR_CONF_FILE=/etc/asterisk/cdr.conf
CDR_CONF_FILE_TMP=/etc/asterisk/cdr.conf.tmp
CDR_MANAGER_CONF_FILE=/etc/asterisk/cdr_manager.conf
MANAGER_CONFIG=/etc/asterisk/manager_custom.conf
LOG_ROTATE_CONFIG=/etc/logrotate.d/humbug-collector

# Password length
l=16
PASS=`tr -dc A-Za-z0-9 < /dev/urandom | head -c ${l}`

# Write humbug config file
if [ ! -e $HUMBUG_CONF_FILE ]
then
	echo "# humbug.conf - config file for Humbug Analytical Suite agent" > $HUMBUG_CONF_FILE
	echo "# version 0.5.0" >> $HUMBUG_CONF_FILE
	echo "#" >> $HUMBUG_CONF_FILE
	echo "# automatic generated" >> $HUMBUG_CONF_FILE
	echo " " >> $HUMBUG_CONF_FILE
	echo "# Connection to Manager" >> $HUMBUG_CONF_FILE
	echo "address = 127.0.0.1" >> $HUMBUG_CONF_FILE
	echo "port = 5038" >> $HUMBUG_CONF_FILE
	echo "user = humbug" >> $HUMBUG_CONF_FILE
	echo -n "secret = " >> $HUMBUG_CONF_FILE 
	echo $PASS >> $HUMBUG_CONF_FILE
	echo " " >> $HUMBUG_CONF_FILE
	echo "# Humbug service connection" >> $HUMBUG_CONF_FILE
	echo "h_apikey = " >> $HUMBUG_CONF_FILE
	echo "h_host = api.humbuglabs.org" >> $HUMBUG_CONF_FILE
	echo "h_port = 80" >> $HUMBUG_CONF_FILE
	echo " " >> $HUMBUG_CONF_FILE
	echo "# Config parameters" >> $HUMBUG_CONF_FILE
	echo "h_read = cdr, reload, shutdown, alarm" >> $HUMBUG_CONF_FILE
	echo "encrypted = no" >> $HUMBUG_CONF_FILE
	echo "community_blacklist = no" >> $HUMBUG_CONF_FILE
	echo "action_hangup = no" >> $HUMBUG_CONF_FILE
	echo "h_key = " >> $HUMBUG_CONF_FILE
	echo " " >> $HUMBUG_CONF_FILE
	echo "log_file = /var/log/humbug/humbug-collector.log" >> $HUMBUG_CONF_FILE
	echo "debug_level = 3" >> $HUMBUG_CONF_FILE
	chown "$MY_USER":"$MY_GROUP" "$HUMBUG_CONF_FILE"
fi

# Write manager config file
if [ ! -e $MANAGER_HUMBUG_FILE ]
then
	echo "[humbug]" > $MANAGER_HUMBUG_FILE
	echo -n "secret = " >> $MANAGER_HUMBUG_FILE
	echo $PASS >> $MANAGER_HUMBUG_FILE
	echo "deny = 0.0.0.0/0.0.0.0" >> $MANAGER_HUMBUG_FILE
	echo "permit = 127.0.0.1/255.255.255.0" >> $MANAGER_HUMBUG_FILE
	echo "read = cdr,system,call" >> $MANAGER_HUMBUG_FILE
	echo "write = call,all" >> $MANAGER_HUMBUG_FILE
	chown "$MY_USER":"$MY_GROUP" "$MANAGER_HUMBUG_FILE"
fi

# Write cdr config file
if [ -e "$CDR_CONF_FILE" ]
then
	sed 's/^;.*enable.*=.*yes/enable = yes/' "$CDR_CONF_FILE" > "$CDR_CONF_FILE_TMP"
	mv "$CDR_CONF_FILE_TMP" "$CDR_CONF_FILE"
else
	echo "[general]" > $CDR_CONF_FILE
	echo "enable = yes" >> $CDR_CONF_FILE
fi
chown "$MY_USER":"$MY_GROUP" "$CDR_CONF_FILE"

# Write CDR manager config file
echo "[general]" > $CDR_MANAGER_CONF_FILE
echo "enabled = yes" >> $CDR_MANAGER_CONF_FILE
chown "$MY_USER":"$MY_GROUP" "$CDR_MANAGER_CONF_FILE"

MANAGER_STR=`grep -i '#include manager_humbug.conf' $MANAGER_CONFIG`
if [ -z "$MANAGER_STR" ]
then 
	echo " " >> $MANAGER_CONFIG
	echo "; Added by humbug-collector" >> $MANAGER_CONFIG
	echo "#include manager_humbug.conf" >> $MANAGER_CONFIG
fi
	
# Logrotate /etc/logrotate.d/humbug-collector
echo "/var/log/humbug/humbug-collector.log {" >> $LOG_ROTATE_CONFIG
echo "    daily" >> $LOG_ROTATE_CONFIG
echo "    rotate 3" >> $LOG_ROTATE_CONFIG
echo "    size 100k" >> $LOG_ROTATE_CONFIG
echo "    postrotate" >> $LOG_ROTATE_CONFIG
echo "        kill -HUP \`cat /var/run/humbug/humbug-collector.pid\`" >> $LOG_ROTATE_CONFIG
echo "    endscript" >> $LOG_ROTATE_CONFIG
echo "}" >> $LOG_ROTATE_CONFIG

