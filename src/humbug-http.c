/*
 *   humbug-http.c - HTTP implementation for Humbug analytics collection agent
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
 * Prepare POST string
 */
void process_event(event_t *event){
	char *_encode(char *__s);
	event_t *current;
	char post_str[MAX_REQUEST_DATA_LENGTH];
	char encoded_post[2048] = "data=";
	char *encoded_str;
	char response[2048];
	int  result;

	bzero(post_str, MAX_REQUEST_DATA_LENGTH);

	// Create POST body
	for(current = event; current!=NULL; current = current->next){
		strcat(post_str, current->key);
		strcat(post_str, "=");
		strcat(post_str, current->value);
		strcat(post_str, "&");
		sprintf(log_message, "Key: %s; Value: %s", current->key, current->value);
		write_log(log_message, DEBUG);
	}

	if (config.encrypted == 1){
		encoded_str = _encode(post_str);
		strcat(encoded_post, encoded_str);
		free(encoded_str);
		strcat(encoded_post, "&");
		strcat(encoded_post, gateway_str);
		strcat(encoded_post, "&encoded=1");
		result = send_to_humbug(encoded_post, config.humbugScriptLink, response);
	}else{
		strcat(post_str, "&");
		strcat(post_str, gateway_str);
		strcat(post_str, "&encoded=0");
		write_log(post_str, DEBUG);
		result = send_to_humbug(post_str, config.humbugScriptLink, response);
	}

	bzero(post_str, MAX_REQUEST_DATA_LENGTH);

	return;
}

/*******************************************************************************
 * Encode request string
 */
char *_encode(char *tmp) {
	int i, y;
	char *out = (char*) calloc(1, 2048);
	char *pOut;
	int key_len = strlen(config.humbugKey);
	int len     = strlen(tmp);

	write_log("Encode data.", DEBUG);

	y = 0;
	pOut = out;

	for (i = 0; i < len; i++) {
		if (y == key_len)
			y = 0;
		char sym = (char) (tmp[i] ^ config.humbugKey[y]);
		sprintf(pOut, "%02x", sym);
		pOut += 2;
		y++;
	}
	write_log("Return encoded string.", DEBUG);
	return out;
}

/*******************************************************************************
 * Send HTTP POST request to humbug API
 */
int send_to_humbug(char *post_str, char *uri, char *resp){
	char request[4096];
	int  post_str_len;
	int  request_len;
	int  sock_desc = -1;
	struct hostent *server;
	struct sockaddr_in serv_addr;
	struct timeval tv;
	errno = 0;

	// Define socket timeout
	tv.tv_sec  = SEND_TIMEOUT;
	tv.tv_usec = 0;

	// Connect to Humbug
	if((server = gethostbyname(config.humbugHost)) == NULL){
		sprintf(log_message, "Can`t resolve '%s' host.", config.humbugHost);
		write_log(log_message, ERROR);
		return 0;
	}

	if((sock_desc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
		sprintf(log_message, "Socket error %d: %s", errno, strerror(errno));
		write_log(log_message, ERROR);
		errno = 0;
		return 0;
	}

	// Set socket timeout
	if(setsockopt(sock_desc, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1){
		sprintf(log_message, "Error while set socket option %d: %s", errno, strerror(errno));
		write_log(log_message, ERROR);
		errno = 0;
		return 0;
	}

	//memset((char*) &serv_addr, 0, sizeof(serv_addr));
	bzero((char*) &serv_addr, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
	serv_addr.sin_port = htons(config.humbugPort);

	if(connect(sock_desc, (const void *) &serv_addr, sizeof(serv_addr)) == -1){
		sprintf(log_message, "Can`t connect to humbug host %d: %s", errno, strerror(errno));
		write_log(log_message, ERROR);
		shutdown(sock_desc, SHUT_RDWR);
		close(sock_desc);
		return 0;
	}

	// Prepare HTTP POST request
	post_str_len = strlen(post_str);

	sprintf(request, "POST /%s HTTP/1.1\r\n"
					 "Host: %s\r\n"
					 "User-Agent: Humbug Collector Agent v.%s\r\n"
					 "Content-Type: application/x-www-form-urlencoded\r\n"
					 "Connection: close\r\n"
					 "Content-Length: %i\r\n"
					 "\r\n"
					 "%s",
					 uri, config.humbugHost, MY_VERSION, post_str_len, post_str
			);

	request_len = strlen(request);

	write_log(request, DEBUG);

	// Send data
	write_log("Sending request to Humbug", DEBUG);
	int result = send(sock_desc, request, request_len, 0);
	if (result < 0) {
		sprintf(log_message, "Can`t send data to Humbug: %d; %s", errno, strerror(errno));
		write_log(log_message, ERROR);
		shutdown(sock_desc, SHUT_RDWR);
		close(sock_desc);
		return 0;
	}
	write_log("Sent OK", DEBUG);

	// Receive response from Humbug
	char c;
	char stack[4];
	char *p = resp;
	int  done = 0;
	write_log("Receive data from Humbug", DEBUG);

	while(1){
		if(recv(sock_desc, &c, 1, 0) > 0){
			if(done == 1){
				// Get response body
				*p++ = c;
			}else{
				// Drop response header
				//fprintf(stdout, "%c", c);
				stack[3] = stack[2];
				stack[2] = stack[1];
				stack[1] = stack[0];
				stack[0] = c;
				if(stack[0] == '\n' && stack[1] == '\r' && stack[2] == '\n' && stack[3] == '\r')
					done = 1;
				continue;
			}
		}else{
			if(errno == EWOULDBLOCK){
				sprintf(log_message, "Can`t receive data from Humbug. Socket timeout: %d; %s",
					errno, strerror(errno));
				write_log(log_message, ERROR);
				shutdown(sock_desc, SHUT_RDWR);
				close(sock_desc);
				return 0;
			}
			// Data received
			break;
		}
	}

	*p = 0;
	shutdown(sock_desc, SHUT_RDWR);
	close(sock_desc);
	write_log("Data received", DEBUG);
	return 1;
}
