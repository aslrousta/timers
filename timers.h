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

#ifndef __TIMERS_INCLUDED
#define __TIMERS_INCLUDED

/**
 * Initializes timers.
 *
 * @return 0 on success, -1 on error
 *
 * @remarks It must be called before any other function in this library.
 * @see timers_cleanup()
 */
int timers_init(void);

/**
 * Destroys all timers and frees all resources.
 *
 * @return 0 on success, -1 on error
 *
 * @see timers_init()
 */
int timers_cleanup(void);

/**
 * Starts a timer that will expire after the specified time interval.
 *
 * @param ms    The interval of the timer in milliseconds
 * @param cb    The callback to call when the timer expires
 * @param v     The data to pass to the callback function
 *
 * @return 0 on success, -1 on error
 */
int timers_start(int ms, void (*cb)(void *), void *v);

#endif /* __TIMERS_INCLUDED */
