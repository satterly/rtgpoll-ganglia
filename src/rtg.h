/****************************************************************************
   Program:     $Id: rtg.h,v 1.20 2003/09/25 15:56:04 rbeverly Exp $ 
   Author:      $Author: rbeverly $
   Date:        $Date: 2003/09/25 15:56:04 $
   Description: RTG headers
****************************************************************************/

#ifndef _RTG_H_
#define _RTG_H_ 1

/* Defines */ 
#ifndef FALSE
# define FALSE 0
#endif
#ifndef TRUE
# define TRUE !FALSE
#endif

/* Constants */
#define MAX_THREADS 10
#define BUFSIZE 512
#define BITSINBYTE 8
#define THIRTYTWO 4294967295ul
#define SIXTYFOUR 18446744073709551615ul

/* Define CONFIG_PATHS places to search for the rtg.conf file.  Note
   that RTG_HOME, as determined during autoconf is one path */
#define CONFIG_PATHS 3
#define CONFIG_PATH_1 ""
#define CONFIG_PATH_2 "/etc/"

/* Defaults */
#define DEFAULT_CONF_FILE "rtg.conf"
#define DEFAULT_THREADS 5
#define DEFAULT_INTERVAL 300
#define DEFAULT_HIGHSKEWSLOP 3
#define DEFAULT_LOWSKEWSLOP .5
#define DEFAULT_OUT_OF_RANGE 93750000000ull
#define DEFAULT_GMOND_CONF "/etc/ganglia/gmond.conf"
#define DEFAULT_SNMP_VER 1
#define DEFAULT_SNMP_PORT 161

/* PID File */
#define PIDFILE "/tmp/rtgpoll.pid"

#define STAT_DESCRIP_ERROR 99
#define HASHSIZE 5000

/* pthread error messages */
#define PML_ERR "pthread_mutex_lock error\n"
#define PMU_ERR "pthread_mutex_unlock error\n"
#define PCW_ERR "pthread_cond_wait error\n"
#define PCB_ERR "pthread_cond_broadcast error\n"

/* pthread macros */
#define PT_MUTEX_LOCK(x) if (pthread_mutex_lock(x) != 0) printf(PML_ERR);
#define PT_MUTEX_UNLOCK(x) if (pthread_mutex_unlock(x) != 0) printf(PMU_ERR);
#define PT_COND_WAIT(x,y) if (pthread_cond_wait(x, y) != 0) printf(PCW_ERR);
#define PT_COND_BROAD(x) if (pthread_cond_broadcast(x) != 0) printf(PCB_ERR);

/* Verbosity levels LOW=info HIGH=info+SQL DEBUG=info+SQL+junk */
enum debugLevel {OFF, LOW, HIGH, DEBUG, DEVELOP}; 

/* Target state */
enum targetState {NEW, LIVE, STALE};

/* Typedefs */
typedef struct worker_struct {
    int index;
    pthread_t thread;
    struct crew_struct *crew;
} worker_t;

typedef struct config_struct {
    unsigned int interval;
    unsigned long long out_of_range;
    enum debugLevel verbose;
    unsigned short withzeros;
    unsigned short goff;
    unsigned short multiple;
    unsigned short snmp_ver;
    unsigned short snmp_port;
    unsigned short threads;
    float highskewslop;
    float lowskewslop;
} config_t;

typedef struct target_struct {
    char host[64];
    char objoid[128];
    unsigned short bits;	// 32/64=counter, 0=gauge
    char community[64];
    char table[64];		// metric name
    char units[64];
    unsigned int iid;		// dmax -- not supported until gmetric v3.2.0
    char title[64];
    char group[64];
    char descr[128];
#ifdef HAVE_STRTOLL
    long long maxspeed;
#else
    long maxspeed;
#endif
    enum targetState init;
    unsigned long long last_value;
    struct target_struct *next;
} target_t;

typedef struct crew_struct {
    int work_count;
    worker_t member[MAX_THREADS];
    pthread_mutex_t mutex;
    pthread_cond_t done;
    pthread_cond_t go;
} crew_t;

typedef struct poll_stats {
    pthread_mutex_t mutex;
    unsigned long long polls;
    unsigned long long gmetrics;
    unsigned int round;
    unsigned int wraps;
    unsigned int no_resp;
    unsigned int out_of_range;
    unsigned int errors;
    unsigned int slow;
    double poll_time; 
} stats_t;

typedef struct hash_struct {
    target_t *table[HASHSIZE];
    int bucket;
    target_t *target;
} hash_t;


/* Precasts: rtgpoll.c */
void *sig_handler(void *);
void usage(char *);

/* Precasts: rtgpoll.c */
void *poller(void *);

/* Precasts: rtgutil.c */
int read_rtg_config(char *, config_t *);
int write_rtg_config(char *, config_t *);
void config_defaults(config_t *);
void print_stats (stats_t);
void sleepy(float);
void timestamp(char *);
int checkPID(char *);
int alldigits(char *);

/* Precasts: rtghash.c */
void init_hash();
void init_hash_walk();
target_t *getNext();
void free_hash();
unsigned long make_key(const void *);
void mark_targets(int);
int delete_targets(int);
void walk_target_hash();
void *in_hash(target_t *, target_t *);
int compare_targets(target_t *, target_t *);
int del_hash_entry(target_t *);
int add_hash_entry(target_t *);
int hash_target_file(char *);

/* Globals */
config_t set;
int lock;
int waiting;
char config_paths[CONFIG_PATHS][BUFSIZE];
hash_t hash;

#endif /* not _RTG_H_ */
