#include <pandora.h>
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

  pandora.listen(1337);

  sleep(5);
  pandora.emit("hi", "sucker pollens");
  
  sleep(10);
  pandora.close(0);
  exit(0);
}
