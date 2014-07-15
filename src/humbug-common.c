/*
 *   humbug-common.c - The Humbug analytics collection agent for Asterisk(TM)
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

#include "include/humbug-collector.h"
#include "jansson/jansson.h"

/*******************************************************************************
 * Init config structure
 */
void init_config(void){
	int i;
	config.port            = 5038;
	strcpy(config.managerIp, "127.0.0.1");
	config.userName[0]     = 0;
	config.secret[0]       = 0;
	config.humbugPort      = 80;
	config.reload_int	   = 300;
	strcpy(config.humbugHost, "api.humbuglabs.org");
	config.humbugApikey[0] = 0;
	for(i=0; i<EVENTS_LEN; i++){
		config.events[i][0] = 0;
	}
	config.eventsSize      = 0;
	config.humbugKey[0]    = 0;
	config.encrypted       = 0;
	config.hangup	       = 0;
	config.debug_level     = 0;
	config.check_community = 0;
	config.logFileName[0]  = 0;
	config.trunks_conf     = NULL;
	config.conf_all        = NULL;

	// Init global var`s
	manager_socket = -1;
	debug = 0;

	return;
}

/*******************************************************************************
 * Read and parse config file, fill 'params' structure
 */
int read_config(void) {
	FILE *file;
	char conf_buf[CONF_STRING_LENGTH];

	file = fopen(CONF_FILE, "r");
	if (NULL == file) {
		fprintf(stderr, "ERROR Can`t open config file: %s\n", strerror(errno));
		return -1;
	}

	while (NULL != fgets(conf_buf, CONF_STRING_LENGTH, file)) {

		if (('#' == *conf_buf) || ('=' == *conf_buf) || ('\n' == *conf_buf)
				|| (';' == *conf_buf) || ('\r' == *conf_buf)) {
			continue;
		}

		if ('\n' == conf_buf[strlen(conf_buf) - 1]) {
			conf_buf[strlen(conf_buf) - 1] = '\0';
		}

		if (strlen(conf_buf) <=2  || '=' == conf_buf[strlen(conf_buf) - 1] ||
				NULL == strchr(conf_buf, '=')) {
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
			config.port = atoi(value);
		}
		if (0 == strncmp("address", variable, 7)) {
			strcpy(config.managerIp, value);
		}
		if (0 == strncmp("user", variable, 4)) {
			if(strlen(value) == 0){
				fprintf(stderr, "ERROR Asterisk Manager 'user' is empty. Check config file\n");
				return -1;
			}
			strcpy(config.userName, value);
		}
		if (0 == strncmp("secret", variable, 6)) {
			if(strlen(value) == 0){
				fprintf(stderr, "ERROR Asterisk Manager 'secret' is empty. Check config file\n");
				return -1;
			}
			strcpy(config.secret, value);
		}
		if (0 == strncmp("h_port", variable, 6)) {
			config.humbugPort = atoi(value);
		}
		if (0 == strncmp("conf_reload", variable, 11)) {
			config.reload_int = atoi(value);
		}
		if (0 == strncmp("h_host", variable, 6)) {
			strcpy(config.humbugHost, value);
		}
		if (0 == strncmp("h_apikey", variable, 8)) {
			if(strlen(value) == 0){
				fprintf(stderr, "ERROR Humbug 'h_apikey' is empty. Check config file\n");
				return -1;
			}
			strcpy(config.humbugApikey, value);
		}
		if (0 == strncmp("h_read", variable, 6)) {
			char *pEvent;
			config.eventsSize = 0;
			if ( NULL != (pEvent = strtok(value, ",")) ) {
				strcpy( config.events[config.eventsSize++], pEvent );
				while ( NULL != ( pEvent = strtok(NULL, ",")) ) {
					strcpy( config.events[config.eventsSize++], pEvent );
				}
			}
		}
		if (0 == strncmp("encrypted", variable, 9)) {
			if (0 == strncasecmp("yes", value, 3)) {
				config.encrypted = 1;
			} else {
				config.encrypted = 0;
			}
		}
		if (0 == strncmp("community_blacklist", variable, 19)) {
			if (0 == strncasecmp("yes", value, 3)) {
				config.check_community = 1;
			} else {
				config.check_community = 0;
			}
		}
		if (0 == strncmp("action_hangup", variable, 13)) {
			if (0 == strncasecmp("yes", value, 3)) {
				config.hangup = 1;
			} else {
				config.hangup = 0;
			}
		}

		if (0 == strncmp("h_key", variable, 5)) {
			strcpy(config.humbugKey, value);
		}
		if ( 0 == strncmp("log_file", variable, 8) ) {
			if (0 != strlen(value)) {
				strcpy(config.logFileName, value);
			}
		}
		if (0 == strncmp("debug_level", variable, 11)) {
			if(config.debug_level == 0){
				config.debug_level = (value == NULL) ? 0 : atoi(value);
			}
		}

	}

	fclose(file);
	return 0;
}

