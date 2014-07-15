/*
 *   humbug-setup.c - The Humbug TUI setup for Asterisk(TM)
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

#define HBG_CONF_FILE "/etc/humbug/humbug.conf"
#define HBG_LOG_FILE  "/var/log/humbug/humbug-collector.log"
#define EVENTS_SIZE 7

#include <newt.h>
#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <time.h>

typedef struct parameters {
	char 	astIpVal		[16];
	char	astPortVal		[8];
	char 	astUserVal		[32];
	char 	astPassVal		[64];
	char  	humbugPort		[8];
	char 	humbugHost		[256];
	char 	humbugApikey	[512];
	char 	events			[EVENTS_SIZE][20];
	int  	eventsSize;
	char 	humbugKey		[512];
	int  	encrypted;
	int  	debug_level;
	char 	log_file		[256];
} parameters;
parameters *params;

void show_help(void){
	newtWinMessage("Help", "Return", "Manager IP     - Asterisk Manager host IP\nManager Port   - Asterisk Manager port\nManager User   - Asterisk Manger user, default: humbug\nManager Pass   - Password for the above manager user\nHumbug Host    - default: api.humbuglabs.org\nHumbug Port    - default: 80\nHumbug API key - API key provided to you by Humbug\nEncryption Key - Encryption key provided to you by Humbug\nEvents to analyze - Events to Analyze, default: CDR, Shutdown,Reload\n\nWhen enabled, log file saves to: /var/log/humbug/humbug-collector.log");
}

int readHumbugConfig(void);
int saveHumbugConfig(void);

int main(void){
	newtComponent hForm, button1, button2;
	newtComponent astIpLabel, astPortLabel, astUserLabel, astPassLabel;
	newtComponent astIpEntry, astPortEntry, astUserEntry, astPassEntry;
	const char *astIpVal, *astPortVal, *astUserVal, *astPassVal;

	newtComponent humbugHostLabel, humbugPortLabel, humbugApiLabel;
	newtComponent humbugHostEntry, humbugPortEntry, humbugApiEntry;
	const char *humbugHostVal, *humbugPortVal, *humbugApiVal;

	int i, y;

	newtComponent eventsLabel, eventsChk[EVENTS_SIZE], chkList;
	newtComponent encChk, encLabel, encEntry, chkLog;
	char encVal, logVal;
	const char *encKey;
	char eventsList[EVENTS_SIZE][16];
	char eventsResults[EVENTS_SIZE];
	strcpy(eventsList[0], "CDR");
	strcpy(eventsList[1], "NewExten");
	strcpy(eventsList[2], "Reload");
	strcpy(eventsList[3], "Shutdown");
	strcpy(eventsList[4], "Alarm");
	strcpy(eventsList[5], "AlarmClear");
	strcpy(eventsList[6], "CollectorAlarm");

	struct newtExitStruct es;

	params = (parameters*) calloc(1, sizeof(parameters));

	if(readHumbugConfig() < 0) {
		exit(EXIT_FAILURE);
	}

	newtInit();
	newtCls();

	newtDrawRootText(1, 0, "Text Mode Setup for humbug-collector  (c) 2011 Humbug Telecom Labs, Ltd.");
	newtPushHelpLine(" <Tab>/<Alt-Tab> between elements | <ESC> exits without save | <F12> for help");

	newtCenteredWindow(66, 17, "Humbug Setup");

	astIpLabel   = newtLabel(1, 1, "      Manager IP:");
	astIpEntry   = newtEntry(19, 1, params->astIpVal, 20, &astIpVal, 0);
	astPortLabel = newtLabel(1, 2, "    Manager Port:");
	astPortEntry = newtEntry(19, 2, params->astPortVal, 20, &astPortVal, 0);
	astUserLabel = newtLabel(1, 3, "Manager Username:");
	astUserEntry = newtEntry(19, 3, params->astUserVal, 20, &astUserVal, 0);
	astPassLabel = newtLabel(1, 4, "Manager Password:");
	astPassEntry = newtEntry(19, 4, params->astPassVal, 20, &astPassVal, 0); //NEWT_FLAG_PASSWORD);

	humbugHostLabel = newtLabel(1, 6, "     Humbug Host:");
	humbugHostEntry = newtEntry(19, 6, params->humbugHost, 20, &humbugHostVal, 0);
	humbugPortLabel = newtLabel(1, 7, "     Humbug Port:");
	humbugPortEntry = newtEntry(19, 7, params->humbugPort, 20, &humbugPortVal, 0);
	humbugApiLabel  = newtLabel(1, 8, "  Humbug API key:");
	humbugApiEntry  = newtEntry(19, 8, params->humbugApikey, 20, &humbugApiVal, NEWT_FLAG_SCROLL);

	if(1 == params->encrypted){
		encChk = newtCheckbox(19, 10, "Encrypted", 'X', " X", &encVal);
	}else{
		encChk = newtCheckbox(19, 10, "Encrypted", ' ', " X", &encVal);
	}
	encLabel = newtLabel(1, 11, "  Encryption key:");
	encEntry = newtEntry(19, 11, params->humbugKey, 42, &encKey, NEWT_FLAG_SCROLL);

	if(strlen(params->log_file) > 0 ){
		chkLog = newtCheckbox(19, 13, "Log ON/OFF", 'X', " X", &logVal);
	}else{
		chkLog = newtCheckbox(19, 13, "Log ON/OFF", ' ', " X", &logVal);
	}

	eventsLabel  = newtLabel(42, 1, "Events for analyze:");

	button1 = newtButton(36, 13, "Save & Exit");
	button2 = newtButton(53, 13, "Quit");

	chkList = newtForm(NULL, NULL, 0);
	newtFormSetBackground(chkList, NEWT_COLORSET_CHECKBOX);

	for(i=0; i<EVENTS_SIZE; i++) {
		int selected = 0;
		for(y=0; y<params->eventsSize; y++){
			if(0 == strcasecmp(eventsList[i], params->events[y])){
				selected = 1;
				break;
			}
		}
		if (1 == selected) {
			eventsChk[i] = newtCheckbox(47, 2 + i, eventsList[i], 'X', " X", &eventsResults[i]);
		} else {
			eventsChk[i] = newtCheckbox(47, 2 + i, eventsList[i], ' ', " X", &eventsResults[i]);
		}
		newtFormAddComponent(chkList, eventsChk[i]);
	}

	hForm = newtForm(NULL, NULL, 0);

	newtFormAddComponents(hForm, button1, button2,
			astIpLabel, astPortLabel, astUserLabel, astPassLabel,
			astIpEntry, astPortEntry, astUserEntry, astPassEntry,
			humbugHostLabel, humbugPortLabel, humbugApiLabel,
			humbugHostEntry, humbugPortEntry, humbugApiEntry,
			eventsLabel, chkList, encChk, encLabel, encEntry, chkLog,
			NULL);

	for(;;){
		do{
			newtFormRun(hForm, &es);
		}while(es.reason == NEWT_EXIT_TIMER);
		if (es.reason == NEWT_EXIT_HOTKEY) {
			int done = 0;
			switch (es.u.key) {
				case NEWT_KEY_ESCAPE:
					done = 1;
					break;
				case NEWT_KEY_F12:
					show_help();
					break;
			}
			if(done)
				break;
		}else if (es.reason == NEWT_EXIT_COMPONENT) {
			if(es.u.co == button1){
				strcpy(params->astIpVal, astIpVal);
				strcpy(params->astPortVal, astPortVal);
				strcpy(params->astUserVal, astUserVal);
				strcpy(params->astPassVal, astPassVal);
				strcpy(params->humbugApikey, humbugApiVal);

				params->eventsSize = 0;
				for(i=0; i<EVENTS_SIZE; i++) {
					if(eventsResults[i] == 'X') {
						strcpy(params->events[params->eventsSize], eventsList[i]);
						params->eventsSize++;
					}
				}
				params->encrypted = (encVal == 'X') ? 1 : 0;
				strcpy(params->humbugKey, encKey);
				strcpy(params->log_file, HBG_LOG_FILE);
				saveHumbugConfig();
			}
			break;
		}
	}
	newtPopWindow();
	newtPopHelpLine();
	newtFinished();
	newtFormDestroy(hForm);
	free(params);
	return(EXIT_SUCCESS);
}

//******************************************************************************
/**/
int saveHumbugConfig( void ){
	FILE *file;
	int i;
	char events_buf[512] = "\0";
	file = fopen(HBG_CONF_FILE, "w");
	fprintf(file, "# humbug.conf - config file for Humbug Analytical Suite agent\n");
	fprintf(file, "# automatically generated\n\n");
	fprintf(file, "# Astersk manager connection\n");
	fprintf(file, "address = %s\n", params->astIpVal);
	fprintf(file, "port = %s\n", params->astPortVal);
	fprintf(file, "user = %s\n", params->astUserVal);
	fprintf(file, "secret = %s\n\n", params->astPassVal);
	fprintf(file, "# Humbug service connection\n");
	fprintf(file, "h_apikey = %s\n", params->humbugApikey);
	fprintf(file, "h_host = %s\n", params->humbugHost);
	fprintf(file, "h_port = %s\n\n", params->humbugPort);
	fprintf(file, "# Config parameters\n");

	for(i=0; i<params->eventsSize; i++){
		strcat(events_buf, params->events[i]);
		strcat(events_buf, ", ");
	}
	events_buf[strlen(events_buf)-2] = '\0';
	fprintf(file, "h_read = %s\n", events_buf);
	if(params->encrypted == 1){
		fprintf(file, "encrypted = yes\n");
		fprintf(file, "h_key = %s\n\n", params->humbugKey);
	}else{
		fprintf(file, "encrypted = no\n");
		fprintf(file, "h_key = \n");
	}
	fprintf(file, "log_file = %s\n", params->log_file);
	fprintf(file, "debug_level = %s\n", "3");

	fclose(file);
	return(0);
}

