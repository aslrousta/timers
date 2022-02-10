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

#include <limits.h>
#include <pthread.h>
#include <time.h>

typedef long long duration_t; /* in ms */
typedef struct timer {
  duration_t dt;
  void (*cb)(void *);
  void *v;
} timer_t;

#define MAX_TIMERS_PER_HEAP 1000
typedef struct heap {
  timer_t timers[MAX_TIMERS_PER_HEAP];
  int size;
} heap_t;

static void heap_push(heap_t *h, timer_t *t) {
  int i = h->size;
  while (i > 0) {
    int p = (i - 1) / 2; /* parent */
    if (h->timers[p].dt <= t->dt) break;
    h->timers[i] = h->timers[p];
    i = p;
  }
  h->timers[i] = *t;
  h->size++;
}

static void heap_pop(heap_t *h) {
  h->timers[0] = h->timers[--h->size];
  int i = 0;
  while (i < h->size) {
    int l = i * 2 + 1; /* left child */
    int r = i * 2 + 2; /* right child */
    int min = i;
    if (l < h->size && h->timers[l].dt < h->timers[min].dt) min = l;
    if (r < h->size && h->timers[r].dt < h->timers[min].dt) min = r;
    if (min == i) break;
    timer_t tmp = h->timers[i];
    h->timers[i] = h->timers[min];
    h->timers[min] = tmp;
    i = min;
  }
}

static timer_t *heap_min(heap_t *h) {
  if (h->size == 0) return NULL;
  return &h->timers[0];
}

static heap_t heap;
static pthread_t th;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int running = 0;
struct timespec era;

/* computes the passed time since era in milliseconds */
static duration_t passed() {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return (now.tv_sec - era.tv_sec) * 1000LL +
         (now.tv_nsec - era.tv_nsec) / 1000000;
}

static void *loop(void *arg) {
  while (1) {
    pthread_mutex_lock(&lock);
    if (!running) {
      pthread_mutex_unlock(&lock);
      break;
    }
    timer_t *t = heap_min(&heap);
    if (!t || t->dt > passed()) {
      pthread_mutex_unlock(&lock);
      struct timespec nap = {0, 10000000}; /* 10 ms */
      nanosleep(&nap, NULL);
    } else {
      void (*cb)(void *) = t->cb;
      void *v = t->v;
      heap_pop(&heap);
      pthread_mutex_unlock(&lock);
      if (cb) cb(v);
    }
  }
  return NULL;
}

int timers_init(void) {
  pthread_mutex_lock(&lock);
  if (running) {
    pthread_mutex_unlock(&lock);
    return 0;
  }
  if (pthread_create(&th, NULL, loop, NULL) < 0) {
    pthread_mutex_unlock(&lock);
    return -1;
  }
  clock_gettime(CLOCK_MONOTONIC, &era);
  heap.size = 0;
  running = 1;
  pthread_mutex_unlock(&lock);
  return 0;
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

int timers_start(int ms, void (*cb)(void *), void *v) {
  timer_t t = {.dt = passed() + ms, .cb = cb, .v = v};
  pthread_mutex_lock(&lock);
  if (!running) {
    pthread_mutex_unlock(&lock);
    return -1;
  }
  heap_push(&heap, &t);
  pthread_mutex_unlock(&lock);
  return 0;
}
