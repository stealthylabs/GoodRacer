# GoodRacer


## DEBUGGING

To load the `goodracer` executable into `gdb` or `valgrind` use the `libtool` script created
in the repository. It will find all the required libraries and will load the
executable correctly.

```bash
## running executable under gdb
$ ./libtool --mode=execute gdb --args ./src/goodracer

## running executable under valgrind
$ ./libtool --mode=execute valgrind --tool=memcheck ./src/goodracer
```
