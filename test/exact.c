/* -----------------------------------------------------------------------
 * Timers -- A library to handle tons of timers
 * Copyright (C) 2022  Ali AslRousta <aslrousta@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * -----------------------------------------------------------------------
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <timers.h>

#define N 1000

struct {
  int count;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
} waitgroup = {
    .count = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .cond = PTHREAD_COND_INITIALIZER,
};

static void wg_add(int n) {
  pthread_mutex_lock(&waitgroup.mutex);
  waitgroup.count += n;
  pthread_mutex_unlock(&waitgroup.mutex);
}

static void wg_done(void) {
  pthread_mutex_lock(&waitgroup.mutex);
  waitgroup.count--;
  pthread_cond_signal(&waitgroup.cond);
  pthread_mutex_unlock(&waitgroup.mutex);
}

static void wg_wait(void) {
  pthread_mutex_lock(&waitgroup.mutex);
  while (waitgroup.count > 0) {
    pthread_cond_wait(&waitgroup.cond, &waitgroup.mutex);
  }
  pthread_mutex_unlock(&waitgroup.mutex);
}

static void callback(void *arg) { wg_done(); }

int main(int argc, char *argv[]) {
  int delays[N], expected = 0;
  srand(time(NULL));
  for (int i = 0; i < N; i++) {
    delays[i] = (rand() % 100 + 1) * 10;
    if (delays[i] > expected) expected = delays[i];
  }
  timers_init();
  wg_add(N);
  struct timespec start;
  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < N; i++) {
    timers_start(delays[i], callback, NULL);
  }
  wg_wait();
  struct timespec end;
  clock_gettime(CLOCK_MONOTONIC, &end);
  int actual = (end.tv_sec - start.tv_sec) * 1000 +
               (end.tv_nsec - start.tv_nsec) / 1000000;
  printf("%d timers, expected: %d ms, actual: %d ms", N, expected, actual);
  timers_cleanup();
  return expected > actual ? EXIT_FAILURE : EXIT_SUCCESS;
}
