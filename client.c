#include <pandora.h>

PND_HANDLER handle(PND_MESSAGE *msgs) {
  pandora.emit("world!\0");
}

int loop = 1;
PND_HANDLER stopDigesting(PND_MESSAGE *msgs) {
  printf("eysay stop\n");
  loop = 0;
  pandora.emit("stop\0");
}

int main(int argc, char *argv[]) {
  printf("using %s\n\n", pandora.version);
  pandora.connect("localhost", 1337);
  pandora.info();

  pandora.on("connection\0", handle);
  pandora.on("stop\0", stopDigesting);

  while (loop) pandora.digest();

  pandora.close(PND_OK);
  return 0;
}