//******************************************************************************
/**/
int readHumbugConfig( void ){
	FILE *file;
	char conf_buf[512];

	file = fopen(HBG_CONF_FILE, "r");

	if (NULL == file) {
		fprintf( stdout, "Can`t open config file: %s\n", strerror(errno));
		return -1;
	}

	while (NULL != fgets(conf_buf, 512, file)) {

		if (('#' == *conf_buf) || ('=' == *conf_buf) || ('\n' == *conf_buf)
				|| (';' == *conf_buf) || ('\r' == *conf_buf)) {
			continue;
		}

		if ('\n' == conf_buf[strlen(conf_buf) - 1]) {
			conf_buf[strlen(conf_buf) - 1] = '\0';
		}

		if (strlen(conf_buf) <=2  || '=' == conf_buf[strlen(conf_buf) - 1] || NULL == strchr(conf_buf, '=')) {
			continue;
		}

		char *variable, value[256], *tmp_value, *p;

		variable = strtok(conf_buf, "=");
		tmp_value = strtok(NULL, "=");

		p = value;
		while (*tmp_value) {
			if (*tmp_value == ' ' || *tmp_value == '\t') {
				tmp_value++;
			} else {
				*p++ = *tmp_value++;
			}
		}
		*p = 0;

		if (0 == strncmp("port", variable, 4)) {
			strcpy(params->astPortVal, value);
		} else if (0 == strncmp("address", variable, 7)) {
			strcpy(params->astIpVal, value);
		} else if (0 == strncmp("user", variable, 4)) {
			strcpy(params->astUserVal, value);
		} else if (0 == strncmp("secret", variable, 6)) {
			strcpy(params->astPassVal, value);
		}else if (0 == strncmp("h_port", variable, 6)) {
			strcpy(params->humbugPort, value);
		} else if (0 == strncmp("h_host", variable, 6)) {
			strcpy(params->humbugHost, value);
		}else if (0 == strncmp("h_apikey", variable, 8)) {
			strcpy(params->humbugApikey, value);
		}else if (0 == strncmp("h_read", variable, 6)) {
			char *pEvent;
			params->eventsSize = 0;
			if ( NULL != (pEvent = strtok(value, ",")) ) {
				strcpy( params->events[params->eventsSize++], pEvent );
				while ( NULL != ( pEvent = strtok(NULL, ",")) ) {
					strcpy( params->events[params->eventsSize++], pEvent );
				}
			}
		}else if (0 == strncmp("encrypted", variable, 9)) {
			if (0 == strncasecmp("yes", value, 3)) {
				params->encrypted = 1;
			} else {
				params->encrypted = 0;
			}
		} else if (0 == strncmp("h_key", variable, 5)) {
			strcpy(params->humbugKey, value);
		}else if ( 0 == strncmp("log_file", variable, 8) ) {
			strcpy(params->log_file, value);
		}else if (0 == strncmp("debug_level", variable, 11)) {
			params->debug_level = atoi(value);
		}

	}

	fclose(file);
	return 0;
}

