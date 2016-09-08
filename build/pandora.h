#ifndef pandora_h
#define pandora_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define PND_OK               0x00
#define PND_INVOCATION_ERROR 0x01
#define PND_CONNECTION_ERROR 0x02
#define PND_SOCKET_ERROR     0x03

typedef void PND_HANDLER;
typedef struct {
  char *msg;
  int *imsg;
} PND_MESSAGE;
typedef struct {
  char *ev;
  PND_HANDLER (*callback)(PND_MESSAGE*);
} PND_LISTENER;

struct Pandora {
  char *host;
  int port;
  int socket;
  int listenersc;

  struct hostent *server;
  /* char buffer[256]; */

  void (*info)(void);
  int (*check)(void);
  int (*connect)(char*, int);
  void (*on)(char*, PND_HANDLER*);
  void (*emit)(char*);
  void (*digest)(void);
  void (*close)(int);

  struct sockaddr_in serv_addr;
  PND_LISTENER *listeners;
};
void _info(void);
int _check(void);
int _connect(char*, int);
void _on(char*, PND_HANDLER*);
void _emit(char*);
void _digest(void);
void _close(int);

struct Pandora pandora;
#endif