/*******************************************************************************
 * Get and parse config from Humbug
 */
int get_config(void){
	char response[4096];
	int res;

	bzero(response, 4096);
	res = send_to_humbug(gateway_str, HUMBUG_CONFIG, response);

	if(debug) fprintf(stdout, "%s\n\n", response);

	if(!res || strcasecmp("false", response) == 0){
		/* If no result, or result is false - live all days of week opened,
		 * or leave old configuration
		 */
		return 0;
	}

	json_t *root;
	json_t *data;
	json_error_t error;

	root = json_loads(response, 0, &error);

	if(!root){
		sprintf(log_message, "JSON parser: on line %d: %s\n", error.line, error.text);
		write_log(log_message, ERROR);
		return 0;
	}

	/* Destroy list if present */
	destroy_trunks_list();

	int config_len;
	const char *pref;

	int i;
	data = json_object_get(root, "data");
	if(!json_is_array(data)){
		sprintf(log_message, "JSON parser: 'data' value isn`t array");
		write_log(log_message, WARNING);
		json_decref(root);
		return (0);
	}

	config_len = json_array_size(data);
	if(config_len == 0){
		sprintf(log_message, "JSON parser: 'data' value array is empty");
		write_log(log_message, WARNING);
		json_decref(root);
		return (0);
	}

	config.trunks_conf = list_init(&numbers_cmp);
	trunk_t *tr;

	for(i=0; i<config_len; i++){
		json_t *trunk_info;
		json_t *buf;

	/* Init trunk config structure */
		tr = (trunk_t *)malloc(sizeof(trunk_t));
		tr->name[0]            = 0;
		tr->local_pref[0]      = 0;
		tr->country_trunk[0]   = 0;
		tr->local_area_code[0] = 0;
		tr->min_len            = -1;
		tr->max_len            = -1;
		tr->long_dist_pref     = NULL;
		tr->custom_pref        = NULL;
		tr->special_pref       = NULL;
		tr->blacklist          = NULL;
		tr->whitelist          = NULL;
		tr->black_timeranges   = NULL;
		// Open all hours of day
		int m;
		for(m=0; m<7; m++){
			tr->hours_list[m].day_f = 1;
		}

		trunk_info = json_array_get(data, i);
		if(!json_is_object(trunk_info)){
			sprintf(log_message, "JSON parser: 'data' array value %d is not a object", i);
			write_log(log_message, WARNING);
			free(tr);
			json_decref(root);
			return (0);
		}

	/* Get local country prefix */
		buf = json_object_get(trunk_info, "local_prefix");
		if(!json_is_string(buf)){
			sprintf(log_message, "JSON parser: in 'data' array %d (local_prefix) is not a string", i);
			write_log(log_message, WARNING);
			strcpy(tr->local_pref, "");
		}else{
			strcpy(tr->local_pref, json_string_value(buf));
		}
//		fprintf(stdout, "local_prefix: %s\n", tr->local_pref);

	/* Get local area code */
		buf = json_object_get(trunk_info, "local_areacode");
		if(!json_is_string(buf)){
			sprintf(log_message, "JSON parser: in 'data' array %d (local_areacode) is not a string", i);
			write_log(log_message, WARNING);
			strcpy(tr->local_area_code, "");
		}else{
			strcpy(tr->local_area_code, json_string_value(buf));
		}
//		fprintf(stdout, "local_areacode: %s\n", tr->local_area_code);

	/* Get min length of local number */
		buf = json_object_get(trunk_info, "min_len");
		if(!json_is_integer(buf)){
			sprintf(log_message, "JSON parser: in 'data' array %d (min_len) is not a number", i);
			write_log(log_message, WARNING);
		}else{
			tr->min_len = json_integer_value(buf);
		}
//		fprintf(stdout, "min_len: %d\n", tr->min_len);

	/* Get max length of local number */
		buf = json_object_get(trunk_info, "max_len");
		if(!json_is_integer(buf)){
			sprintf(log_message, "JSON parser: in 'data' array %d (max_len) is not a number", i);
			write_log(log_message, WARNING);
		}else{
			tr->max_len = json_integer_value(buf);
		}
//		fprintf(stdout, "max_len: %d\n", tr->min_len);

	/* Get country trunk */
		buf = json_object_get(trunk_info, "country_trunk");
		if(!json_is_string(buf)){
			sprintf(log_message, "JSON parser: in 'data' array %d (country_trunk) is not a string", i);
			write_log(log_message, WARNING);
			strcpy(tr->country_trunk, "");
		}else{
			strcpy(tr->country_trunk, json_string_value(buf));
		}
//		fprintf(stdout, "country_trunk: %s\n", tr->country_trunk);

	/* Get trunk name */
		buf = json_object_get(trunk_info, "trunk");
		if(!json_is_string(buf)){
			sprintf(log_message, "JSON parser: in 'data' array %d (trunk) is not a string", i);
			write_log(log_message, WARNING);
			strcpy(tr->name, "all");
		}else{
			const char *tr_n = json_string_value(buf);
			if(strlen(tr_n) == 0){
				strcpy(tr->name, "all");
			}else{
				strcpy(tr->name, tr_n);
			}
		}
		if(strncasecmp(tr->name, "all", 3) == 0){
			config.conf_all = tr;
		}
//		fprintf(stdout, "trunk: %s\n", tr->name);

	/* Get long distance prefixes */
		buf = json_object_get(trunk_info, "long_prefix");
		if(!json_is_array(buf)){
			sprintf(log_message, "JSON parser: in 'data' array %d (long_prefix) is not a array", i);
			write_log(log_message, WARNING);
		}else{
			int n, l;
			l = json_array_size(buf);
			if(l > 0){
				// Init long distance prefixes list in trunk config
				tr->long_dist_pref = list_init(&numbers_cmp);
				for(n=0; n<l; n++){
					json_t *ldp = json_array_get(buf, n);
					if(!json_is_string(ldp)){
						sprintf(log_message, "JSON parser: in 'data' array %d (long_prefix) %d is not a string", i, n);
						write_log(log_message, WARNING);
						continue;
					}
					num_data_t *nd = (num_data_t *)malloc(sizeof(num_data_t));
					pref = json_string_value(ldp);
					if(strlen(pref) == 0){
						free(nd);
						continue;
					}
					strcpy(nd->number, pref);
					nd->num_len = strlen(nd->number);
					list_insert(tr->long_dist_pref, nd->number, nd);
				}
			}
		} // End if (not array)

	/* Get custom prefixes */
		buf = json_object_get(trunk_info, "custom_prefix");
		if(!json_is_array(buf)){
			sprintf(log_message, "JSON parser: in 'data' array %d (custom_prefix) is not a array", i);
			write_log(log_message, WARNING);
		}else{
			int n, l;
			l = json_array_size(buf);
			if(l > 0){
				// Init long distance prefixes list in trunk config
				tr->custom_pref = list_init(&numbers_cmp);
				for(n=0; n<l; n++){
					json_t *cp = json_array_get(buf, n);
					if(!json_is_string(cp)){
						sprintf(log_message, "JSON parser: in 'data' array %d (cp) %d is not a string", i, n);
						write_log(log_message, WARNING);
						continue;
					}
					num_data_t *pd = (num_data_t *)malloc(sizeof(num_data_t));
					pref = json_string_value(cp);
					if(strlen(pref) == 0){
						free(pd);
						continue;
					}
					strcpy(pd->number, pref);
					pd->num_len = strlen(pd->number);
					list_insert(tr->custom_pref, pd->number, pd);
				}
			}
		} // End if (not array)

	/* Get special prefixes */
		buf = json_object_get(trunk_info, "special_prefix");
		if(!json_is_array(buf)){
			sprintf(log_message, "JSON parser: in 'data' array %d (special_prefix) is not a array", i);
			write_log(log_message, WARNING);
		}else{
			int n, l;
			l = json_array_size(buf);
			if(l > 0){
				// Init special prefixes list in trunk config
				tr->special_pref = list_init(&numbers_cmp);
				for(n=0; n<l; n++){
					json_t *sp = json_array_get(buf, n);
					if(!json_is_string(sp)){
						sprintf(log_message, "JSON parser: in 'data' array %d (sp) %d is not a string", i, n);
						write_log(log_message, WARNING);
						continue;
					}
					num_data_t *spn = (num_data_t *)malloc(sizeof(num_data_t));
					pref = json_string_value(sp);
					if(strlen(pref) == 0){
						free(spn);
						continue;
					}
					strcpy(spn->number, pref);
					spn->num_len = strlen(spn->number);
					list_insert(tr->special_pref, spn->number, spn);
				}
			}
		} // End if (not array)

	/* Get private blacklist */
		buf = json_object_get(trunk_info, "black");
		if(!json_is_array(buf)){
			sprintf(log_message, "JSON parser: in 'data' array %d (black) is not a array", i);
			write_log(log_message, WARNING);
		}else{
			int n, l;
			l = json_array_size(buf);
			if(l > 0){
				// Init blacklist
				tr->blacklist = avl_init(&numbers_cmp);
				for(n=0; n<l; n++){
					json_t *bl = json_array_get(buf, n);
					if(!json_is_string(bl)){
						sprintf(log_message, "JSON parser: in 'data' array %d (black) %d is not a string", i, n);
						write_log(log_message, WARNING);
						continue;
					}
					num_data_t *bln = (num_data_t *)malloc(sizeof(num_data_t));
					strcpy(bln->number, json_string_value(bl));
					bln->num_len = strlen(bln->number);
					avl_insert(tr->blacklist, bln->number, bln);
				}
			}
		} // End if (not array)

	/* Get private whitelist */
		buf = json_object_get(trunk_info, "white");
		if(!json_is_array(buf)){
			sprintf(log_message, "JSON parser: in 'data' array %d (white) is not a array", i);
			write_log(log_message, WARNING);
		}else{
			int n, l;
			l = json_array_size(buf);
			if(l > 0){
				// Init whitelist
				tr->whitelist = avl_init(&numbers_cmp);
				for(n=0; n<l; n++){
					json_t *wl = json_array_get(buf, n);
					if(!json_is_string(wl)){
						sprintf(log_message, "JSON parser: in 'data' array %d (white) %d is not a string", i, n);
						write_log(log_message, WARNING);
						continue;
					}
					num_data_t *wln = (num_data_t *)malloc(sizeof(num_data_t));
					strcpy(wln->number, json_string_value(wl));
					wln->num_len = strlen(wln->number);
					avl_insert(tr->whitelist, wln->number, wln);
				}
			}
		} // End if (not array)

	/* Get business hours */
		buf = json_object_get(trunk_info, "business_hours");
		if(!json_is_array(buf)){
			sprintf(log_message, "JSON parser: in 'data' array %d (business_hours) is not a array", i);
			write_log(log_message, WARNING);
		}else{
			int n, l;
			l = json_array_size(buf);
			if(l == 7){
				for(n=0; n<l; n++){
					json_t *bh = json_array_get(buf, n);
					if(!json_is_string(bh)){
						sprintf(log_message, "JSON parser: in 'data' array %d (business_hours) %d is not a string", i, n);
						write_log(log_message, WARNING);
						continue;
					}
					// Fill business hours list
					char pair[16];
					char value[3];
					strcpy(pair, json_string_value(bh));

					memcpy(value, &pair[0], 2);
					value[2] = 0;
					tr->hours_list[n].from = atoi(value)*60;

					memcpy(value, &pair[3], 2);
					value[2] = 0;
					tr->hours_list[n].from += atoi(value);

					memcpy(value, &pair[6], 2);
					value[2] = 0;
					tr->hours_list[n].to = atoi(value)*60;

					memcpy(value, &pair[9], 2);
					value[2] = 0;
					tr->hours_list[n].to += atoi(value);

					if(tr->hours_list[n].from == 0 && tr->hours_list[n].to == 0){
						tr->hours_list[n].day_f = 1;
					}else{
						tr->hours_list[n].day_f = 0;
					}
				}
			}else{
				/* Fucki`n week has less or more than seven days */
			}
		} // End if (not array)

	/* Get timeranges */
		buf = json_object_get(trunk_info, "timeranges");
		if(!json_is_array(buf)){
			sprintf(log_message, "JSON parser: in 'data' array %d (timeranges) is not a array", i);
			write_log(log_message, WARNING);
		}else{
			int n, l;
			l = json_array_size(buf);
			if(l > 0){
				timerange_t *curr;
				for(n=0; n<l; n++){
					json_t *tran = json_array_get(buf, n);
					if(!json_is_string(tran)){
						sprintf(log_message, "JSON parser: in 'data' array %d (timeranges) %d is not a string", i, n);
						write_log(log_message, WARNING);
						continue;
					}
					char tmp_range[32];
					strcpy(tmp_range, json_string_value(tran));
					if(tr->black_timeranges == NULL){
						tr->black_timeranges = create_timeranges_node(tmp_range);
						curr = tr->black_timeranges;
					}else{
						curr->next = create_timeranges_node(tmp_range);
						curr = curr->next;
					}
				}
			}
		} // End if (not array)

		/* Insert current trunk config into list */
		if(list_insert(config.trunks_conf, tr->name, tr) == 0){
			sprintf(log_message, "JSON parser: can`t create list entry for trunk %s", tr->name);
			write_log(log_message, WARNING);
		}
	} // End for

	json_decref(root);
	return 1;
}

