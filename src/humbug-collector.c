/*
 *   humbug-collector.c - The Humbug analytics collection agent for Asterisk(TM)
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


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <fcntl.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#include "include/humbug-collector.h"

volatile sig_atomic_t graceful_shutdown = 0;

int pid_file_desc = -1;

/*******************************************************************************
 *
 */
int main (int argc, char *argv[]) {

	int conf_signals(void);
	int become_daemon(int * const pid_file_desc);

	init_config();

	int   debugLevel;
	int   next_option;
	char *short_options = "d:c:p:hv?";

	struct option long_options[] = {
				{ "debug", required_argument, NULL, 'd' },
				{ "configfile", required_argument, NULL, 'c'},
				{ "pidfile", required_argument, NULL, 'p'},
			    { "help", 0, no_argument, 'h' },
			    { "version", 0, no_argument, 'v' },
			    { NULL, 0, NULL, 0 }
		};

	// Read CLI parameters
	do {
		next_option = getopt_long(argc, argv, short_options, long_options, NULL);
		switch (next_option) {
			case 'd':
				debug = 1;
				debugLevel = atoi(optarg);
				config.debug_level = (debugLevel >= ERROR && debugLevel <= DEBUG)
										? debugLevel : DEBUG;
				version();
				sprintf(log_message, "Starting in DEBUG mode (level %d)",
						config.debug_level);
				write_log(log_message, 0);
				break;
			case 'c':
				strcpy(config.configFileName, optarg);
				printf("=== Loading configuration from %s\n", config.configFileName);
				break;
			case 'p':
				strcpy(config.humbugPid, optarg);
				printf("=== Writing pid file to %s\n", config.humbugPid);	
				break;
			case 'h':
				version();
				usage();
				exit(EXIT_SUCCESS);
				break;
			case 'v':
				version();
				exit(EXIT_SUCCESS);
				break;
			case '?':
				version();
				usage();
				exit(EXIT_SUCCESS);
				break;
		}
	} while ( -1 != next_option);

	// Read config file
	if (read_config() < 0) {
		exit(EXIT_FAILURE);
	}

	// Create gateway string
	sprintf(gateway_str, "gateway=%s", config.humbugApikey);

	// Configure signals
	conf_signals();

	// Fork process
	if (debug == 0) {
		write_log("Starting collector", INFO);
		if (become_daemon(&pid_file_desc) < 0) {
			exit(EXIT_FAILURE);
		}
	}

	// Connect to asterisk manager
	if ((manager_socket = create_manager_socket()) < 0) {
		shutdown_graceful();
	}

	// Login to asterisk manager
	if (login_to_manager() < 0) {
		shutdown_graceful();
	}

	// Get data from Humbug
	get_config();

	// Set last reload time
	struct tm *rt = get_event_time();
	reload_time   = get_timestamp(rt);

	// Read data from asterisk manager
	char c;
	char last_c = 0;
	char *p;
	int done = 0;

	event_t *event   = NULL; // Event parameters list head
	event_t *current = NULL; // Current parameter

	while(1){
		int result = recv(manager_socket, &c, 1, 0);

		if (result <= 0) {
			write_log("Connection to asterisk manager lost.", WARNING);
			shutdown(manager_socket, SHUT_RDWR);
			manager_socket = -1;

			int attempt_count = 1;
			while(attempt_count < 21){
				sprintf(log_message, "Try reconnect to asterisk manager #%d.", attempt_count);
				write_log(log_message, DEBUG);

				if ((manager_socket = create_manager_socket()) > 0) {
					sprintf(log_message, "New socket created. desc: %i", manager_socket);
					write_log(log_message, DEBUG);
					if(login_to_manager()){
						write_log("Now connected to asterisk manager.", INFO);
						break;
					}
				}
				attempt_count++;
				sleep(10);
			} /* WHILE reconnect to manager */
		}else{
			if(debug) fprintf(stdout, "%c", c);

			if(event == NULL){
				// Init event list
				event   = new_event_param();
				current = event;
				p = current->key;
			}

			if (' ' == c && ':' == last_c) {
				// Start value processing
				*(--p) = '\0';
				p = current->value;
				continue;
			}else if(last_c == '\r' && c == '\n'){
				if (1 == done) {
					// Add event_time parameter
					char my_time[32];
					struct tm *curr_et = get_event_time();
					format_time(my_time, curr_et);
					strcpy(current->key, "event_time");
					strcpy(current->value, my_time);

					check_event(event);
					destroy_event(event);

					// End event processing
					done   = 0;
					last_c = 0;
					p = NULL;
					current = NULL;
					event = NULL;
					// Check last reload
					if(config.reload_int < (get_timestamp(curr_et) - reload_time)){
						get_config();
						reload_time = get_timestamp(curr_et);
					}
					continue;
				}else{
					done = 1;
					*p   = '\0';
					// Create new key/value pair
					current->next = new_event_param();
					current = current->next;
					// start key processing
					p = current->key;
					continue;
				}

			}else if('\r' == c){
				last_c = c;
			}else{
				last_c = c;
				if('<' == c || '"' == c || '>' == c){
					*p++ = '\'';
				}else{
					*p++ = c;
				}
				done = 0;
			} /* End IF key/value processing */
		} /* End IF connection lost */
	} /* WHILE */


	shutdown_graceful();
	exit(EXIT_SUCCESS);
}

