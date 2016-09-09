# PANDORA
Easy to use TCP socket manager written in c

# HOW DOES IT WORKS?
Pandora wrappes sys/socket.h and netinet/in.h and gives you a simple interface to interact with, so you can connect to a sockets server, register a callback for a signal
 and make it work in just 6 lines of code

# IMPORTANT!!!!
Only Linux is suported, maybe it works in MacOs too, but havn't tried it myself, it has been developed, compiled and tested in Debian 8 x64. 
Other architectures, systems or even distros could be not supported.

# LETS SEE
```c
  #include <pandora.h>

  PND_HANDLER *sayWorld(PND_MESSAGE *msg) { printf("%s World!", msg -> msg); }
  int main() {
    pandora.connect("locahost", 1234);
    pandora.on("hello", sayWorld);
    pandora.digest();
    pandora.close(PND_OK);
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
`gcc main.c -lpandora`

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

