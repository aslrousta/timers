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
#include <stdlib.h>
#include <time.h>

/* -----------------------------------------------------------------------
 * Timer
 * -----------------------------------------------------------------------
 */

/* keeps time intervals in milliseconds */
typedef long long duration_t;

/* the beginning time for all events */
struct timespec era;

/* computes the time passed in milliseconds */
static duration_t passed() {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return (now.tv_sec - era.tv_sec) * 1000LL +
         (now.tv_nsec - era.tv_nsec) / 1000000;
}

typedef struct timer {
  duration_t dt;      /* the expire time in milliseconds */
  void (*cb)(void *); /* the function to be called when the timer expires */
  void *v;            /* the callback function argument */
} timer_t;

/* -----------------------------------------------------------------------
 * Min-heap
 * -----------------------------------------------------------------------
 */

#define TIMERS_PER_CHUNK 100
#define MAX_CHUNKS 1000

typedef struct chunk {
  timer_t timers[TIMERS_PER_CHUNK];
  int nt;
} chunk_t;

typedef struct heap {
  chunk_t *chunks[MAX_CHUNKS];
  int nc, nt;
} heap_t;

static inline timer_t *at(heap_t *h, int n) {
  int c = n / TIMERS_PER_CHUNK;
  int i = n % TIMERS_PER_CHUNK;
  return c < h->nc ? &h->chunks[c]->timers[i] : NULL;
}

static inline void hpush(heap_t *h, timer_t *t) {
  int c = h->nt / TIMERS_PER_CHUNK;
  int i = h->nt % TIMERS_PER_CHUNK;
  if (c >= h->nc) {
    h->chunks[c] = malloc(sizeof(chunk_t));
    h->chunks[c]->nt = 0;
    h->nc++;
  }
  h->chunks[c]->timers[i] = *t;
  h->chunks[c]->nt++;
  h->nt++;
}

static inline void hpop(heap_t *h) {
  int c = (h->nt - 1) / TIMERS_PER_CHUNK;
  int i = (h->nt - 1) % TIMERS_PER_CHUNK;
  h->chunks[0]->timers[0] = h->chunks[c]->timers[i];
  h->chunks[c]->nt--;
  h->nt--;
}

static inline void swap(timer_t *ti, timer_t *tj) {
  timer_t t = *ti;
  *ti = *tj;
  *tj = t;
}

/* adds a timer to the heap */
static void hadd(heap_t *h, timer_t *t) {
  timer_t *ti, *tp;
  int i, p;
  hpush(h, t);
  i = h->nt - 1, ti = at(h, i);
  while (i > 0) {
    p = (i - 1) / 2, tp = at(h, p);
    if (tp->dt <= ti->dt) break;
    swap(ti, tp);
    ti = tp, i = p;
  }
}

/* removes the min timer from the heap */
static void hdel(heap_t *h) {
  timer_t *tl, *tr, *ti, *tm;
  int l, r, m, i = 0;
  hpop(h);
  ti = at(h, i);
  while (i < h->nt) {
    l = i * 2 + 1, tl = at(h, l);
    r = i * 2 + 2, tr = at(h, r);
    m = i;
    if (l < h->nt && tl->dt < ti->dt) m = l, tm = tl;
    if (r < h->nt && tr->dt < ti->dt) m = r, tm = tr;
    if (m == i) break;
    swap(ti, tm);
    ti = tm, i = m;
  }
}

/* returns the min timer from the heap without removing it */
static timer_t *hmin(heap_t *h) {
  if (h->nt == 0) return NULL;
  return &h->chunks[0]->timers[0];
}

/* -----------------------------------------------------------------------
 * Event loop
 * -----------------------------------------------------------------------
 */

static heap_t heap = {0};
static pthread_t th;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int running = 0;

const struct timespec nap = {0, 10000000}; /* 10 ms */

static void *eventloop(void *arg) {
  while (1) {
    pthread_mutex_lock(&lock);
    if (!running) {
      pthread_mutex_unlock(&lock);
      break;
    }
    timer_t *t = hmin(&heap);
    if (!t || t->dt > passed()) {
      pthread_mutex_unlock(&lock);
      nanosleep(&nap, NULL);
    } else {
      void (*cb)(void *) = t->cb;
      void *v = t->v;
      hdel(&heap);
      pthread_mutex_unlock(&lock);
      if (cb) cb(v);
    }
  }
  return NULL;
}

/* -----------------------------------------------------------------------
 * Public API
 * -----------------------------------------------------------------------
 */

int timers_init(void) {
  pthread_mutex_lock(&lock);
  if (running) {
    pthread_mutex_unlock(&lock);
    return 0;
  }
  if (pthread_create(&th, NULL, eventloop, NULL) < 0) {
    pthread_mutex_unlock(&lock);
    return -1;
  }
  clock_gettime(CLOCK_MONOTONIC, &era);
  heap.nt = 0;
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
  hadd(&heap, &t);
  pthread_mutex_unlock(&lock);
  return 0;
}
