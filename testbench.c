#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sched.h>
#include <assert.h>
#include <limits.h>
#include <signal.h>
#include "cachehog_lib.h"

const uint8_t* message;
int message_size;
uint8_t* recv;
int recc;

void transmitter(const uint8_t* message, size_t size) {
  init_cachehog();

  for(int i = 0; i < size; i++) {
    transmit_word(message[i]);
  }
}

void after_receiving();

void recv_sighandler(int sig) {
    after_receiving();
    _exit(0);
}

void receiver(const uint8_t* original_message, size_t size) {
  init_cachehog();

  // Original null-byte is part of transmitted data and could be corrupted
  // so we have one additional null-byte at the end of the buffer
  recv = malloc(size+1);
  recc = 0;

  struct sigaction sigact = { recv_sighandler };
  sigaction(SIGINT, &sigact, NULL);

  for(int i = 0; i < size; i++) {
    recv[i] = read_word();
    recc++;
  }
}

int calc_error_count() {
  int error_count = 0;
  for(int i = 0; i < message_size; i++) {
    error_count += __builtin_popcount(message[i] ^ recv[i]);
  }
  return error_count;
}

const char* byte_to_binary(int x) {
  static char b[9];
  b[0] = '\0';

  for(int i = 0; i < 8; i++) {
    b[i] = (x & (128 >> i)) ? '1' : '0';
  }

  return b;
}
void show_error_bytes() {
  for(int i = 0; i < message_size; i++) {
    uint8_t diff = message[i] ^ recv[i];
    if(diff != 0) {
      printf("\n    %s\n", byte_to_binary(message[i]));
      printf("    %s\n", byte_to_binary(recv[i]));
      printf("    --------\n");
      printf("xor %s  %d\n", byte_to_binary(diff), __builtin_popcount(diff));
    }
  }
}

void after_receiving() {
  recv[message_size+1] = 0; // additional null-byte at the end for safety

  int error_count = calc_error_count();
  float ber = error_count / ((float)message_size * word_size);

  printf("Received %d bytes (%d bits)\n", recc, recc * word_size);
  printf("Lost %d bytes\n", message_size - recc);
  printf("ORG: %s\n", message);
  printf("REC: %s\n", recv);
  show_error_bytes();
  printf("Total error bits: %d, bit error rate: %f\n", error_count, ber);
}

int main(int argc, char** argv) {
  assert(word_size == sizeof(uint8_t)*8);

  message = (argc > 1) ? argv[1] : "Lorem ipsum dolor sit amet.";
  message_size = strlen(message)+1;

  int est_running_time_sec =
    message_size *
    word_size *
    2 *   // sync byte
    intervals_per_bit *
    base_interval_ms /
    1000; // ms to s

  printf("Message size: %d\n", message_size);
  printf("Estimated running time: %d sec\n", est_running_time_sec);

  pid_t pid = fork();

  if(pid == -1) {
      return EXIT_FAILURE;
  }

  if(pid != 0) {
    sched_yield();
    transmitter(message, message_size);
    kill(pid, SIGINT);
  } else {
    sched_yield();
    receiver(message, message_size);
    raise(SIGINT);
  }

  return 0;
}
