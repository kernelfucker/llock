# llock
logical x display locker

# compile
$ clang llock.c -o llock -Os -lcrypt -lxcb -lxcb-keysyms \# *no need -lX11 lflag*

# hash passwd with yescrypt
use **lethargy** or **mkpasswd** to generate a random hash comptabile with yescrypt

i recommend using **lethargy** because it is more minimal))

lethargy: $ lethargy

mkpasswd: $ mkpasswd --method=yescrypt llock \# replace llock with your own passwd

# usage
**set hashed passwd in config.h and you can change the colors if you want**

$ ./llock
