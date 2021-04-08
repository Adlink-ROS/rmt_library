# Introduction

The repo contains several components:

* RESTful API server
* RMT Server (Server Side)
* RMT Agent (Client Side)

# Install necessary packages

```bash
sudo apt install -y libcunit1-dev swig
# Used by CycloneDDS
sudo apt install maven default-jdk
```

# Download

```bash
cd $HOME
git clone https://github.com/Adlink-ROS/RMT.git
```

# Build

## RMT_core:

```bash
cd ~/RMT/RMT_core
cmake -Bbuild -H.
cmake --build build
```

# Run

* Run RMT_core example
  
```bash
# 1st terminal: Run the agent
cd ~/RMT/RMT_core/build/agent
./agent_example
# 2nd terminal: Run the server
cd ~/RMT/RMT_core/build/server
./server_example
```

* You can also assign id and interface

```bash
# 1st terminal: Run the agent
cd ~/RMT/RMT_core/build/agent
./agent_example --id 6166 --net enp1s0
# 2nd terminal: Run the server
cd ~/RMT/RMT_core/build/server
./server_example --net enp1s0
```

# Test

## CUnit:

```bash
cd ~/RMT/RMT_core/build
ctest
# verbose
ctest -V
```

## Python Wrapper Test:

* Run few agents before testing

```bash
cd ~/RMT/RMT_core/build/agent
./agent_example
```

* Start testing

```bash
cd ~/RMT/RMT_core/build/swig
python3 python_example.py
```
