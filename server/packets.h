extern int per_packet(const dag_record_t *erfptr, uint32_t caplen, uint64_t ts);
extern void empty_flows();
extern void init_packets(int new_fd);
extern int send_flows(int fd);