/*******************************************************************************
 * Comparison function for numbers table
 */
int numbers_cmp(void *a, void *b){
	return strncmp((char *)a, (char *)b, strlen((char *)a));
}

/*******************************************************************************
 *
 */
void check_event(event_t *event){
	struct tm *et;
	char *e_name;

	e_name = get_event_value(event, "Event");
	if(e_name == NULL){
		return;
	}
	et = get_event_time();

	// Check if needed to send event to humbug
	int event_flag = -1;
	int i;
	for(i=0; i<config.eventsSize; i++){
		if (0 == strncasecmp(e_name, config.events[i], strlen(config.events[i]))){
			if((strncasecmp("dial", e_name, 4) == 0)){
				char *sub_e_name = get_event_value(event, "SubEvent");
				if((strncasecmp("begin", sub_e_name, 5) == 0)){
					// Check timerange, business hours, blacklist
					process_dial_event(event, et);
					return;
				}
			}
			event_flag = 1;
			break;
		}
	}
	if (event_flag < 0) {
		return;
	}

	sprintf(log_message, "Send '%s' data to Humbug", e_name);
	write_log(log_message, DEBUG);
	process_event(event);

	return;
}

/*******************************************************************************
 * Process DIAL event
 * Check white list, time range, business hours and blacklist
 */
