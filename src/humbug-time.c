/*
 * humbug-time.c
 *
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

#include "include/humbug-time.h"
#include "include/humbug-collector.h"

//******************************************************************************
timerange_t *create_timeranges_node(char *s_str){
	timerange_t *t_int;
	t_int = (timerange_t *)malloc(sizeof(timerange_t));
	if(!t_int) return NULL;
	t_int->next = NULL;
	t_int->from = atoi(strtok(s_str, ":"));
	t_int->to   = atoi(strtok(NULL,  ":"));
	return t_int;
}

//******************************************************************************
int check_timeranges_list(struct tm *event_time, timerange_t *list){
	timerange_t *curr;
	if(!list) return 0;
	int tm = get_timestamp(event_time);
	if(debug) fprintf(stdout, "Current timestamp: %i\n", tm);
	for(curr = list; curr!=NULL; curr = curr->next){
		if(curr->from <= tm && tm <= curr->to){
			return 1;
		}
	}
	return 0;
}

//******************************************************************************
void destroy_timeranges_list(timerange_t *d_list){
	if(!d_list)
		return;
	timerange_t *current = d_list;
	timerange_t *next    = d_list->next;
	while(next != NULL){
		free(current);
		current = next;
		next    = current->next;
	}
	free(current);
	d_list = NULL;
	return;
}


//******************************************************************************
int check_hours_list(hours_t list[], struct tm *event_time){
	int t;
	if(list[event_time->tm_wday].day_f){
		return 1;
	}
	t = event_time->tm_hour*60 + event_time->tm_min;
	if(list[event_time->tm_wday].from <= t &&
			t <= list[event_time->tm_wday].to){
		return 1;
	}
	return 0;
}


