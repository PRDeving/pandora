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
  printf("OPEN\n");

  int opt = 1;
  if(setsockopt(pandora.socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
      printf("ERROR\ncant configure socket\n");
      return PND_SOCKET_ERROR;
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

  printf("Waiting for connection...\n");
  while (1) {
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
      if ((activesocket = accept(pandora.socket, (struct sockaddr *)&pandora.serv_addr, (socklen_t*)&socketlen)) < 0){
        printf("ERROR cant accept connection\n");
        return PND_SOCKET_ERROR;
      }

      printf("New connection , socket fd: %d , ip: %s , port: %d \n" , activesocket , inet_ntoa(pandora.serv_addr.sin_addr) , ntohs(pandora.serv_addr.sin_port));

      // emit "connection"
      if( send(activesocket, "connection\0", 11, 0) != 11 ) {
        printf("ERROR handshake doesnt match\n");
        return PND_SOCKET_ERROR;
      }

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
          PND_MESSAGE msg = { buffer };
          int cev = pandora.listenersc - 1;
          while (cev--) {
            if (strcmp(buffer, pandora.listeners[cev].ev) == 0) {
              pandora.listeners[cev].callback(&msg);
            }
          }
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
      printf("broadcast to %d as %d", pandora.clients[c], c);
      _send(pandora.clients[c], msg);
  }
}
void _emit(char* msg) {
  if (pandora.status == 2) _broadcast(msg);
  else if (pandora.status == 1) _send(pandora.socket, msg);
}

char *buffer;
void _digest() {
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
  close(isocket);
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
