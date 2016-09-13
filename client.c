#include <pandora.h>

PND_HANDLER handle(PND_MESSAGE *msgs) {
  printf("Server said %s to connection", msgs -> msg);
}

PND_HANDLER world(PND_MESSAGE *msg) {
  printf("that fucker said %s\n", msg -> msg);
}
int main(int argc, char *argv[]) {
  printf("using %s\n\n", pandora.version);
  pandora.connect("localhost", 1337);
  pandora.info();

  pandora.on("connection", handle);
  pandora.on("hi", world);

  while (1) pandora.digest();

  pandora.close(PND_OK);
  return 0;
}