int process_dial_event(event_t *dial, struct tm *et){

	char *channel     = get_event_value(dial, "Channel");
	char *destination = get_event_value(dial, "Destination");
	char *dial_str    = get_event_value(dial, "Dialstring");
	char *event_time  = get_event_value(dial, "event_time");
	char *uniqid      = get_event_value(dial, "uniqueid");

	char *trunk  = get_trunk(dial_str);

	trunk_t *trunk_conf = get_trunk_config(config.trunks_conf, trunk);
	if(!trunk_conf){
		sprintf(log_message, "Config for trunk %s not found", trunk);
		write_log(log_message, DEBUG);
		free(trunk);
		return 1;
	}

	sprintf(log_message, "Use config for trunk: %s", trunk_conf->name);
	write_log(log_message, DEBUG);

	char *number = get_number(dial_str);

	// Format number
	format_number(trunk_conf, &number);

	sprintf(log_message, "Check permissions for %s number", number);
	write_log(log_message, DEBUG);

//	fprintf(stdout, "C: %s; D: %s; et: %s; U: %s; N: %s\n", channel, dial_str,
//			event_time, uniqid, number);

	avl_node_t *s_result = avl_search(trunk_conf->whitelist, number);
	if(s_result != NULL){
		// Number found in white list, nothing to do
		sprintf(log_message, "Number %s found in white list. Make call.",
				number);
		write_log(log_message, INFO);
		return 1;
	}

	if(check_timeranges_list(et, trunk_conf->black_timeranges)){
		do_hangup(destination);
		do_hangup(channel);
		create_collector_event("CollectorHangup", number, "TIMERANGE",
				uniqid, event_time);
		sprintf(log_message, "Call dropped because timerange is closed. Channel: %s", channel);
		write_log(log_message, WARNING);
		free(trunk);
		free(number);
		return 0;
	}


	if(check_hours_list(trunk_conf->hours_list, et) == 0){
		do_hangup(destination);
		do_hangup(channel);
		create_collector_event("CollectorHangup", number, "BUSINESSHOURS",
				uniqid, event_time);
		sprintf(log_message, "Call dropped because hour is closed. Channel: %s", channel);
		write_log(log_message, WARNING);
		free(trunk);
		free(number);
		return 0;
	}

	s_result = avl_search(trunk_conf->blacklist, number);
	if(s_result != NULL){
		do_hangup(destination);
		do_hangup(channel);
		create_collector_event("CollectorHangup", number, "BLACKLIST",
				uniqid, event_time);
		sprintf(log_message, "Call dropped because number %s found in blacklist.",
				number);
		write_log(log_message, WARNING);
		free(number);
		return 0;
	}

	if(config.check_community == 1){
		if(check_community_blacklist(number)){
			do_hangup(destination);
			do_hangup(channel);
			create_collector_event("CollectorHangup", number, "COMMUNITYBLACKLIST",
					uniqid, event_time);
			sprintf(log_message, "Call dropped because number %s found in community blacklist.",
					number);
			write_log(log_message, WARNING);
			free(number);
			return 0;
		}
	}
	sprintf(log_message, "Number %s is OK", number);
	write_log(log_message, DEBUG);
	free(number);
	free(trunk);
	return 1;
}