/*******************************************************************************
 * Daemonize program
 */
int become_daemon(int * const pid_file_desc) {
	// Thanks to David Gillies
	int lock_file, kill_result, lock_result, my_pid, stdio_desc;
	FILE *pid_file;
	char pid_buf[17], *lock_str, pid_str[16];
	char pid_filename[1024];

	unsigned long lock_pid;
	struct flock exclusiveLock;

	errno = 0;

	chdir("/");
	// TODO chroot
	strcpy(pid_filename, PID_FILE);
	if (strlen(config.humbugPid)) {
		strcpy(pid_filename, config.humbugPid);
	}

	lock_file = open(pid_filename, O_RDWR | O_CREAT | O_EXCL, 0644);
	if (lock_file < 0) {
		pid_file = fopen(pid_filename, "r");

		if (NULL == pid_file) {
			fprintf(stderr, "ERROR. Can`t read/create %s file: %s\n", PID_FILE, strerror(errno));
			return -1;
		}

		lock_str = fgets(pid_buf, 16, pid_file);
		if (NULL != lock_str) {
			if ('\n' == pid_buf[strlen(pid_buf) - 1]){
				pid_buf[strlen(pid_buf)-1] = 0;
			}
			lock_pid = strtoul(pid_buf, (char**)0, 10);
			kill_result = kill(lock_pid, 0);
			if (0 == kill_result) {
				fprintf(stderr, "ERROR. A lock file %s has been detected. "
					"It is owned by the (active) process with PID %ld.\n",
					pid_filename, lock_pid);
			}else{
				if (errno == ESRCH) {
					fprintf(stderr, "ERROR. A lock file %s has been detected. "
						"It appears it is owned by the process with PID %ld, which is now defunct. "
						"Delete the lock file and try again.\n",
						pid_filename, lock_pid);
				} else {
					fprintf(stderr, "Could not acquire exclusive lock on PID file.\n");
				}
			}
		}else{
			fprintf(stderr,	"ERROR. Can`t read process number from PID file.");
		}
		fclose(pid_file);
		return -1;
	}
	exclusiveLock.l_type   = F_WRLCK;
	exclusiveLock.l_whence = SEEK_SET;
	exclusiveLock.l_len    = 0;
	exclusiveLock.l_start  = 0;
	exclusiveLock.l_pid    = 0;

	lock_result = fcntl(lock_file, F_SETLK, &exclusiveLock);
	if (lock_result < 0) {
		fprintf(stderr, "ERROR. Can't lock PID file: %s\n", strerror(errno));
		close(lock_file);
		return -1;
	}

	my_pid = fork();
	switch (my_pid) {
		case 0:
			if (setsid() < 0) {
				fprintf(stderr, "ERROR. Can`t create new session: %s\n", strerror(errno));
				return -1;
			}
			break;
		case -1:
			fprintf(stderr, "ERROR. Can`t fork process: %s\n", strerror(errno));
			return -1;
			break;
		default:
			exit(EXIT_SUCCESS);
			break;
	}

	if (ftruncate(lock_file, 0) < 0) {
		fprintf(stderr, "ERROR. Can`t truncate PID file: %s\n", strerror(errno));
		return -1;
	}

	sprintf(pid_str, "%d\n", (int) getpid());
	write(lock_file, pid_str, strlen(pid_str));
	*pid_file_desc = lock_file;

	umask(0);

	stdio_desc = open("/dev/null", O_RDWR);
	dup(stdio_desc);
	dup(stdio_desc);

	setpgrp();
	return 0;
}

/*******************************************************************************
 * Configure signals
 */
