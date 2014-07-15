################################################################################
#
# humbug-collector - The Humbug Analytical Suite agent
# 
# Top level Makefile
#
# Copyright (C) 2011-2012, Humbug Telecom Labs, Ltd.
#
# Igor Ratgauzer <igor@humbuglabs.org>
#
# This program is free software, distributed under the terms of
# the GNU General Public License
#
################################################################################

# Do not modify these lines, these control where files are loated
VERSION=0.8.0
SHELL=/bin/bash
HBC_CONFPATH=/etc/humbug
HBC_CONFPATH_UNINSTALLED=/etc/humbug._uninstalled
HBC_BINPATH=/usr/bin
HBC_PIDPATH=/var/run/humbug
HBC_LOGPATH=/var/log/humbug
FIND=/usr/bin/find
OS_TYPE=`uname -o`

HBC_OWNER=root
HBC_GROUP=root

# Compiler options
CC=gcc
CCFLAGS:=-Wall -D_REENTRANT -fPIC
JANSSON:=src/jansson/*.c -I src/jansson/

helpscreen:
	@echo "+-----------------------------------------------------+"
	@echo "| humbug-collector, v.$(VERSION) (c) 2012 Humbug Labs Ltd. |"
	@echo "+-----------------------------------------------------+"
	@echo "| Usage instructions:                                 |"
	@echo "| -------------------                                 |"
	@echo "| make installdirs - Create the directories           |"
	@echo "| make all         - Compile the code                 |"
	@echo "| make install     - Create, compile and install      |"
	@echo "| make clean       - Clean binaris from directory     |"
	@echo "| make config      - Install as service               |"
	@echo "| make uninstall   - Uninstalled humbug-collector :-( |"
	@echo "+-----------------------------------------------------+"

.PHONY: installdirs
installdirs:
	@if [ -d $(HBC_CONFPATH_UNINSTALLED) ]; then	\
		mv $(HBC_CONFPATH_UNINSTALLED) $(HBC_CONFPATH);	\
	fi;	\

	@if [ ! -d $(HBC_CONFPATH) ]; then	\
		mkdir -p $(HBC_CONFPATH);	\
	fi;	\

	@if [ ! -d $(HBC_PIDPATH) ]; then	\
		mkdir -p $(HBC_PIDPATH);	\
	fi;	\
	
	@if [ ! -d $(HBC_LOGPATH) ]; then	\
		mkdir -p $(HBC_LOGPATH);	\
	fi;	\

	@if [ -z "$(shell $(FIND) $(HBC_CONFPATH) -user $(HBC_OWNER) -group $(HBC_GROUP) -type d)" ]; then	\
		chown $(HBC_OWNER).$(HBC_GROUP) -R $(HBC_CONFPATH);	\
	fi;	\

	@if [ -z "$(shell $(FIND) $(HBC_PIDPATH) -user $(HBC_OWNER) -group $(HBC_GROUP) -type d)" ]; then	\
		chown $(HBC_OWNER).$(HBC_GROUP) -R $(HBC_PIDPATH);	\
	fi;	\
	
	@if [ -z "$(shell $(FIND) $(HBC_LOGPATH) -user $(HBC_OWNER) -group $(HBC_GROUP) -type d)" ]; then	\
		chown $(HBC_OWNER).$(HBC_GROUP) -R $(HBC_LOGPATH);	\
	fi;	\

	@if [ ! -x $(HBC_CONFPATH) ]; then	\
		chmod 755 $(HBC_CONFPATH);	\
	fi;	\
	
	@if [ ! -x $(HBC_PIDPATH) ]; then	\
		chmod 755 $(HBC_PIDPATH);	\
	fi;	\
	
	@if [ ! -x $(HBC_LOGPATH) ]; then	\
		chmod 755 $(HBC_LOGPATH);	\
	fi;	\

.PHONY: install
install: installdirs allprogs
	@if [ -a $(HBC_CONFPATH)/humbug.conf ]; then 	\
		cp humbug.conf $(HBC_CONFPATH)/humbug.conf.new;	\
	else	\
		cp humbug.conf $(HBC_CONFPATH);	\
	fi;	\
	
	cp humbug-collector $(HBC_BINPATH)
	cp humbug-setup $(HBC_BINPATH)
	cp humbug-config.sh $(HBC_BINPATH)
	chmod +x $(HBC_BINPATH)/humbug-collector
	chmod +x $(HBC_BINPATH)/humbug-setup
	chmod +x $(HBC_BINPATH)/humbug-config.sh
	chmod 644 $(HBC_CONFPATH)/humbug.conf
	
	@echo
	@echo "+-----------------------------------------------------+"
	@echo "| humbug-collector, v.$(VERSION) (c) 2012 Humbug Labs Ltd. |"
	@echo "+-----------------------------------------------------+"
	@echo "|       To start the Humbug Collector Agent           |"
	@echo "|         as a service dont forget to run             |"
	@echo "|              the following command:                 |"
	@echo "|                                                     |"
	@echo "|                                                     |"
	@echo "|                    make config                      |"
	@echo "+-----------------------------------------------------+"

.PHONY: uninstall
uninstall: clean
	@if [ "$(OS_TYPE)" = "GNU/Linux" ]; then \
		if [ -f $(HBC_PIDPATH)/humbug-collector.pid ]; then \
			kill `cat $(HBC_PIDPATH)/humbug-collector.pid`; \
		fi; \
		if [ -f /etc/redhat-release -o -f /etc/fedora-release ]; then \
			/sbin/chkconfig --del humbug-collector; \
		elif [ -f /etc/debian_version -o -f /etc/lsb-release ] ; then \
			/usr/sbin/update-rc.d -f humbug-collector remove; \
		fi; \
		rm -f /etc/init.d/humbug-collector; \
	fi;
	mv $(HBC_CONFPATH) $(HBC_CONFPATH)._uninstalled
	rm -f $(HBC_BINPATH)/humbug-collector
	rm -f $(HBC_BINPATH)/humbug-setup
	rm -rf $(HBC_TMPPATH)
	@echo "The HumBug data collector agent had been successfuly un-installed"

.PHONY: all
all: allprogs

allprogs:
	$(CC) $(CCFLAGS) $(JANSSON) -c src/humbug-common.c
	$(CC) $(CCFLAGS) -c src/humbug-avl.c
	$(CC) $(CCFLAGS) -c src/humbug-list.c
	$(CC) $(CCFLAGS) -c src/humbug-http.c
	$(CC) $(CCFLAGS) -c src/humbug-manager.c
	$(CC) $(CCFLAGS) -c src/humbug-time.c
	$(CC) $(CCFLAGS) -c src/humbug-collector.c
	$(CC) src/humbug-setup.c -o humbug-setup -lnewt

	$(CC) $(CCFLAGS) $(JANSSON) humbug-collector.o humbug-common.o humbug-avl.o humbug-http.o humbug-manager.o humbug-time.o humbug-list.o -o humbug-collector -lm

.PHONY: installall
installall: installdirs allprogs install

humbug-collector: humbug-collector.o humbug-common.o humbug-avl.o humbug-time.o humbug-http.o humbug-manager.o humbug-time.o humbug-list.o
	$(CC) $(CCFLAGS) $(JANSSON) humbug-collector.o humbug-common.o \ 
	humbug-numbers-list.o humbug-time.o -o humbug-collector -lm

humbug-common.o: src/include/humbug-collector.h src/humbug-common.c
	$(CC) $(CCFLAGS) $(JANSSON) -c src/humbug-common.c
	
humbug-avl.o: src/include/humbug-collector.h src/include/humbug-avl.h src/humbug-avl.c
	$(CC) $(CCFLAGS) -c src/humbug-avl.c

humbug-list.o: src/include/humbug-collector.h src/include/humbug-list.h src/humbug-list.c
	$(CC) $(CCFLAGS) -c src/humbug-list.c

humbug-http.o: src/include/humbug-collector.h src/include/humbug-http.h src/humbug-http.c
	$(CC) $(CCFLAGS) -c src/humbug-http.c

humbug-manager.o: src/include/humbug-collector.h src/include/humbug-manager.h src/humbug-manager.c
	$(CC) $(CCFLAGS) -c src/humbug-manager.c
	
humbug-time.o: src/include/humbug-collector.h src/include/humbug-time.h src/humbug-time.c
	$(CC) $(CCFLAGS) -c src/humbug-time.c

humbug-collector.o: src/include/humbug-collector.h src/humbug-collector.c
	$(CC) $(CCFLAGS) -c src/humbug-collector.c

.PHONY: distclean
distclean: clean

clean:
	rm -f humbug-collector *.o humbug-setup

config:
	@if [ "$(OS_TYPE)" = "GNU/Linux" ]; then \
		if [ -f /etc/redhat-release -o -f /etc/fedora-release ]; then \
			cp init.d/rc.redhat.humbug /etc/init.d/humbug-collector; \
			chmod +x /etc/init.d/humbug-collector; \
			/sbin/chkconfig --add humbug-collector; \
		elif [ -f /etc/debian_version -o -f /etc/lsb-release ] ; then \
			cp init.d/rc.debian.humbug /etc/init.d/humbug-collector; \
			chmod +x /etc/init.d/humbug-collector; \
			/usr/sbin/update-rc.d humbug-collector defaults 99 50; \
		fi; \
	fi; \
	
	@echo
	@echo "+-----------------------------------------------------+"
	@echo "| humbug-collector, v.$(VERSION) (c) 2012 Humbug Labs Ltd. |"
	@echo "+-----------------------------------------------------+"
	@echo "|  To start the Humbug Agent now, edit the config:    |"
	@echo "|             /etc/humbug/humbug.conf                 |"
	@echo "|                                                     |"
	@echo "|                  and then run                       |"
	@echo "|        /etc/init.d/humbug-collector start           |"
	@echo "|                                                     |"
	@echo "+-----------------------------------------------------+"
