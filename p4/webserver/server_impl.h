#ifndef __SERVER_IMPL_H__
#define __SERVER_IMPL_H__

void server_init(int argc, char* argv[]);
void server_dispatch(int connfd);

#endif /* __SERVER_IMPL_H__ */
