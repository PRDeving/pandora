#include <pandora.h>

PND_HANDLER handle(PND_MESSAGE *msgs) {
  pandora.emit("world!");
}

int loop = 1;
PND_HANDLER stopDigesting(PND_MESSAGE *msgs) {
  loop = 0;
}

int main(int argc, char *argv[]) {
  printf("using %s\n\n", pandora.version);
  pandora.connect("localhost", 1337);
  pandora.info();

  pandora.on("hello", handle);
  pandora.on("stop", stopDigesting);

  /* while (loop) pandora.digest(); */
  sleep(2);
  pandora.digest();

  pandora.close(PND_OK);
  return 0;
}
