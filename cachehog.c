#include "cachehog_lib.h"
#include <stdio.h>

// Transmit mode main loop
void do_transmit() {
  while(1) {
    int c = getchar();
    if(c == EOF) {
      break;
    }

    transmit_word(c);
  }
}

// Receive mode main loop
void do_receive() {
  while(1) {
    int word = read_word();
    if(word != 0) {
      putchar(word);
      fflush(stdout);
    }
  }
}

int main(int argc, char **argv) {
  init_cachehog();

  if(argc >= 2 && !strcmp(argv[1], "transmit")) {
    do_transmit();
  } else if(argc >= 2 && !strcmp(argv[1], "receive")) {
    do_receive();
  } else {
    fprintf(stderr, "Usage: %s ( transmit | receive )\n", argv[0]);
    return 1;
  }

  return 0;
}
