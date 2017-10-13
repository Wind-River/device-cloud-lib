Wind River's Internet of Things Solution
========================================

This is the base source code tree for Wind River's Internet of Things (IoT)
solution.  It contains several improvements over previous versions for of the
solution.

Features Included are:
- Portability (ported to multiple operating systems)
- API is operating system independent
- API is can be either threaded or non-threaded
- Dynamic plug-in architecture allowing for add-ons within the library
- Easier device API (reduction in api, makes easier to use for developers)
- Agent support is provided all within a library

Building
--------
```sh
./build-deps.sh
cd build
make
```
