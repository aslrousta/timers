# libtimers

**libtimers** is a native library to handle tons of timers.

_Note_: This is a work in progress.

## Getting Started

The following code demonstrates an example which uses _libtimers_ to print `tick` every second.

```c
#include <stdio.h>
#include <timers.h>

void callback(void *arg) {
    printf("> tick\n");
    timers_start(1000 /* ms */, callback, NULL);
}

int main() {
    timers_init();
    timers_start(1000 /* ms */, callback, NULL);
    // ... do other time-consuming stuff
    timers_cleanup();
    return 0;
}
```

Now, you can build and run the program.

```bash
$ gcc -o ticker ticker.c -ltimers
$ ./ticker
> tick
> tick
> ...
```

## To Do

- [ ] Extend to above 1k timers
- [ ] Support for Windows threads
- [ ] Tickers (repeating timers)
- [ ] Reduce memory footprint

## License

_libtimers_ is published under [GPL-3.0](LICENSE) license.
