/*
 *	 humbug-time.h
 *   Copyright (C) 2012, Humbug Analytics Labs
 *
 *   http://www.humbuglabs.org or support@humbuglabs.org
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

#include <time.h>

#ifndef HUMBUG_TIME_H_
#define HUMBUG_TIME_H_

typedef struct interval_t {
	int from;
	int to;
	struct interval_t *next;
} timerange_t;

typedef struct hours_t {
	int day_f;
	int from;
	int to;
} hours_t;

#endif /* HUMBUG_TIME_H_ */

/* */
timerange_t *create_timeranges_list(char *__str);

/* */
timerange_t *create_timeranges_node(char *__str);

/* */
int check_timeranges_list(struct tm *__event_time, timerange_t *__list);

/* */
void destroy_timeranges_list(timerange_t *__dates_list);

/* */
int create_hours_list(char *__hours);

/* */
int check_hours_list(hours_t __list[], struct tm *__event_time);

/* */
timerange_t *get_timeranges_list(char *__uri);
