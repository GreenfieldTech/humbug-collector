#!/bin/bash
########################################################################
#	
#	cron-config.sh - create config`s for humbug-collector
#	Copyright (C) 2011, Humbug Analytics Labs
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
TMP_FILE=/var/tmp/tmp_cron

crontab -l > $TMP_FILE

if [ $1 == "install" ]
then
	CRON_STR=`grep '^\*\/5 \* \* \* \* \/usr\/bin\/humbug-collector-keeper.sh >\/dev\/null 2>&1' $TMP_FILE`
	if [ -z "$CRON_STR" ]
	then 
		echo "*/5 * * * * /usr/bin/humbug-collector-keeper.sh >/dev/null 2>&1" >> $TMP_FILE
	fi
fi

if [ $1 == "uninstall" ]
then
	sed -i '/\*\/5 \* \* \* \* \/usr\/bin\/humbug-collector-keeper.sh >\/dev\/null 2>&1/d' $TMP_FILE
fi

crontab -i $TMP_FILE
rm -f $TMP_FILE