/*******************************************************************************
 *
 */
void format_number(trunk_t *trunk_conf, char **number){
	char *pNum;
	char tmp_n[32];

	strcpy(tmp_n, *number);
	pNum = tmp_n;

	/* Remove lead (+) */
	if(strncasecmp("+", pNum, 1) == 0){
		pNum ++;
	}

	/* Remove custom prefix */
	list_node_t *cp = list_search(trunk_conf->custom_pref, pNum);
	if(cp){
		int removal = (int)((num_data_t *)cp->data)->num_len;
		pNum += removal;
	}

	/* Check special prefix */
	list_node_t *sp = list_search(trunk_conf->special_pref, pNum);
	if(sp){
		sprintf(log_message, "Number '%s' includes special prefix", pNum);
		write_log(log_message, DEBUG);
		sprintf(*number, "%s", pNum);
		return;
	}

	/* Remove long distance prefix */
	list_node_t *ldp = list_search(trunk_conf->long_dist_pref, pNum);
	if(ldp){
		int removal = (int)((num_data_t *)ldp->data)->num_len;
		pNum += removal;
		sprintf(*number, "%s", pNum);
		return;
	}

	/* Local call */
	if(strlen(trunk_conf->country_trunk) > 0 && strncmp(pNum, trunk_conf->country_trunk,
				strlen(trunk_conf->country_trunk)) == 0){
		pNum += strlen(trunk_conf->country_trunk);
		sprintf(*number, "%s%s", trunk_conf->local_pref, pNum);
		sprintf(log_message, "Change country trunk '%s' to country prefix '%s' "
				"result: %s", trunk_conf->country_trunk, trunk_conf->local_pref,
				*number);
		write_log(log_message, DEBUG);
	}else{
		/* Long distance call without long distance prefix */
		if(strlen(pNum) > trunk_conf->max_len){
			sprintf(*number, "%s", pNum);
			sprintf(log_message, "Long dist prefix not found but number is to long result: %s",
					*number);
			write_log(log_message, DEBUG);
		}else{
			/* Local call */
			if(trunk_conf->min_len <= strlen(pNum) && strlen(pNum) <= trunk_conf->max_len){
				sprintf(*number, "%s%s", trunk_conf->local_pref, pNum);
				sprintf(log_message, "Add local trunk '%s' result: %s",
						trunk_conf->country_trunk, *number);
				write_log(log_message, DEBUG);
			}else{
				sprintf(*number, "%s%s%s", trunk_conf->local_pref,
						trunk_conf->local_area_code, pNum);
				sprintf(log_message, "Add country and area codes");
				write_log(log_message, DEBUG);
			}

		}
	}
	return;
}

