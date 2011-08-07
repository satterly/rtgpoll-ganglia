/****************************************************************************
   Program:     $Id: rtgpoll.c,v 1.22 2003/09/25 15:56:04 rbeverly Exp $
   Author:      $Author: rbeverly $
   Date:        $Date: 2003/09/25 15:56:04 $
   Description: RTG SNMP get dumps to MySQL database
****************************************************************************/

#define _REENTRANT
#include "common.h"
#include "rtg.h"

#include <syslog.h>
#include <ganglia.h>

/* Yes.  Globals. */
stats_t stats =
{PTHREAD_MUTEX_INITIALIZER, 0, 0, 0, 0, 0, 0, 0, 0, 0.0};
char *target_file = NULL;
target_t *current = NULL;
int entries = 0;
/* dfp is a debug file pointer.  Points to stderr unless debug=level is set */
FILE *dfp = NULL;
const char *program = "rtgpoll";

Ganglia_pool global_context;
Ganglia_metric gmetric;
Ganglia_gmond_config gmond_config;
Ganglia_udp_send_channels send_channels;

/* Main rtgpoll */
int main(int argc, char *argv[]) {
    crew_t crew;
    pthread_t sig_thread;
    sigset_t signal_set;
    struct timeval now;
    double begin_time, end_time, sleep_time;
    char *conf_file = NULL;
    char *gmond_conf = NULL;
    char errstr[BUFSIZE];
    int ch, i;

	dfp = stderr;

    /* Check argument count */
    if (argc < 3)
	usage(argv[0]);

	/* Set default environment */
    config_defaults(&set);

    /* Parse the command-line. */
    while ((ch = getopt(argc, argv, "c:g:Ghmt:vz")) != EOF)
	switch ((char) ch) {
	case 'c':
	    conf_file = optarg;
	    break;
	case 'g':
	    gmond_conf = optarg;
	    break;
	case 'G':
	    set.goff = TRUE;
	    break;
	case 'h':
	    usage(argv[0]);
	    break;
	case 'm':
	    set.multiple++;
	    break;
	case 't':
	    target_file = optarg;
	    break;
	case 'v':
	    set.verbose++;
	    break;
	case 'z':
	    set.withzeros = TRUE;
	    break;
	}

   //fork unless debug mode
   if (set.verbose == OFF) {
       if (fork() == 0) {
           umask(0);
           setsid();
           close(0);
           close(1);
           close(2);
           if (fork())
               return (0);
       } else {
           return (0);
       }
   }

    /* Initialize syslog */
    openlog(program, LOG_PID, LOG_USER);

    if (set.verbose >= LOW)
	printf("RTG version %s starting.\n", VERSION);
    syslog(LOG_NOTICE, "RTG version %s starting.", VERSION);

    /* Initialize signal handler */
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGHUP);
    sigaddset(&signal_set, SIGUSR1);
    sigaddset(&signal_set, SIGUSR2);
    sigaddset(&signal_set, SIGTERM);
    sigaddset(&signal_set, SIGINT);
    sigaddset(&signal_set, SIGQUIT);
	if (!set.multiple) 
    	checkPID(PIDFILE);

    if (pthread_sigmask(SIG_BLOCK, &signal_set, NULL) != 0) {
	printf("pthread_sigmask error\n");
        syslog(LOG_ERR, "pthread_sigmask error!");
    }

    /* Read configuration file to establish local environment */
    if (conf_file) {
      if ((read_rtg_config(conf_file, &set)) < 0) {
         printf("Could not read config file: %s\n", conf_file);
         syslog(LOG_ERR, "Could not read config file: %s", conf_file);
	 closelog();
         exit(-1);
      }
    } else {
      conf_file = malloc(BUFSIZE);
      if (!conf_file) {
         printf("Fatal malloc error!\n");
         syslog(LOG_ERR, "Fatal malloc error!");
         closelog();
         exit(-1);
      }
      for(i=0;i<CONFIG_PATHS;i++) {
        snprintf(conf_file, BUFSIZE, "%s%s", config_paths[i], DEFAULT_CONF_FILE); 
        if (read_rtg_config(conf_file, &set) >= 0) {
           break;
        } 
        if (i == CONFIG_PATHS-1) {
           snprintf(conf_file, BUFSIZE, "%s%s", config_paths[0], DEFAULT_CONF_FILE); 
	   if ((write_rtg_config(conf_file, &set)) < 0) {
	      fprintf(stderr, "Couldn't write config file.\n");
	      syslog(LOG_ERR, "Couldn't write config file '%s'.", conf_file);
              closelog();
	      exit(-1);
	    }
        }
      }
    }
    if (gmond_conf) {
    /* Open the Ganglia config file */
    FILE *gfp;

        if ((gfp = fopen(gmond_conf, "r")) == NULL) {
                fprintf(stderr, "\nCould not open target file for reading '%s'.\n", gmond_conf);
                syslog(LOG_ERR, "Could not open target file for reading '%s'.", gmond_conf);
                closelog();
                exit(1);
        }
    } else {
      gmond_conf = malloc(BUFSIZE);
      if (!gmond_conf) {
         printf("Fatal malloc error!\n");
         syslog(LOG_ERR, "Fatal malloc error!");
         closelog();
         exit(1);
      }
        snprintf(gmond_conf, BUFSIZE, "%s", DEFAULT_GMOND_CONF); 
    }
    if (set.verbose >= LOW)
            printf("\nUsing Ganglia config file [%s].\n", gmond_conf);
    syslog(LOG_NOTICE, "Using Ganglia config file [%s].", gmond_conf);

    /* hash list of targets to be polled */
    if (target_file != NULL) 
	entries = hash_target_file(target_file);
    else {
        fprintf(stderr, "No target file defined. Exiting.\n");
        syslog(LOG_ERR, "No target file defined. Exiting.");
        closelog();
        exit(-1);
    }
    if (entries <= 0) {
	fprintf(stderr, "Error updating target list.\n");
	syslog(LOG_ERR, "Error updating target list.");
	closelog();
	exit(-1);
    }
    if (set.verbose >= LOW)
	printf("Initializing threads (%d).\n", set.threads);
    syslog(LOG_NOTICE, "Initializing threads (%d).", set.threads);
    pthread_mutex_init(&(crew.mutex), NULL);
    pthread_cond_init(&(crew.done), NULL);
    pthread_cond_init(&(crew.go), NULL);
    crew.work_count = 0;

    /* Initialize the SNMP session */
    if (set.verbose >= LOW)
	printf("Initializing SNMP (v%d, port %d).\n", set.snmp_ver, set.snmp_port);
    syslog(LOG_NOTICE, "Initializing SNMP (v%d, port %d).", set.snmp_ver, set.snmp_port);
    init_snmp("RTG");

    /* Ganglia setup */
    if (set.verbose >= LOW)
	printf("Initializing Ganglia UDP send channels.\n");
    syslog(LOG_NOTICE, "Initializing Ganglia UDP send channels.");
    /* create the global context */
    global_context = Ganglia_pool_create(NULL);
    if (!global_context) {
        fprintf(stderr, "Unable to create global context. Exiting.\n");
        syslog(LOG_ERR, "Unable to create global context. Exiting.");
        closelog();
        exit(1);
    }

    /* parse the configuration file */
    gmond_config = Ganglia_gmond_config_create(gmond_conf, 0);       // FIXME -- use SYSCONFDIR

    /* build the udp send channels */
    send_channels = Ganglia_udp_send_channels_create(global_context, gmond_config);
    if (!send_channels) {
        fprintf(stderr, "Unable to create ganglia send channels. Exiting.\n");
        syslog(LOG_ERR, "Unable to create ganglia send channels. Exiting.");
        closelog();
        exit(1);
    }
    if (set.verbose >= LOW)
	printf("Ganglia Ready.\n");
    syslog(LOG_NOTICE, "Ganglia Ready.");

    if (set.verbose >= HIGH)
	printf("Starting threads.\n");
    syslog(LOG_NOTICE, "Starting threads.");

    for (i = 0; i < set.threads; i++) {
	crew.member[i].index = i;
	crew.member[i].crew = &crew;
	if (pthread_create(&(crew.member[i].thread), NULL, poller, (void *) &(crew.member[i])) != 0) {
	    printf("pthread_create error\n");
            syslog(LOG_ERR, "pthread_create error");
        }
    }
    if (pthread_create(&sig_thread, NULL, sig_handler, (void *) &(signal_set)) != 0) {
	printf("pthread_create error\n");
	syslog(LOG_ERR, "pthread_create error");
    }

    /* give threads time to start up */
    sleep(1);

    if (set.verbose >= LOW)
	printf("RTG Ready.\n");
    syslog(LOG_NOTICE, "RTG Ready.");

    /* Loop Forever Polling Target List */
    while (1) {
	lock = TRUE;
	gettimeofday(&now, NULL);
	begin_time = (double) now.tv_usec / 1000000 + now.tv_sec;

	PT_MUTEX_LOCK(&(crew.mutex));
	init_hash_walk();
	current = getNext();
	crew.work_count = entries;
	PT_MUTEX_UNLOCK(&(crew.mutex));
	    
	if (set.verbose >= LOW)
        timestamp("Queue ready, broadcasting thread go condition.");
	PT_COND_BROAD(&(crew.go));
	PT_MUTEX_LOCK(&(crew.mutex));
	    
	while (crew.work_count > 0) {
		PT_COND_WAIT(&(crew.done), &(crew.mutex));
	}
	PT_MUTEX_UNLOCK(&(crew.mutex));

	gettimeofday(&now, NULL);
	lock = FALSE;
	end_time = (double) now.tv_usec / 1000000 + now.tv_sec;
	stats.poll_time = end_time - begin_time;
        stats.round++;
	sleep_time = set.interval - stats.poll_time;

	if (waiting) {
	    if (set.verbose >= HIGH)
		printf("Processing pending SIGHUP.\n");
            syslog(LOG_NOTICE, "Processing pending SIGHUP.");
	    entries = hash_target_file(target_file);
	    waiting = FALSE;
	}
	if (set.verbose >= LOW) {
        snprintf(errstr, sizeof(errstr), "Poll round %d complete.", stats.round);
        timestamp(errstr);
        syslog(LOG_NOTICE, "%s", errstr);
	    print_stats(stats);
    }
	if (sleep_time <= 0)
	    stats.slow++;
	else
	    sleepy(sleep_time);

    } /* while */

    exit(0);
}


