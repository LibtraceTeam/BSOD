extern int setup_listen_socket();
extern int bind_tcp_socket(int listener, int port);
extern int check_client(int *fdmax, fd_set *master);
extern int send_new_flow(int new_fd, float start[3], float end[3], uint32_t count);
extern int send_new_packet(/*fd_set *master,*/int new_fd, double ts, uint32_t id, char colour[3], uint16_t size);
extern int send_kill_flow(int new_fd, uint32_t id);