/*******************************************************************************
 *
 */
int check_community_blacklist(char *num){
	char resp[32];
	char post[256];
	int res;

	if(!num) return 1;

	sprintf(post, "%s&num=%s", gateway_str, num);
	res = send_to_humbug(post, HUMBUG_COMM, resp);

	if(debug) fprintf(stdout, "%s\n\n", resp);

	if(res == 0) return 0;

	if(strcasecmp("true", resp) == 0){
		return 1;
	}
	return 0;
}

/*******************************************************************************
 * Writes formatted time into arg str
 */
void create_time(char *my_time) {
	time_t result;
	struct tm *tmp;
	result = time(NULL);
	tmp = localtime(&result);
	strftime(my_time, 24, "%Y-%m-%d %H:%M:%S", tmp);
	return;
}

/*******************************************************************************
 * Format time string
 */
void format_time(char *my_time, struct tm *tmp) {
	strftime(my_time, 24, "%Y-%m-%d %H:%M:%S", tmp);
	return;
}

/*******************************************************************************
 * Returns current timestamp
 */
int get_timestamp(struct tm *tmp){
	return (int) mktime(tmp);
}

/*******************************************************************************
 * Returns local time 'tm'
 */
struct tm *get_event_time(void){
	time_t result = time(NULL);
	return (localtime(&result));
}

