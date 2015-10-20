#include <stdlib.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <string.h>
#include "stats.h"

static web_stats *site_stats;
static sem_t *sem;

int init_stats(void) {
	sem = mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	sem_init(sem, 0, 1);
	site_stats = mmap(NULL, sizeof *site_stats, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	site_stats->served_connections = 0;
	site_stats->served_requests = 0;
	site_stats->ok_200 = 0;
	site_stats->ko_400 = 0;
	site_stats->ko_403 = 0;
	site_stats->ko_404 = 0;
}

web_stats *get_stats(void) {
	sem_wait(sem);
	web_stats *stats = (web_stats *)malloc(sizeof(web_stats));
	memcpy(stats, site_stats, sizeof(site_stats));
	sem_post(sem);
	return stats;
}

void stats_new_connection(void) {
	sem_wait(sem);
	site_stats->served_connections++;
	sem_post(sem);
}

void stats_new_request(void) {	
	sem_wait(sem);
	site_stats->served_requests++;
	sem_post(sem);	
}

void stats_ok_200(void) {
	sem_wait(sem);
	site_stats->ok_200++;
	sem_post(sem);
}

void stats_ko_400(void) {
	sem_wait(sem);
	site_stats->ko_400++;
	sem_post(sem);
}

void stats_ko_403(void) {
	sem_wait(sem);
	site_stats->ko_403++;
	sem_post(sem);
}

void stats_ko_404(void) {
	sem_wait(sem);
	site_stats->ko_404++;
	sem_post(sem);
}
