# Introduction

The repo contains several components:

* RESTful API server
* RMT Server (Server Side)
* RMT Agent (Client Side)

# Install necessary packages

```bash
sudo apt install libcunit1-dev
```

# Build

* Build RMT_core

```bash
cd RMT_core
cmake -Bbuild -H.
cmake --build build
```

# Run

* Run RMT_core example
  
```bash
cd RMT_core
# Run the agent
./build/agent/agent_example
# Run the server
./build/server/server_example
```

# Test

* Run test

```bash
cd build
ctest
# verbose
ctest -V
```
