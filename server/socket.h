#ifndef _SOCKET_H
#define _SOCKET_H
int setup_listen_socket();
int bind_tcp_socket(int listener, int port);
int check_clients(bool wait);
int send_new_flow(float start[3], float end[3], uint32_t count);
int send_update_flow(int fd, float start[3], float end[3], uint32_t count);
int send_new_packet(uint64_t ts, uint32_t id, uint8_t colour[3], uint16_t size);
int send_kill_flow(uint32_t id);
void hax_fdmax(int fd);

#endif // _SOCKET_H
