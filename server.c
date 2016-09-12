#include <pandora.h>

/* PND_HANDLER world(PND_MESSAGE *msg) { */
/*   printf("that fucker said woeld\n"); */
/*   pandora.emit("stop\0"); */
/* } */
/*  */
/* PND_HANDLER stop(PND_MESSAGE *msg) { */
/*     pandora.close(0); */
/* } */
/*  */
/* void* list(void* arg) { */
/*   pandora.listen(1337); */
/* } */

int main() {
  printf("using %s\n\n", pandora.version);

  /* pandora.on("world!\0", world); */

  /* pandora.listen(1337); */
  /* pandora.info(); */

  PND_deserialize_event("asdfjaowejflsdkfmþ↓ŧæasfiaf39fi3");

  /* sleep(10); */
  pandora.close(0);
  exit(0);
}
