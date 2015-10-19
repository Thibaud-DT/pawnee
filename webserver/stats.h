typedef struct {
	int served_connections;
	int served_requests;
	int ok_200;
	int ko_400;
	int ko_403;
	int ko_404;
} web_stats;

int init_stats(void);
web_stats *get_stats(void);
void stats_new_connection(void);
void stats_new_request(void);
void stats_ok_200(void);
void stats_ko_400(void);
void stats_ko_403(void);
void stats_ko_404(void);

