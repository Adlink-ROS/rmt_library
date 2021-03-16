# Introduction

The repo contains several components:

* RESTful API server
* RMT Library (Server Side)
* RMT Engine (Client Side)

# Build

* Build RMT_core

```bash
cd RMT_core
cmake -Bbuild -H.
cmake --build build
```

# Test

* Run test

```bash
cd build
ctest
# verbose
ctest -V
```