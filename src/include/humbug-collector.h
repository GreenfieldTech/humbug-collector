/*
 *   humbug-collector.h - The Humbug analytics collection agent for Asterisk(TM)
 *   Copyright (C) 2011, Humbug Analytics Labs
 *
 *   <http://www.humbuglabs.org> or <support@humbuglabs.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   Igor Ratgauzer <igor@humbuglabs.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "humbug-manager.h"
#include "humbug-avl.h"
#include "humbug-list.h"
#include "humbug-time.h"
#include "humbug-http.h"

#ifndef HUMBUGCOLLECTOR_H_
#define HUMBUGCOLLECTOR_H_

#define MY_VERSION   "0.8.3"
#define CONF_FILE    "/etc/humbug/humbug.conf"
#define WORK_DIR     "/usr/bin"
#define PID_FILE     "/var/run/humbug/humbug-collector.pid"
#define HUMBUG_LINK  "receiver.php"
#define HUMBUG_COMM  "check_community.php"
#define HUMBUG_CONFIG "config.php"

/* Define log message type */
#define DEBUG	4
#define INFO	3
#define WARNING	2
#define ERROR	1

/* Define parameters size */
#define CONF_STRING_LENGTH	128
#define ADDRESS_LENGTH		16
#define DIR_LENGTH			32
#define USERNAME_LENGTH		32
#define SECRET_LENGTH		128
#define MAX_APIKEY_LENGTH	128
#define URL_LENGTH			128
#define EVENT_NAME_LENGTH	16
#define EVENTS_LEN          16
#define MAX_KEY_LENGTH		512
#define MAX_REQUEST_DATA_LENGTH 2048
#define SEND_TIMEOUT        20

/* Log message */
char log_message[4096];

/* Gateway str */
char gateway_str[256];

/* Parameters from config file */
struct config_t {
	unsigned short port;
	char 	managerIp		[ADDRESS_LENGTH];
	char 	userName		[USERNAME_LENGTH];
	char 	secret			[SECRET_LENGTH];
	int  	humbugPort;
	char 	humbugHost		[URL_LENGTH];
	char 	humbugApikey	[MAX_APIKEY_LENGTH];
	char 	events			[EVENTS_LEN][EVENT_NAME_LENGTH];
	int  	eventsSize;
	int     reload_int;
	char 	humbugKey		[MAX_KEY_LENGTH];
	int  	encrypted;
	int     check_community;
	int		hangup;
	int  	debug_level;
	char 	logFileName		[1024];
	char 	configFileName		[1024];
	char	humbugScriptLink	[128];
	char	humbugScriptComm	[128];
	char	humbugScriptConfig	[128];
	char	humbugScriptPath	[1024];
	char	humbugPid		[1024];
	
	/* Trunks configs list */
	list_root_t *trunks_conf;
	struct trunk_t *conf_all;
} config;

/* Trunk config */
typedef struct trunk_t {
	char         name[64];
	char         local_pref[8];
	char         country_trunk[8];
	char         local_area_code[8];
	int          min_len;
	int          max_len;
	list_root_t *long_dist_pref;
	list_root_t *custom_pref;
	list_root_t *special_pref;
	avl_root_t  *blacklist;
	avl_root_t  *whitelist;
	hours_t      hours_list[7];
	timerange_t *black_timeranges;
} trunk_t;

/* Number data struct */
typedef struct num_data_t {
	char number[32];
	int  num_len;
} num_data_t;

/* Print debug info to console */
int debug;

/* Last reload time */
int reload_time;

#endif /* HUMBUGCOLLECTOR_H_ */

int    read_config(void);
void   create_time(char *);
int    get_timestamp(struct tm *tmp);
struct tm *get_event_time(void);
void   write_log(char *__message, int __level);
void   shutdown_graceful(void);
void   init_config(void);
void   check_event(event_t *__event_root);
void   format_time(char *my_time, struct tm *tmp);
int    get_config(void);
void   get_business_hours(void);
int    process_dial_event(event_t *__dial, struct tm *__event_time);
int    check_community_blacklist(char *__num);

avl_root_t *get_numbers_list(char *__link);
int numbers_cmp(void *__a, void *__b);
/* */
trunk_t *get_trunk_config(list_root_t *__trunks_list, char *__name);
/* */
void format_number(trunk_t *__trunk, char **__num);
/* */
void destroy_trunks_list(void);

void usage(void);
void version(void);
