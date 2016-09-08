#include "pandora.h"

struct Pandora pandora = {
  NULL,       // host (char*)
  0,          // port (int)
  -1,         // socket (int)
  1,          // listenersc (int)
  NULL,       // server (struct hostent*)

  _info,      // void _info
  _check,     // int _check
  _connect,   // int _connect
  _on,        // void _on
  _emit,      // void _emit
  _digest,    // void _digest
  _close,     // int _close
};

int _connect(char *host, int port) {
  if (!strlen(host) || !port) {
    printf("Error connecting\n");
    return PND_INVOCATION_ERROR;
  }
  printf("creating connection with %s:%d...", host, port);
  pandora.host = host;
  pandora.port = port;
  pandora.socket = socket(AF_INET, SOCK_STREAM, 0);
  pandora.server = gethostbyname(pandora.host);
  if (pandora.socket < 0 || pandora.server < 0) {
    printf("ERROR\ncant get sockfd or can't reach host\n");
    return PND_CONNECTION_ERROR;
  }

  bzero((char*) &pandora.serv_addr, sizeof(pandora.serv_addr));
  pandora.serv_addr.sin_family = AF_INET;
  bcopy((char*) pandora.server -> h_addr, 
        (char*) &pandora.serv_addr.sin_addr.s_addr,
        pandora.server -> h_length);
  pandora.serv_addr.sin_port = htons(pandora.port);

  if (connect(pandora.socket,(struct sockaddr*) &pandora.serv_addr,sizeof(pandora.serv_addr)) < 0) {
    printf("ERROR cant connect\n");
    pandora.close(PND_CONNECTION_ERROR);
  }
  pandora.listeners = malloc(sizeof(PND_LISTENER));

  printf("SUCCESS\n");
  return pandora.socket;
}
void _info(void) {
  printf("CONFIGURATION \t\n \
          host: %s\n \
          port: %d\n",
          pandora.host,
          pandora.port);
  printf("STATUS\n \
          %s\n", \
          pandora.socket > -1 ? "connected" : "disconnected");
}
int _check(void) {
  return pandora.socket > -1;
}

void _on(char* ev, PND_HANDLER *callback) {
  PND_LISTENER listener = {ev, callback};
  pandora.listeners[pandora.listenersc - 1] = listener;
  pandora.listenersc++;
  pandora.listeners = realloc(pandora.listeners, sizeof(PND_LISTENER) * pandora.listenersc);
}

void _emit(char* msg) {
  int osocket = write(pandora.socket, msg, strlen(msg));
  if (osocket < 0) {
    printf("ERROR writting in socket\n");
    pandora.close(PND_SOCKET_ERROR);
  }
  printf("i say %s\n", msg);
}

char buffer[255];
void _digest() {
  bzero(buffer, sizeof(char) * 255);
  int isocket = read(pandora.socket, buffer, 255);
  if (isocket < 0) {
    printf("ERROR reading from socket\n");
    pandora.close(PND_SOCKET_ERROR);
  }
  printf("server says: %s\n", buffer);
  PND_MESSAGE msg = { buffer };

  int c = pandora.listenersc - 1;
  while (c--) {
    if (strcmp(buffer, pandora.listeners[c].ev) == 0) {
      pandora.listeners[c].callback(&msg);
    }
  }
}

void _close(int err) {
  close(pandora.socket);
  free(pandora.listeners);
  exit(err);
}
