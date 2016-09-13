#include "pandora.h"

struct Pandora pandora = {
  "0.0.2",    // version (char*)
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

char *PND_serialize_event (char *ev, void* params){
  int totsize = strlen(ev) + strlen((char*)params);
  char *str = malloc(totsize * sizeof(char));
  strcpy(str, ev);
  strcat(str, "þ↓ŧæ");
  strcat(str, params);
  return str;
}

PND_EVENT *PND_deserialize_event (char *serie) {
  PND_EVENT *ev = malloc(sizeof(PND_EVENT));

  serie = strdup(serie);
  printf("serie: %s\n", serie);
  char *piece;

  piece = strtok(serie, "þ↓ŧæ");
    ev -> key = piece;
  piece = strtok(NULL, "þ↓ŧæ");
    ev -> value = piece;

  printf("{ key:%s, value:%s }\n", ev -> key, ev -> value);
  return ev;
}

void* _hostRuntime(){
  pandora.clients = malloc(1 * sizeof(int));
  int maxsd;
  int activity;
  int activesocket;
  socklen_t socketlen;
  int valread;
  char *buffer;
  buffer = malloc(255 * sizeof(char));

  int needsort;
  int c;
  int *temp;

  printf("Waiting for connections...\n");
  while (pandora.status = PND_STATUS_LISTENING) {
    FD_ZERO(&pandora.fdclients);
    FD_SET(pandora.socket, &pandora.fdclients);
    maxsd = pandora.socket;

    c = pandora.clientsc;
    while (c-- > 0) if (pandora.clients[c] > 0) {
      FD_SET(pandora.clients[c], &pandora.fdclients);
      if (pandora.clients[c] > maxsd) maxsd = pandora.clients[c];
    }

    activity = select(maxsd + 1, &pandora.fdclients, NULL, NULL, NULL);

    // Main socket
    if (FD_ISSET(pandora.socket, &pandora.fdclients)) {
      if ((activesocket = accept(pandora.socket, (struct sockaddr *)&pandora.serv_addr, (socklen_t*)&socketlen)) < 0)
        printf("ERROR cant accept connection\n");

      printf("New client connected - fd: %d, ip: %s, port: %d\n", activesocket,
          inet_ntoa(pandora.serv_addr.sin_addr),
          ntohs(pandora.serv_addr.sin_port));

      // emit "connection"
      char *handshake = PND_serialize_event("connection\0", "start\0");
      if( send(activesocket, handshake, strlen(handshake), 0) != strlen(handshake)) printf("ERROR handshake doesnt match\n");

      // push client
      pandora.clientsc++;
      pandora.clients = realloc(pandora.clients, pandora.clientsc * sizeof(int));
      pandora.clients[pandora.clientsc - 1] = activesocket;
    }

    // clients sockets
    c = pandora.clientsc;
    needsort = 0;
    while (c-- > 0) {
      activesocket = pandora.clients[c];
      if (FD_ISSET(activesocket , &pandora.fdclients)) {
        memset(buffer, 0, 255 * sizeof(char));
        valread = read(activesocket, buffer, 255); 

        // Check if disconnection
        if (strlen(buffer) == 0 || valread == 0) {
          getpeername(activesocket , (struct sockaddr*)&pandora.serv_addr , (socklen_t*)&socketlen);
          printf("Host disconnected , ip %s , port %d \n" ,
            inet_ntoa(pandora.serv_addr.sin_addr),
            ntohs(pandora.serv_addr.sin_port));

          close(activesocket);
          pandora.clients[c] = -1;
          needsort++;
        } else { // if not, its a input
          printf("Message recieved: %s\n", buffer);
          PND_EVENT *ev = PND_deserialize_event(buffer);
          PND_MESSAGE msg = { ev -> value };
          int cev = pandora.listenersc - 1;
          while (cev--) {
            if (strcmp(ev -> key, pandora.listeners[cev].ev) == 0) {
              pandora.listeners[cev].callback(&msg);
            }
          }
          free(ev);
        }
      }
    }

    if (needsort > 0) {
        temp = malloc(1);
        c = 0;
        int i = 0;
        while (c < pandora.clientsc) {
            if (pandora.clients[c] != -1) {
                temp = realloc(temp, sizeof(int) * (i + 1));
                memcpy(&temp[i], &pandora.clients[c], sizeof(int));
                i++;
            }
            c++;
        }

        if (i > 0) {
            pandora.clients = realloc(pandora.clients, sizeof(int) * (i + 1));
            memcpy(&pandora.clients, &temp, sizeof(int) * (i + 1));
            pandora.clientsc = i + 1;
        } else {
            pandora.clients = realloc(pandora.clients, 1);
            pandora.clientsc = 0;
        }
        free(temp);
    }
    printf("\n%d clients connected\n\n", pandora.clientsc);
  }
}

int _listen(int port) {
  if (pandora.status > PND_STATUS_DISCONNECTED) {
    printf("ERROR pandora is currently %s\n", pandora.status == PND_STATUS_CONNECTED ? "conected" : "listening");
    return PND_ERROR_INVOCATION;
  }
  if (!port){
    printf("Error connecting\n");
    return PND_ERROR_INVOCATION;
  }
  printf("Opening port %d to listen...", port);

  pandora.port = port;
  pandora.socket = socket(AF_INET, SOCK_STREAM, 0);
  if (pandora.socket < 0) {
    printf("ERROR\ncant get sockfd\n");
    return PND_ERROR_CONNECTION;
  }
  printf("OPEN\n");

  int opt = 1;
  if(setsockopt(pandora.socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
      printf("ERROR\ncant configure socket\n");
      return PND_ERROR_SOCKET;
  }

  bzero((char*) &pandora.serv_addr, sizeof(pandora.serv_addr));
  pandora.serv_addr.sin_family = AF_INET;
  pandora.serv_addr.sin_addr.s_addr = INADDR_ANY;
  pandora.serv_addr.sin_port = htons(pandora.port);

  if (bind(pandora.socket, (struct sockaddr*) &pandora.serv_addr, sizeof(pandora.serv_addr)) < 0 || listen(pandora.socket, 5) < 0) {
    printf("ERROR\ncant bind or listen socket\n");
    return 5;
  }

  pandora.status = PND_STATUS_LISTENING;
  int running = pthread_create(&pandora.hostRuntime, NULL, _hostRuntime, NULL);
  if (running == 0 ) printf("Listener thread is up\n");
  else {
    printf("ERROR setting up listener thread, server is not running\n");
    pandora.status = PND_STATUS_DISCONNECTED;
  }

  return pandora.socket;
}

int _connect(char *host, int port) {
  if (pandora.status > 0) {
    printf("ERROR pandora is currently %s", pandora.status == 1 ? "conected" : "listening\n");
    return PND_ERROR_INVOCATION;
  }
  if (!strlen(host) || !port) {
    printf("Error connecting\n");
    return PND_ERROR_INVOCATION;
  }
  printf("creating connection with %s:%d...\n", host, port);
  pandora.host = host;
  pandora.port = port;
  pandora.socket = socket(AF_INET, SOCK_STREAM, 0);
  pandora.server = gethostbyname(pandora.host);
  if (pandora.socket < 0 || pandora.server < 0) {
    printf("ERROR\ncant get sockfd or can't reach host\n");
    return PND_ERROR_CONNECTION;
  }

  bzero((char*) &pandora.serv_addr, sizeof(pandora.serv_addr));
  pandora.serv_addr.sin_family = AF_INET;
  bcopy((char*) pandora.server -> h_addr, 
        (char*) &pandora.serv_addr.sin_addr.s_addr,
        pandora.server -> h_length);
  pandora.serv_addr.sin_port = htons(pandora.port);

  if (connect(pandora.socket,(struct sockaddr*) &pandora.serv_addr,sizeof(pandora.serv_addr)) < 0) {
    printf("ERROR cant connect\n");
    pandora.close(PND_ERROR_CONNECTION);
  }

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
          pandora.status == PND_STATUS_LISTENING ? "listening" :
          (pandora.status ? "connected" : "disconnected"));
}
int _check(void) {
  return pandora.status > PND_STATUS_DISCONNECTED;
}

void _on(char* ev, PND_HANDLER callback) {
  if (!pandora.listeners) pandora.listeners = malloc(sizeof(PND_LISTENER));
  PND_LISTENER listener = {ev, callback};
  pandora.listeners[pandora.listenersc - 1] = listener;
  pandora.listenersc++;
  pandora.listeners = realloc(pandora.listeners, sizeof(PND_LISTENER) * pandora.listenersc);
}

int osocket;
void _send(int client, char* msg) {
  osocket = write(client, msg, strlen(msg));
  if (osocket < 0) {
    printf("ERROR writting in socket\n");
    /* pandora.close(PND_SOCKET_ERROR); */
  } else
  printf("i say %s to %d\n", msg, client);
}
void _broadcast(char* msg) {
  int c = pandora.clientsc + 1;
  while (c--) {
      printf("\tbroadcast to %d as %d\n", pandora.clients[c], c);
      _send(pandora.clients[c], msg);
  }
}
void _emit(char* key, void* msg) {
  char *serie = PND_serialize_event(key, msg);
  if (pandora.status == PND_STATUS_LISTENING) _broadcast(serie);
  else if (pandora.status == PND_STATUS_CONNECTED) _send(pandora.socket, serie);
}

char *buffer;
void _digest() {
  buffer = malloc(sizeof(char) * 255);
  int isocket = read(pandora.status == PND_STATUS_CONNECTED ? pandora.socket : pandora.clients[0], buffer, 255);
  if (isocket < 0) {
    printf("ERROR reading from socket\n");
    pandora.close(PND_ERROR_SOCKET);
  }
  printf("\n\n\n\tBUFFER: %s\n\n\n", buffer);
  buffer = realloc(buffer, strlen(buffer) * sizeof(char));
  if (strlen(buffer) == 0) pandora.close(PND_OK);

  PND_EVENT *ev = PND_deserialize_event(buffer);
  PND_MESSAGE msg = { ev -> value };

  int c = pandora.listenersc - 1;
  while (c--) {
    if (strcmp(ev -> key, pandora.listeners[c].ev) == 0) {
      pandora.listeners[c].callback(&msg);
    }
  }
  free(ev);
  close(isocket);
  free(buffer);
}

void _close(int err) {
  if (pandora.clientsc > 0)
    while (pandora.clientsc-- > 0) {
      if (pandora.clients[pandora.clientsc] > 0) close(pandora.clients[pandora.clientsc]);
    }
  close(pandora.socket);
  free(pandora.listeners);
  free(pandora.clients);
  exit(err);
}
