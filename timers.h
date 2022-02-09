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
 * Initializes the library.
 *
 * It must be called before any other function in this library.
 *
 * @return 0 on success, -1 on error
 *
 * @remarks This function is thread-safe.
 * @see timers_cleanup()
 */
int timers_init(void);

/**
 * Deinitializes the library.
 *
 * It destroys all timers and frees all resources.
 *
 * @return 0 on success, -1 on error
 *
 * @remarks This function is thread-safe.
 * @see timers_init()
 */
int timers_cleanup(void);

/**
 * Start a timer.
 *
 * Creates a timer that will expire after the specified time. The timer is
 * started automatically and can be stopped by calling @c timers_stop()
 * function.
 *
 * @param ms    The interval of the timer in milliseconds
 * @param cb    The callback to call when the timer expires
 * @param v     The data to pass to the callback function
 * @param rep   Whether the timer should repeat or not
 * @param id    A pointer to store the timer id
 *
 * @return 0 on success, -1 on error
 *
 * @remarks This function is thread-safe.
 * @see timers_stop()
 */
int timers_start(int ms, void (*cb)(void *), void *v, int rep, int *id);

/**
 * Stop a timer
 *
 * Stops a timer that was started by @c timers_start() function.
 *
 * @param id    The id of the timer to stop
 *
 * @return 0 on success, -1 on error
 *
 * @remarks This function is thread-safe.
 * @see timers_start()
 */
int timers_stop(int id);

#endif /* __TIMERS_INCLUDED */
