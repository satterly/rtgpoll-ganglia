/****************************************************************************
   Program:     $Id: rtgsnmp.c,v 1.24 2003/09/25 18:23:35 rbeverly Exp $
   Author:      $Author: rbeverly $
   Date:        $Date: 2003/09/25 18:23:35 $
   Description: RTG SNMP Routines
****************************************************************************/

#include "common.h"
#include "rtg.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#ifdef OLD_UCD_SNMP
 #include "asn1.h"
 #include "snmp_api.h"
 #include "snmp_impl.h"
 #include "snmp_client.h"
 #include "mib.h"
 #include "snmp.h"
#else
 #include "net-snmp-config.h"
 #include "net-snmp-includes.h"
#endif

#include <syslog.h>
#include <ganglia.h>

extern target_t *current;
extern stats_t stats;
extern const char *program;

extern Ganglia_pool global_context;
extern Ganglia_metric gmetric;
extern Ganglia_gmond_config gmond_config;
extern Ganglia_udp_send_channels send_channels;

void *poller(void *thread_args)
{
    worker_t *worker = (worker_t *) thread_args;
    crew_t *crew = worker->crew;
    target_t *entry = NULL;
    void *sessp = NULL;
    struct snmp_session session;
    struct snmp_pdu *pdu = NULL;
    struct snmp_pdu *response = NULL;
    oid anOID[MAX_OID_LEN];
    size_t anOID_len = MAX_OID_LEN;
    struct variable_list *vars = NULL;
    unsigned long long result = 0;
    unsigned long long last_value = 0;
    unsigned long long insert_val = 0;
    int status = 0, bits = 1, init = 0, is_string = 0;
    char query[BUFSIZE];
    char storedoid[BUFSIZE];
    char result_string[BUFSIZE];
    int rval;

    openlog(program, LOG_PID, LOG_USER);
    syslog(LOG_NOTICE, "Poller sub-routine.");

    if (set.verbose >= HIGH) {
	printf("Thread [%d] starting.\n", worker->index);
        syslog(LOG_NOTICE, "Thread [%d] starting.", worker->index);
    }
    while (1) {
	if (set.verbose >= DEVELOP) {
	    printf("Thread [%d] locking (wait on work)\n", worker->index);
            syslog(LOG_DEBUG, "Thread [%d] locking (wait on work)\n", worker->index);
        }

	PT_MUTEX_LOCK(&crew->mutex);

	while (current == NULL) {
		PT_COND_WAIT(&crew->go, &crew->mutex);
	}
	if (set.verbose >= DEVELOP) {
	    printf("Thread [%d] done waiting, received go (work cnt: %d)\n", worker->index, crew->work_count);
	    syslog(LOG_DEBUG, "Thread [%d] done waiting, received go (work cnt: %d)\n", worker->index, crew->work_count);
        }

	if (current != NULL) {
	    if (set.verbose >= HIGH) {
	      printf("Thread [%d] processing %s %s (%d work units remain in queue)\n", worker->index, current->host, current->objoid, crew->work_count);
	      syslog(LOG_NOTICE, "Thread [%d] processing %s %s (%d work units remain in queue)\n", worker->index, current->host, current->objoid, crew->work_count);
            }
	    snmp_sess_init(&session);
		if (set.snmp_ver == 2)
	      session.version = SNMP_VERSION_2c;
		else
	      session.version = SNMP_VERSION_1;
	    session.peername = current->host;
		session.remote_port = set.snmp_port;
	    session.community = current->community;
	    session.community_len = strlen(session.community);

	    sessp = snmp_sess_open(&session);
	    anOID_len = MAX_OID_LEN;
	    pdu = snmp_pdu_create(SNMP_MSG_GET);
	    read_objid(current->objoid, anOID, &anOID_len);
	    entry = current;
	    last_value = current->last_value;
	    init = current->init;
	    insert_val = 0;
	    bits = current->bits;
	    strncpy(storedoid, current->objoid, sizeof(storedoid));
		current = getNext();
	}
	if (set.verbose >= DEVELOP) {
	    printf("Thread [%d] unlocking (done grabbing current)\n", worker->index);
	    syslog(LOG_DEBUG, "Thread [%d] unlocking (done grabbing current)\n", worker->index);
        }
	PT_MUTEX_UNLOCK(&crew->mutex);
	snmp_add_null_var(pdu, anOID, anOID_len);
	if (sessp != NULL) 
	   status = snmp_sess_synch_response(sessp, pdu, &response);
	else
	   status = STAT_DESCRIP_ERROR;

	/* Collect response and process stats */
	PT_MUTEX_LOCK(&stats.mutex);
	if (status == STAT_DESCRIP_ERROR) {
	    stats.errors++;
            syslog(LOG_ERR, "*** SNMP Error: (%s) Bad descriptor.\n", session.peername);
	} else if (status == STAT_TIMEOUT) {
	    stats.no_resp++;
	    syslog(LOG_ERR, "*** SNMP No response: (%s@%s).\n", session.peername,
	       storedoid);
	} else if (status != STAT_SUCCESS) {
	    stats.errors++;
	    syslog(LOG_ERR, "*** SNMP Error: (%s@%s) Unsuccessuful (%d).\n", session.peername,
	       storedoid, status);
	} else if (status == STAT_SUCCESS && response->errstat != SNMP_ERR_NOERROR) {
	    stats.errors++;
	    syslog(LOG_ERR, "*** SNMP Error: (%s@%s) %s\n", session.peername,
	       storedoid, snmp_errstring(response->errstat));
	} else if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
	    stats.polls++;
	} 
	PT_MUTEX_UNLOCK(&stats.mutex);

	/* Liftoff, successful poll, process it */
	if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
	    vars = response->variables;
#ifdef OLD_UCD_SNMP
            sprint_value(result_string, anOID, anOID_len, vars);
#else
	    snprint_value(result_string, BUFSIZE, anOID, anOID_len, vars);
#endif
	    is_string = 0;
	    switch (vars->type) {
		/*
		 * Switch over vars->type and modify/assign result accordingly.
		 */
		case ASN_COUNTER64:
		    if (set.verbose >= DEBUG) printf("64-bit result: (%s@%s) %s\n", session.peername, storedoid, result_string);
		    result = vars->val.counter64->high;
		    result = result << 32;
		    result = result + vars->val.counter64->low;
		    break;
		case ASN_COUNTER:
		    if (set.verbose >= DEBUG) printf("32-bit result: (%s@%s) %s\n", session.peername, storedoid, result_string);
		    result = (unsigned long) *(vars->val.integer);
		    break;
		case ASN_INTEGER:
		    if (set.verbose >= DEBUG) printf("Integer result: (%s@%s) %s\n", session.peername, storedoid, result_string);
		    result = (unsigned long) *(vars->val.integer);
		    break;
		case ASN_GAUGE:
		    if (set.verbose >= DEBUG) printf("32-bit gauge: (%s@%s) %s\n", session.peername, storedoid, result_string);
		    result = (unsigned long) *(vars->val.integer);
		    break;
		case ASN_TIMETICKS:
		    if (set.verbose >= DEBUG) printf("Timeticks result: (%s@%s) %s\n", session.peername, storedoid, result_string);
		    result = (unsigned long) *(vars->val.integer) / 100; /* TimeTicks are hundredths of a second */
		    break;
		case ASN_OPAQUE:
		    if (set.verbose >= DEBUG) printf("Opaque result: (%s@%s) %s\n", session.peername, storedoid, result_string);
		    result = (unsigned long) *(vars->val.integer);
		    break;
		case ASN_OCTET_STR:
		    if (set.verbose >= DEBUG) printf("String result: (%s@%s) %s\n", session.peername, storedoid, result_string);
                    snprintf(result_string, BUFSIZE, "%s", (char*)vars->val.string);
		    is_string = 1;
		    break;
		default:
		    if (set.verbose >= DEBUG) printf("Unknown result type: (%s@%s) %s\n", session.peername, storedoid, result_string);
		    syslog(LOG_WARNING, "Unknown result type: (%s@%s) %s\n", session.peername, storedoid, result_string);
	    }

		/* Gauge Type */
		if (bits == 1) {
			if (result != last_value) {
				insert_val = result;
				if (set.verbose >= HIGH) 
					printf("Thread [%d]: Gauge change from %lld to %lld\n", worker->index, last_value, insert_val);
			} else {
				if (set.withzeros) 
					insert_val = result;
				if (set.verbose >= HIGH)
					printf("Thread [%d]: Gauge steady at %lld\n", worker->index, insert_val);
			}
	    /* Counter Wrap Condition */
	    } else if (result < last_value) {
			PT_MUTEX_LOCK(&stats.mutex);
              stats.wraps++;
			PT_MUTEX_UNLOCK(&stats.mutex);
	      if (bits == 32) insert_val = (THIRTYTWO - last_value) + result;
	      else if (bits == 64) insert_val = (SIXTYFOUR - last_value) + result;
	      if (set.verbose >= LOW) {
	         printf("*** Counter Wrap (%s@%s) [poll: %llu][last: %llu][insert: %llu]\n",
	         session.peername, storedoid, result, last_value, insert_val);
	      }
	    /* Not a counter wrap and this is not the first poll */
	    } else if ((last_value >= 0) && (init != NEW)) {
		insert_val = result - last_value;
	        /* Print out SNMP result if verbose */
	        if (set.verbose == DEBUG)
		  printf("Thread [%d]: (%lld-%lld) = %llu\n", worker->index, result, last_value, insert_val);
	        if (set.verbose == HIGH)
		  printf("Thread [%d]: %llu\n", worker->index, insert_val);
            /* last_value < 0, so this must be the first poll */
	    } else {
                if (set.verbose >= HIGH) printf("Thread [%d]: First Poll, Normalizing\n", worker->index);
	    }

		/* Check for bogus data, either negative or unrealistic */
	    if (insert_val > entry->maxspeed || result < 0) {
			if (set.verbose >= LOW) {
                            printf("*** Out of Range (%s@%s) [insert_val: %llu] [oor: %lld]\n", session.peername, storedoid, insert_val, entry->maxspeed);
                            syslog(LOG_WARNING, "*** Out of Range (%s@%s) [insert_val: %llu] [oor: %lld]\n", session.peername, storedoid, insert_val, entry->maxspeed);
                        }
			insert_val = 0;
			PT_MUTEX_LOCK(&stats.mutex);
			stats.out_of_range++;
			PT_MUTEX_UNLOCK(&stats.mutex);
	    }

		if (!(set.goff)) {
			PT_MUTEX_LOCK(&crew->mutex);

			// Get IP address of SNMP device for spoof option
			struct addrinfo hints, *res, *p;
			char ipstr[INET6_ADDRSTRLEN];
                
			memset(&hints, 0, sizeof hints);
			hints.ai_family = AF_INET;
			hints.ai_flags = AI_PASSIVE;

			if (getaddrinfo(session.peername, NULL, &hints, &res) != 0) 
				syslog(LOG_ERR, "*** Host Resolution Error: getaddrinfo() failed %s.", session.peername);

			for (p = res; p != NULL; p = p->ai_next) {
				void *addr;
				struct sockaddr_in *ipv4 = (struct sockaddr_in *) p->ai_addr;
				addr = &(ipv4->sin_addr);
				inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
			}

			char spoof[BUFSIZE];
			snprintf(spoof, sizeof(spoof), "%s:%s", ipstr, session.peername);

			char buf_val[BUFSIZE];
                        if (is_string == 0) {
			    snprintf(buf_val, sizeof(buf_val), "%llu", insert_val);
			} else {
			    snprintf(buf_val, sizeof(buf_val), "%s", result_string);
			}
			if (!strcmp(entry->units, "\"\""))
				snprintf(entry->units, sizeof(entry->units), "");

			if (set.verbose >= DEBUG)
				printf("SEND: gmetric --spoof %s:%s --name %s --value %s --type %s --units %s --slope both --tmax %d --dmax %d\n",
				ipstr, session.peername, entry->table, buf_val, (is_string == 0 ? "uint32" : "string"), entry->units, set.interval, entry->iid);

			if (set.verbose >= DEBUG)
                                printf(" --group %s --description %s --title %s\n", 
                                entry->group, entry->descr, entry->title);

			gmetric = Ganglia_metric_create(global_context);
			if (!gmetric) {
				printf("*** Ganglia Error: Unable to allocate gmetric structure. Exiting.\n");
                                syslog(LOG_ERR, "*** Ganglia Error: Unable to allocate gmetric structure. Exiting.");
                                closelog();
				exit(1);
			}

			rval = Ganglia_metric_set(gmetric, entry->table, buf_val, (is_string == 0 ? "uint32" : "string"),
				entry->units, (bits ? GANGLIA_SLOPE_BOTH : GANGLIA_SLOPE_ZERO), set.interval, entry->iid);  // FIXME -- is dmax ignored?

			Ganglia_metadata_add(gmetric, SPOOF_HOST, spoof);
                        if (strlen(entry->group))
				Ganglia_metadata_add(gmetric, "GROUP", entry->group);
                        if (strlen(entry->descr))
				Ganglia_metadata_add(gmetric, "DESC", entry->descr);
                        if (strlen(entry->title))
				Ganglia_metadata_add(gmetric, "TITLE", entry->title);

			/* send the message */
			rval = Ganglia_metric_send(gmetric, send_channels);
			if (rval) {
				printf("*** Ganglia Error: There was an error sending to %d of the send channels.\n", rval);
				syslog(LOG_ERR, "*** Ganglia Error: There was an error sending to %d of the send channels.", rval);
			}

			/* cleanup */
			Ganglia_metric_destroy(gmetric);       /* not really necessary but for symmetry */

			PT_MUTEX_UNLOCK(&crew->mutex);

			if (!status) {
				PT_MUTEX_LOCK(&stats.mutex);
				stats.gmetrics++;
				PT_MUTEX_UNLOCK(&stats.mutex);
			} 
		} /* !goff */

	} /* STAT_SUCCESS */

        if (sessp != NULL) {
           snmp_sess_close(sessp);
           if (response != NULL) snmp_free_pdu(response);
        }

	if (set.verbose >= DEVELOP) {
	    printf("Thread [%d] locking (update work_count)\n", worker->index);
	    syslog(LOG_DEBUG, "Thread [%d] locking (update work_count)\n", worker->index);
        }
	PT_MUTEX_LOCK(&crew->mutex);
	crew->work_count--;
	/* Only if we received a positive result back do we update the
	   last_value object */
	if (status == STAT_SUCCESS) entry->last_value = result;
	if (init == NEW) entry->init = LIVE;
	if (crew->work_count <= 0) {
	    if (set.verbose >= HIGH) {
                printf("Queue processed. Broadcasting thread done condition.\n");
                syslog(LOG_NOTICE, "Queue processed. Broadcasting thread done condition.");
            }
	    PT_COND_BROAD(&crew->done);
	}
	if (set.verbose >= DEVELOP) {
	    printf("Thread [%d] unlocking (update work_count)\n", worker->index);
	    syslog(LOG_DEBUG, "Thread [%d] unlocking (update work_count)\n", worker->index);
        }

		PT_MUTEX_UNLOCK(&crew->mutex);
    }				/* while(1) */
}
