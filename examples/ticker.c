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

#include <stdio.h>
#include <timers.h>
#include <unistd.h>

void callback(void *arg) {
  printf("tick\n");
  timers_start(1000 /* ms */, callback, NULL);
}

int main() {
  timers_init();
  timers_start(1000 /* ms */, callback, NULL);
  for (;;) {
    sleep(1);
  }
  timers_cleanup();
  return 0;
}
