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

* You can also assign id and interface

```bash
cd RMT_core
# Run the agent
./build/agent/agent_example --id 6166 --net enp1s0
# Run the server
./build/server/server_example --net enp1s0
```

# Test

* Run test

```bash
cd build
ctest
# verbose
ctest -V
```
