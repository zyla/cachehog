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

// Record mode main loop
void do_record(int nbits) {
  int i;
  for(i = 0; i < nbits; i++) {
    int value = measure(1);
    printf("%d\n", value);
  }
}

int main(int argc, char **argv) {
  init_cachehog();

  if(argc >= 2 && !strcmp(argv[1], "transmit")) {
    do_transmit();
  } else if(argc >= 2 && !strcmp(argv[1], "receive")) {
    do_receive();
  } else if(argc >= 3 && !strcmp(argv[1], "record")) {
    int nbits = atoi(argv[2]);
    do_record(nbits);
  } else {
    fprintf(stderr, "Usage: %s ( transmit | receive | record <nbits> )\n", argv[0]);
    return 1;
  }

  return 0;
}
