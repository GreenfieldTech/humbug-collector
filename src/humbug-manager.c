/*
 *   humbug-manager.c - Communication between Humbug analytics collection agent
 *   					and Asterisk(TM) manager
 *   Copyright (C) 2011-2012 , Humbug Analytics Labs
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

#include <arpa/inet.h>
#include <netdb.h>

#include "include/humbug-collector.h"

/*******************************************************************************
 * Connect to Asterisk manager
 */
int create_manager_socket(void) {
	struct sockaddr_in addr;
	int sock_desc = -1;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(config.port);

	write_log("Try to connect to asterisk manager", DEBUG);

	sock_desc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == sock_desc) {
		sprintf(log_message, "Can`t create socket for asterisk manager on: %s:%d",
				config.managerIp, config.port);
		write_log(log_message, ERROR);
		return -1;
	}
	addr.sin_addr.s_addr = inet_addr(config.managerIp);
	if (-1 == connect(sock_desc, (const void *) &addr, sizeof(addr))) {
		sprintf(log_message, "Can`t connect to asterisk`s manager: %s:%d",
				config.managerIp, config.port);
		write_log(log_message, ERROR);
		return -1;
	}

	write_log("Connected", DEBUG);
	return sock_desc;
}

/*******************************************************************************
 * Login to Asterisk manager
 */
int login_to_manager(void) {
	char auth_str[1024];
	write_log("Login to asterisk manager", INFO);
	sprintf(auth_str, "Action: Login\r\n"
					  "Username: %s\r\n"
			          "Secret: %s\r\n\r\n",
			          config.userName, config.secret);
	send_to_manager(auth_str);
	return 1;
}

/*******************************************************************************
 * Send Hangup action to asterisk manager
 */
void do_hangup(char *channel){
	char command[1024];
	if(config.hangup == 0){
		write_log("Action 'Hangup' is closed in humbug.conf file", WARNING);
		return;
	}
	sprintf(command, "Action: Hangup\r\n"
					 "Channel: %s\r\n\r\n", channel);
	send_to_manager(command);
	return;
}

/*******************************************************************************
 * Send action to asterisk manager
 */
void send_to_manager(char *action){
	errno = 0;
	if(!action){
		return;
	}
	if(send(manager_socket, action, strlen(action), 0) < 0){
		sprintf(log_message, "Can`t send data to manager: %s\n", strerror(errno));
		write_log(log_message, ERROR);
	}
	return;
}

/*******************************************************************************
 *
 */
event_t *new_event_param(void){
	event_t *tmp  = (event_t *)malloc(sizeof(event_t));
	tmp->key[0]   = 0;
	tmp->value[0] = 0;
	tmp->next     = NULL;
	return tmp;
}

/*******************************************************************************
 * Returns event key value
 */
char *get_event_value(event_t *event_root, char *key){
	event_t *current;
	for(current = event_root; current!=NULL; current = current->next){
		if(strncasecmp(current->key, key, strlen(key)) == 0){
			return current->value;
		}
	}
	return NULL;
}

/*******************************************************************************
 * Destroy event`s parameters list
 */
void destroy_event(event_t *event_root){
	if(event_root == NULL) return;
	event_t *current = event_root;
	event_t *next = event_root->next;
	free(current);
	destroy_event(next);
	return;
}

/*******************************************************************************
 * Get PROTO from Channel
 */
proto get_proto(char *channel){
	if(!channel){
		return UNKNOWN;
	}
	if(strncasecmp(channel, "local", 5) == 0) return LOCAL;
	if(strncasecmp(channel, "iax2",  4) == 0) return IAX2;
	if(strncasecmp(channel, "dahdi", 5) == 0) return DAHDI;
	if(strncasecmp(channel, "zap",   3) == 0) return ZAP;
	if(strncasecmp(channel, "sip",   3) == 0) return SIP;
	if(strncasecmp(channel, "h323",  4) == 0) return H323;

	return UNKNOWN;
}

/*******************************************************************************
 * Returns phone number from dial string
 */
char *get_number(char *s){
	char c[] = {'@', '/'};
	char *tc = NULL;
	int i;
	int num_len;

	for (i=0; i<sizeof(c); i++){
		tc = strchr(s, c[i]);
		if(tc) break;
	}

	if (c[i] == '@'){
		num_len = tc - s;
		tc = s;
	}else if (c[i] == '/'){
		tc++;
		num_len = strlen(tc);
	}else{
		num_len = strlen(s);
		tc = s;
	}

	char *num = (char *)malloc(32);
	memcpy(num, tc, num_len);
	num[num_len] = 0;

	return num;
}

/*******************************************************************************
 * Returns trunk from dial string
 */
char *get_trunk(char *s){
	char c[] = {'@', '/'};
	char *tc = NULL;
	int i;
	int trunk_len;

	for (i=0; i<sizeof(c); i++){
		tc = strchr(s, c[i]);
		if(tc) break;
	}

	if (c[i] == '@'){
		tc++;
		trunk_len = strlen(tc);
	}else if (c[i] == '/'){
		trunk_len = tc - s;
		tc = s;
	}else{
		return NULL;
	}

	char *trunk = (char *)malloc(trunk_len + 1);
	memcpy(trunk, tc, trunk_len);
	trunk[trunk_len] = 0;

	return trunk;
}

/*******************************************************************************
 *
 */
void create_collector_event(char *event, char *num, char *desc, char *uniq,
		char *event_time){

	event_t *collector_event = new_event_param();
	event_t *curr = collector_event;
	strcpy(collector_event->key,  "Event");
	strcpy(collector_event->value, event);

	curr->next = new_event_param();
	curr = curr->next;
	strcpy(curr->key,   "Num");
	strcpy(curr->value,  num);

	curr->next = new_event_param();
	curr = curr->next;
	strcpy(curr->key,   "Desc");
	strcpy(curr->value,  desc);

	curr->next = new_event_param();
	curr = curr->next;
	strcpy(curr->key,   "UniqueID");
	strcpy(curr->value,  uniq);

	curr->next = new_event_param();
	curr = curr->next;
	strcpy(curr->key,   "event_time");
	strcpy(curr->value,  event_time);

	process_event(collector_event);
	destroy_event(collector_event);
	return;
}
