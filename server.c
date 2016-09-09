#include <pandora.h>

int main() {
  printf("using %s\n\n", pandora.version);
  pandora.listen(1337);
  pandora.info();

  pandora.emit("hello\0");
  /* sleep(2); */
  pandora.emit("stop\0");
  
  pandora.close(0);
  exit(0);
}
