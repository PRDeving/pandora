#ifndef pandora_h
#define pandora_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <pthread.h>

#define PND_STATUS_DISCONNECTED 0x00
#define PND_STATUS_CONNECTED    0x01
#define PND_STATUS_LISTENING    0x02

#define PND_OK                  0x06
#define PND_ERROR_INVOCATION    0x07
#define PND_ERROR_CONNECTION    0x08
#define PND_ERROR_SOCKET        0x09

typedef void* PND_HANDLER;
typedef struct {
  char *msg;
  int *imsg;
} PND_MESSAGE;

typedef struct {
  char *key;
  void *value;
} PND_EVENT;

typedef struct {
  char *ev;
  PND_HANDLER (*callback)(PND_MESSAGE*);
} PND_LISTENER;

struct Pandora {
  char *version;
  int status;
  char *host;
  int port;
  int socket;
  int listenersc;
  int clientsc;

  struct hostent *server;
  /* char buffer[256]; */

  void (*info)(void);
  int (*check)(void);
  int (*listen)(int);
  int (*connect)(char*, int);
  void (*on)(char*, PND_HANDLER);
  void (*emit)(char*, void*);
  void (*digest)(void);
  void (*close)(int);

  struct sockaddr_in serv_addr;
  PND_LISTENER *listeners;
  int *clients;
  fd_set fdclients;
  pthread_t hostRuntime;
};
void _info(void);
int _check(void);
int _listen(int);
int _connect(char*, int);
void _on(char*, PND_HANDLER);
void _emit(char*, void*);
void _digest(void);
void _close(int);

struct Pandora pandora;
#endif
