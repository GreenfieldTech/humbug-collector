/*
 *   humbug-manager.h - Communication between Humbug analytics collection agent
 *   					and Asterisk manager(TM)
 *   Copyright (C) 2011-2012, Humbug Analytics Labs
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

#ifndef HUMBUG_MANAGER_H_
#define HUMBUG_MANAGER_H_

/* Asterisk event parameter */
typedef struct event_t{
	char key[32];
	char value[1024];
	struct event_t *next;
} event_t;

/* Known protocols */
typedef enum {
	LOCAL, IAX2, DAHDI, ZAP, SIP, H323, UNKNOWN
} proto;

/* Asterisk manager socket desc */
int manager_socket;

#endif /* HUMBUG_MANAGER_H_ */

/* */
int create_manager_socket(void);
/* */
int login_to_manager(void);
/* */
void send_to_manager(char *__action);
/* */
void do_hangup(char *__chan);
/* */
event_t *new_event_param(void);
/* */
char *get_event_value(event_t *__event_root, char *__key);
/* */
void destroy_event(event_t *__event_root);
/* */
proto get_proto(char *__chan);
/* */
char *get_number(char *__dialstr);
/* */
char *get_trunk(char *__dialstr);
/* */
void create_collector_event(char *__event, char *__num, char *__desc,
		char *__uniq, char *__event_time);
