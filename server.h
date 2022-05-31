#ifndef SERVER_H
#define SERVER_H

int server_listen(char const *id_s);
int server_set_fifo_var(const char *id_s);
int server_clear(const char *id_s);
int server_end(const char *id_s);

#endif
