#/bin/sh
gcc -shared -o hooked_tcsetattr.so -fPIC tcsetattr_hook.c -ldl
