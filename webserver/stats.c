#include <stdlib.h>
#include <sys/mman.h>
#include "stats.h"

static web_stats *site_stats;

int init_stats(void) {
	site_stats = mmap(NULL, sizeof *site_stats, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	site_stats->served_connections = 0;
	site_stats->served_requests = 0;
	site_stats->ok_200 = 0;
	site_stats->ko_400 = 0;
	site_stats->ko_403 = 0;
	site_stats->ko_404 = 0;
}

web_stats *get_stats(void) {
	return site_stats;
}

void stats_new_connection(void) {
	site_stats->served_connections++;
}

void stats_new_request(void) {
	site_stats->served_requests++;
}

void stats_ok_200(void) {
	site_stats->ok_200++;
}

void stats_ko_400(void) {
	site_stats->ko_400++;
}

void stats_ko_403(void) {
	site_stats->ko_403++;
}

void stats_ko_404(void) {
	site_stats->ko_404++;
}
