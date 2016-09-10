# PANDORA
Easy to use many to one TCP socket manager written in c from server to clientside

# HOW DOES IT WORKS?
Pandora wrappes sys/socket.h and netinet/in.h and gives you a simple interface to interact with, so you can connect to a sockets server, register a callback for a signal
 and make it work in just 6 lines of code

# IMPORTANT!!!!
Only Linux is suported, maybe it works in MacOs too, but havn't tried it myself, it has been developed, compiled and tested in Debian 8 x64 and Ubuntu 16.4 x64. 
Other architectures, systems or even distros could be not supported.

# LETS SEE
#### server.c
```c
  #include <pandora.h>

  PND_HANDLER hello(PND_MESSAGE *msg) {
    printf("client said %s\n", msg -> msg); // hello
    pandora.emit("world");
    pandora.close(0);
  }

  int main() {
    pandora.on("hello", hello);

    pandora.listen(1337);
    
    pandora.close(0);
    exit(0);
  }
```

#### client.c
```c
  #include <pandora.h>

  PND_HANDLER world(PND_MESSAGE *msg) {
    printf("server said %s", msg -> msg); // world
    pandora.close(0);
  }
  PND_HANDLER connection(PND_MESSAGE *msg) {
    printf("Conected to serveri\n");
    pandora.emit("hello");
  }
  int main() {
    pandora.on("world", world);
    pandora.on("connection", connection);

    pandora.connect("locahost", 1234);

    while (1) pandora.digest();

    pandora.close(PND_OK);
    exit(0);
  }
```

# BUILD AND INSTALL
### easy
With this method pandora will be compiled and installed as static library

clone or download
```
$ cd pandora
$ make
$ sudo make install
```

to compile your program you will need to import pandora.h and compile with
`gcc main.c -lpandora .lpthread`
*pthread is needed too, will be included somehow soon*

### hard (use to compile as shared or with your own conf)
clone or download
```
# this compiles as static library (same as make)
$ gcc -c src/pandora.c -o libpandora.o
$ ar rcs libpandora.a libpandora.o

# this compiles as shared
$ gcc -c -fPIC src/pandora.c -o libpandora.o
$ gcc -shared -Wl,-soname,libpandora.so.1 -o libpandora.so.1.0.1 libpandora.o
```

# METHODS
### pandora.connect(char* host, int port)
opens a stream to the server, it returns -1 if something went wrong and the id of the socket if the connection is setted

### pandora.listen(int port)
starts listening port and fires registered pandora.on() events
#### Update
- Raises a thread, main process isnt freezed anymore

### pandora.check(void)
returns true if it's connected and false if not

### pandora.info(void)
returns pandora's info

### pandora.on(char* ev, *PND_HANDLER handler)
register a callback (handler) that will be fired when ev matches with the message sent by the server

### pandora.emit(char* msg)
emits a message throught the socket

### pandora.digest(void)
reads the socket and gets the last message, it has to be called every time you need to connect, it can be in the main loop aswell

### pandora.close(int status)
closes the socket and free the memory, the param status is the return signal on exit, usually PND_OK or 0


# CHANGELOG
[0.0.2] New supports multiple clients
- Now pandora.listen(int) raises a thread, so the main process is not blocked anymore

#ISSUES
    -   some sessions aint killed appropiatelly, will be fixed in next patch
    -   sometimes sessions freezes, will be fixed in next patch
    -   messages are strings, will be accept parameters in next min-update
    -   messages only can be char\*, will accept also int in next min-update
    -   server -> client comunication is by broadcasting rigth now, new direct method will be added in next patch
