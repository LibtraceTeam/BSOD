extern float compact(struct in_addr number, int offset);
extern float compact2(struct in_addr number, int offset);
extern void get_colour(char color[3], int port, int protocol);
extern int get_start_pos(float start[3], struct in_addr source, int iface);
extern int get_end_pos(float end[3], struct in_addr dest, int iface);
extern int per_packet(const dag_record_t *erfptr, uint32_t caplen, uint64_t ts/*, int new_fd*/);
extern void empty_flows();
extern void init_packets(int new_fd);
