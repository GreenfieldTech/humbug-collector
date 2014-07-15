/*
 *   humbug-http.h - The Humbug analytics collection agent for Asterisk(TM)
 *   Copyright (C) 2012, Humbug Analytics Labs
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

#ifndef HUMBUG_HTTP_H_
#define HUMBUG_HTTP_H_


#endif /* HUMBUG_HTTP_H_ */

/* */
//char *encode(char *__tmp);
/* */
void process_event(event_t *__event_root);
/* */
int send_to_humbug(char *__post_body, char *__uri, char *__response);