/* Signal Handler.  USR1 increases verbosity, USR2 decreases verbosity. 
   HUP re-reads target list */
void *sig_handler(void *arg)
{
    sigset_t *signal_set = (sigset_t *) arg;
    int sig_number;

    while (1) {
	sigwait(signal_set, &sig_number);
	switch (sig_number) {
            case SIGHUP:
                if(lock) {
                    waiting = TRUE;
                }
                else {
                    entries = hash_target_file(target_file);
                    waiting = FALSE;
                }
                break;
            case SIGUSR1:
                set.verbose++;
                break;
            case SIGUSR2:
                set.verbose--;
                break;
            case SIGTERM:
            case SIGINT:
            case SIGQUIT:
                if (set.verbose >= LOW)
                   printf("Quiting: received signal %d.\n", sig_number);
                syslog(LOG_WARNING, "Quiting: received signal %d", sig_number);
                unlink(PIDFILE);
                closelog();
                exit(1);
                break;
        }
   }
}


void usage(char *prog)
{
    printf("rtgpoll - RTG v%s\n", VERSION);
    printf("Usage: %s [-Gmz] [-vvv] [-c <rtg.conf>] [-g <gmond.conf>] -t <target.conf>\n", prog);
    printf("\nOptions:\n");
    printf("  -c <file>   Specify RTG configuration file\n");
    printf("  -g <file>   Specify Ganglia agent config file (default: /etc/ganglia/gmond.conf)\n");
    printf("  -G          Disable Ganglia metric reporting\n");
    printf("  -t <file>   Specify SNMP target file\n");
    printf("  -v          Increase verbosity\n");
	printf("  -m          Allow multiple instances\n");
	printf("  -z          Database zero delta inserts\n");
    printf("  -h          Help\n");
    exit(-1);
}