/*******************************************************************************
 *
 */
trunk_t *get_trunk_config(list_root_t *list, char *trunk_name){
	list_node_t *result;
	if(!trunk_name || !strlen(trunk_name)){
		return (NULL);
	}
	if(!list){
		return (NULL);
	}

	result = list_search(list, trunk_name);

	if(!result){
		if(debug) fprintf(stdout, "Config not found use 'all'\n");
		return (config.conf_all);
	}
	return ((trunk_t *)result->data);
}

/*******************************************************************************
 *
 */
void destroy_trunks_list(){
	list_root_t *trunks_list = config.trunks_conf;
	if(!trunks_list)
		return;

	list_node_t *tr_next;
	list_node_t *tr_curr = trunks_list->head;

	while(tr_curr){
		tr_next = tr_curr->next;

		list_root_t *ld_pref = ((trunk_t *)tr_curr->data)->long_dist_pref;
		list_destroy(ld_pref);
		write_log("Free long distance OK", DEBUG);

		list_root_t *c_pref = ((trunk_t *)tr_curr->data)->custom_pref;
		list_destroy(c_pref);
		write_log("Free country prefixes OK", DEBUG);

		list_root_t *s_pref = ((trunk_t *)tr_curr->data)->special_pref;
		list_destroy(s_pref);
		write_log("Free special prefixes OK", DEBUG);

		avl_root_t *bl = ((trunk_t *)tr_curr->data)->blacklist;
		avl_destroy(bl);
		write_log("Free blacklist OK", DEBUG);

		avl_root_t *wl = ((trunk_t *)tr_curr->data)->whitelist;
		avl_destroy(wl);
		write_log("Free whitelist OK", DEBUG);

		timerange_t *bt = ((trunk_t *)tr_curr->data)->black_timeranges;
		destroy_timeranges_list(bt);
		write_log("Free timeranges OK", DEBUG);

		tr_curr = tr_next;
	}

	list_destroy(trunks_list);
	write_log("Free trunks configs OK", DEBUG);

	return;
}

