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

#include "timers.h"

#include <pthread.h>
#include <time.h>

static pthread_t th;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static volatile int running = 0;

typedef struct ticker {
  long long next; /* next tick in ms since era */
  int interval;   /* interval in ms */
  void (*callback)(void *);
  void *arg;
  int rep;
} ticker_t;

static void *_loop(void *arg) {
  struct timespec era;
  clock_gettime(CLOCK_REALTIME, &era);
  while (1) {
    pthread_mutex_lock(&lock);
    if (!running) {
      pthread_mutex_unlock(&lock);
      break;
    }
    pthread_mutex_unlock(&lock);
  }
  return NULL;
}

int timers_init(void) {
  int res = 0;
  pthread_mutex_lock(&lock);
  if (pthread_create(&th, NULL, _loop, NULL) < 0) {
    res = -1;
  }
end:
  pthread_mutex_unlock(&lock);
  return res;
}

int timers_cleanup(void) {
  pthread_mutex_lock(&lock);
  if (!running) {
    pthread_mutex_unlock(&lock);
    return 0;
  }
  running = 0;
  pthread_mutex_unlock(&lock);
  pthread_join(th, NULL);
  return 0;
}

int timers_start(int ms, void (*cb)(void *), void *v, int rep, int *id) {
  return 0;
}

int timers_stop(int id) { return 0; }