int conf_signals(){
	// Thanks to David Gillies
	void go_fatal(int);
	void go_term(int);
	void go_usr1(int);
	void go_hup(int);
	void go_int(int);

	struct sigaction sig_hup, sig_usr1, sig_term, sig_int;

	/* Ignored */
	signal(SIGUSR2,   SIG_IGN);
	signal(SIGPIPE,   SIG_IGN);
	signal(SIGALRM,   SIG_IGN);
	signal(SIGTSTP,   SIG_IGN);
	signal(SIGTTIN,   SIG_IGN);
	signal(SIGTTOU,   SIG_IGN);
	signal(SIGURG,    SIG_IGN);
	signal(SIGXCPU,   SIG_IGN);
	signal(SIGXFSZ,   SIG_IGN);
	signal(SIGVTALRM, SIG_IGN);
	signal(SIGPROF,   SIG_IGN);
	signal(SIGIO,     SIG_IGN);
	signal(SIGCHLD,   SIG_IGN);

	/* Fatal */
	signal(SIGQUIT,   go_fatal);
	signal(SIGILL,    go_fatal);
	signal(SIGTRAP,   go_fatal);
	signal(SIGABRT,   go_fatal);
	signal(SIGIOT,    go_fatal);
	signal(SIGBUS,    go_fatal);
	signal(SIGFPE,    go_fatal);
	signal(SIGSEGV,   go_fatal);
	signal(SIGSTKFLT, go_fatal);
	signal(SIGCONT,   go_fatal);
	signal(SIGPWR,    go_fatal);
	signal(SIGSYS,    go_fatal);
#ifdef SIGEMT /* this is not defined under Linux */
	signal ( SIGEMT, go_fatal );
#endif

	/* Terminate */
	sig_term.sa_handler = go_term;
	sigemptyset(&sig_term.sa_mask);
	sig_term.sa_flags = 0;
	sigaction(SIGTERM, &sig_term, NULL);

	/* Finish */
	sig_usr1.sa_handler = go_usr1;
	sigemptyset(&sig_usr1.sa_mask);
	sig_usr1.sa_flags = 0;
	sigaction(SIGUSR1, &sig_usr1, NULL);

	/* HUP */
	sig_hup.sa_handler = go_hup;
	sigemptyset(&sig_hup.sa_mask);
	sig_hup.sa_flags = 0;
	sigaction(SIGHUP, &sig_hup, NULL);

	/* INT */
	sig_int.sa_handler = go_int;
	sigemptyset(&sig_int.sa_mask);
	sig_int.sa_flags = 0;
	sigaction(SIGINT, &sig_int, NULL);

	return 0;
}

/*******************************************************************************
 * HUP signal. Do reload config file
 */
void go_hup(int sig) {
	write_log("Caught SIGHUP.", INFO);

	if (manager_socket != -1) {
		shutdown(manager_socket, SHUT_RDWR);
		close(manager_socket);
		manager_socket = -1;
	}

	// Init config structure
	init_config();

	// Read from config file
	if (read_config() < 0) {
		exit(EXIT_FAILURE);
	}

	// Get config from Humbug
	get_config();

	if ((manager_socket = create_manager_socket()) < 0) {
		exit(EXIT_FAILURE);
	}

	if (login_to_manager() < 0) {
		exit(EXIT_FAILURE);
	}
	return;
}

//******************************************************************************
void go_fatal(int sig) {
	sprintf(log_message, "Caught: %s", strsignal(sig));
	write_log(log_message, ERROR);
	shutdown_graceful();
	return;
}

//******************************************************************************
void go_term(int sig) {
	sprintf(log_message, "Caught SEGTERM: %s", strsignal(sig));
	write_log(log_message, INFO);
	shutdown_graceful();
	return;
}

//******************************************************************************
void go_usr1(int sig) {
	sprintf(log_message, "Caught SIGUSR1: %s", strsignal(sig));
	write_log(log_message, INFO);
	shutdown_graceful();
	return;
}

//******************************************************************************

void go_int(int sig) {
	sprintf(log_message, "Caught SIGINT: %s", strsignal(sig));
	write_log(log_message, INFO);
	shutdown_graceful();
	return;
}

/*******************************************************************************
 * Shutdown program
 */
void shutdown_graceful(void) {
	write_log("Shutdown graceful.", INFO);

	// Destroy trunks config list
	destroy_trunks_list();

	if (manager_socket != -1) {
		shutdown(manager_socket, SHUT_RDWR);
		close(manager_socket);
		manager_socket = -1;
	}
	if (pid_file_desc != -1) {
		if (!debug) {
			close(pid_file_desc);
			unlink(PID_FILE);
		}
		pid_file_desc = -1;
	}
	exit(EXIT_SUCCESS);
}