/*******************************************************************************
 * Write message to log file
 */
void write_log(char *message, int type){
	FILE *log_file;
	char time_now[64];
	char class[10];
	errno = 0;

	if(config.debug_level < type) return;

	create_time(time_now);

	switch(type){
		case WARNING:
			sprintf(class, "%s", "WARNING ");
			break;
		case ERROR:
			sprintf(class, "%s", "ERROR   ");
			break;
		case INFO:
			sprintf(class, "%s", "INFO    ");
			break;
		case DEBUG:
			sprintf(class, "%s", "DEBUG   ");
			break;
		default:
			sprintf(class, "%s", "MSG     ");
			break;
	}

	if(1 == debug){
		fprintf(stdout, "%s %s %s\n", time_now, class, message);
	}

	if (0 != strlen(config.logFileName)) {
		log_file = fopen(config.logFileName, "a");
		if(NULL == log_file){
			if(errno == EMFILE || errno == EFBIG || errno == ENOSPC){
				if(debug){
					sprintf(log_message, "ERROR    Can`t write log file. %d: %s", errno,
							strerror(errno));
					fprintf(stdout, "%s %s\n", time_now, log_message);
				}
				errno = 0;
				return;
			}
			sprintf(log_message, "ERROR    Can`t write log file. %d: %s", errno,
				strerror(errno));
			fprintf(stdout, "%s %s\n", time_now, log_message);
			exit(EXIT_FAILURE);
		}
		fprintf(log_file, "%s %s %s\n", time_now, class, message);
		fclose(log_file);
	}
	return;
}

/*******************************************************************************
 * Prints help screen
 */
void usage(void) {
	fprintf(stdout, "Usage:\n");
	fprintf(stdout, " humbug-collector [-d <debug level>] [-v] [-h|?]\n");
	fprintf(stdout, "Options:\n");
	fprintf(stdout,
			" -d,   --debug\t<debug level>\tStarts humbug-collector in debug mode\n");
	fprintf(stdout, "\t\t\t\t  debug level:\n");
	fprintf(stdout, "\t\t\t\t\t1 - displays errors only\n");
	fprintf(stdout, "\t\t\t\t\t2 - displays errors and warnings\n");
	fprintf(stdout, "\t\t\t\t\t3 - all above, and program info\n");
	fprintf(stdout, "\t\t\t\t\t4 - displays debug info (huge data)\n");
	fprintf(stdout,
			" -v,   --version\t\tDisplays version information and exit\n");
	fprintf(stdout, " -h|?, --help\t\t\tDisplays this message\n");
}

/*******************************************************************************
 * Prints version
 */
void version( void ) {
	fprintf(stdout, "humbug-collector v.%s (C) 2011-2012 Humbug Labs Ltd.\n", MY_VERSION);
}

