extern int setup_listen_socket();
extern int bind_tcp_socket(int listener, int port);
extern int check_clients(bool wait);
extern int send_new_flow(float start[3], float end[3], uint32_t count);
extern int send_update_flow(int fd, float start[3], float end[3], uint32_t count);
extern int send_new_packet(uint64_t ts, uint32_t id, uint8_t colour[3], uint16_t size);
extern int send_kill_flow(uint32_t id);
void hax_fdmax(int fd);
