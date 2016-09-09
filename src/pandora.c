#include "pandora.h"

struct Pandora pandora = {
  "0.0.1",    // version (char*)
  0,          // status (int) 0 disconected, 1 connected, 2 listening
  NULL,       // host (char*)
  0,          // port (int)
  -1,         // socket (int)
  1,          // listenersc (int)
  0,          // clientsc (int)
  NULL,       // server (struct hostent*)

  _info,      // void _info
  _check,     // int _check
  _listen,    // int _listen
  _connect,   // int _connect
  _on,        // void _on
  _emit,      // void _emit
  _digest,    // void _digest
  _close,     // int _close
};

int _listen(int port) {
  if (pandora.status > 0) {
    printf("ERROR pandora is currently %s", pandora.status == 1 ? "conected" : "listening");
    return PND_INVOCATION_ERROR;
  }
  if (!port){
    printf("Error connecting\n");
    return PND_INVOCATION_ERROR;
  }
  printf("Opening port %d to listen...", port);

  pandora.port = port;
  pandora.socket = socket(AF_INET, SOCK_STREAM, 0);
  if (pandora.socket < 0) {
    printf("ERROR\ncant get sockfd\n");
    return PND_CONNECTION_ERROR;
  }

  bzero((char*) &pandora.serv_addr, sizeof(pandora.serv_addr));
  pandora.serv_addr.sin_family = AF_INET;
  pandora.serv_addr.sin_addr.s_addr = INADDR_ANY;
  pandora.serv_addr.sin_port = htons(pandora.port);

  if (bind(pandora.socket, (struct sockaddr*) &pandora.serv_addr, sizeof(pandora.serv_addr)) < 0 || listen(pandora.socket, 5) < 0) {
    printf("ERROR\ncant bind or listen socket\n");
    return 5;
  }

  pandora.status = 2;
  printf("OPEN\n");
  printf("Waiting for connection...");
  pandora.clientsc++;
  pandora.clients = malloc(pandora.clientsc * sizeof(int));
  pandora.clients_addr = malloc(pandora.clientsc * sizeof(struct sockaddr_in));
  socklen_t clientlen;
  pandora.clients[pandora.clientsc - 1] = accept(pandora.socket, (struct sockaddr*) &pandora.clients_addr[pandora.clientsc - 1], &clientlen);
  if (pandora.clients[pandora.clientsc - 1] < 0) {
    printf("ERROR\nclient conection failed");
    pandora.close(PND_CONNECTION_ERROR);
  }
  printf("CLIENT CONNECTED\n");

  return pandora.socket;
}

int _connect(char *host, int port) {
  if (pandora.status > 0) {
    printf("ERROR pandora is currently %s", pandora.status == 1 ? "conected" : "listening");
    return PND_INVOCATION_ERROR;
  }
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

  pandora.status = 1;
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
          pandora.status == 2 ? "listening" : (pandora.status ? "connected" : "disconnected"));
}
int _check(void) {
  return pandora.status > 0;
}

void _on(char* ev, PND_HANDLER callback) {
  PND_LISTENER listener = {ev, callback};
  pandora.listeners[pandora.listenersc - 1] = listener;
  pandora.listenersc++;
  pandora.listeners = realloc(pandora.listeners, sizeof(PND_LISTENER) * pandora.listenersc);
}

void _emit(char* msg) {
  int osocket = write(pandora.status == 1 ? pandora.socket : pandora.clients[0], msg, strlen(msg));
  if (osocket < 0) {
    printf("ERROR writting in socket\n");
    pandora.close(PND_SOCKET_ERROR);
  }
  printf("i say %s\n", msg);
}

char *buffer;
void _digest() {
  /* if (pandora.status == 2) { */

  /* } else { */
    buffer = malloc(sizeof(char) * 255);
    int isocket = read(pandora.status == 1 ? pandora.socket : pandora.clients[0], buffer, 255);
    if (isocket < 0) {
      printf("ERROR reading from socket\n");
      pandora.close(PND_SOCKET_ERROR);
    }
    buffer = realloc(buffer, strlen(buffer) * sizeof(char));
    if (strlen(buffer) == 0) pandora.close(PND_OK);
    printf("server says: %s\n", buffer);
    PND_MESSAGE msg = { buffer };

    int c = pandora.listenersc - 1;
    while (c--) {
      if (strcmp(buffer, pandora.listeners[c].ev) == 0) {
        pandora.listeners[c].callback(&msg);
      }
    }
  /* } */
}

void _close(int err) {
  if (pandora.clientsc > 0)
    while (pandora.clientsc-- > -1) {
      close(pandora.clients[pandora.clientsc - 1]);
    }
  close(pandora.socket);
  free(pandora.listeners);
  free(pandora.clients);
  free(pandora.clients_addr);
  exit(err);
}
