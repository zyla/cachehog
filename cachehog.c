#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

// size of random access buffer
const int N = 1024 * 1024 * 10;

const int base_interval_ms = 5;
const int intervals_per_bit = 10;

const int word_size = 8;

const int DEBUG = 0;

int cmp_timespec(struct timespec *a, struct timespec *b) {
  if(a->tv_sec < b->tv_sec) {
    return -1;
  }

  if(a->tv_sec > b->tv_sec) {
    return 1;
  }

  if(a->tv_nsec < b->tv_nsec) {
    return -1;
  }

  if(a->tv_nsec > b->tv_nsec) {
    return 1;
  }

  return 0;
}

void normalize(struct timespec *val) {
  while(val->tv_nsec >= 1000000000) {
    val->tv_sec++;
    val->tv_nsec -= 1000000000;
  }
}

uint32_t *buf;

int measure(int bit_to_transmit) {
  struct timespec end;
  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  end.tv_nsec += base_interval_ms * 1000000;
  normalize(&end);

  int count = 0;

  while(1) {
    struct timespec cur;
    clock_gettime(CLOCK_MONOTONIC_RAW, &cur);
    if(cmp_timespec(&end, &cur) <= 0)
      break;

    if(bit_to_transmit) {
      for(int j = 0; j < 1000; j++) {
        buf[rand() % N]++;
      }
    }
    count++;
  }
  return count;
}

void read_bit(int *readings, int nbits) {
  int val = measure(1);
  memmove(readings, readings + 1, (nbits - 1) * sizeof(int));
  readings[nbits-1] = val;
}

void threshold(int *readings, int *bits, int nbits) {
  int sum = 0;
  for(int i = 0; i < nbits; i++) {
    int v = readings[i];
    sum += v;
  }
  int avg = sum / nbits;

  for(int i = 0; i < nbits; i++) {
    bits[i] = readings[i] < avg ? 1 : 0;
  }
}

int hamming_distance_from_sync(int *bits, int nbits) {
  int distance = 0;
  for(int i = 0; i < nbits; i++) {
    int desired = (i / intervals_per_bit + 1) & 1;
    if(bits[i] != desired) {
      distance++;
    }
  }
  return distance;
}

void transmit(int bit) {
  for(int j = 0; j < intervals_per_bit; j++) {
    measure(bit);
  }
}

// Transmit mode main loop
void do_transmit() {
  while(1) {
    int c = getchar();
    if(c == EOF) {
      break;
    }

    // sync (10101010)
    for(int i = 0; i < word_size; i++) {
      transmit((i + 1) & 1);
    }

    // transmit the word
    for(int i = 0; i < word_size; i++) {
      transmit((c & 0x80) ? 1 : 0);
      c <<= 1;
    }
  }
}

// Receive mode main loop
void do_receive() {
  int nbits = intervals_per_bit * word_size;
  int readings[nbits];
  memset(readings, 0, sizeof(readings));

  while(1) {
    read_bit(readings, nbits);

    int bits[nbits];
    threshold(readings, bits, nbits);

    int distance = hamming_distance_from_sync(bits, nbits);

    if(distance < word_size) {
      if(DEBUG) {
        for(int i = 0; i < nbits; i++) {
          printf("%c", bits[i] + '0');
        }
        printf(" %d\n", distance);
      }

      for(int i = 0; i < nbits; i++) {
        read_bit(readings, nbits);
      }

      threshold(readings, bits, nbits);

      int word[word_size];
      for(int i = 0; i < word_size; i++) {
        int count = 0;
        for(int j = 0; j < intervals_per_bit; j++) {
          count += bits[intervals_per_bit * i + j];
        }
        word[i] = count * 2 >= intervals_per_bit ? 1 : 0;
      }

      if(DEBUG) {

        for(int i = 0; i < word_size; i++) {
          printf("%c", word[i] + '0');
        }
        printf("\n");

      } else {
        int val = 0;
        for(int i = 0; i < word_size; i++) {
          val <<= 1;
          val |= word[i];
        }
        putchar(val);
        fflush(stdout);
      }
    }
  }
}

int main(int argc, char **argv) {
  srand(time(NULL));
  buf = malloc(N * sizeof(uint32_t));
  memset(buf, 77,  N * sizeof(uint32_t));

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
