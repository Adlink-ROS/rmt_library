# Introduction

The repo contains two components:

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

```bash
cd ~/RMT
cmake -Bbuild -H.
cmake --build build
```

# Run

* Run example
  
```bash
# 1st terminal: Run the agent
cd ~/RMT/build/examples/RMT_example/agent
./agent_example
# 2nd terminal: Run the server
cd ~/RMT/build/server
./server_example
```

* You can also assign id and interface

```bash
# 1st terminal: Run the agent
cd ~/RMT/build/examples/RMT_example/agent
./agent_example --id 6166 --net enp1s0
# 2nd terminal: Run the server
cd ~/RMT/build/server
./server_example --net enp1s0
```

* Run multiple agents

```bash
cd ~/RMT/build/examples/RMT_example/agent
# invoke 5 agents with device ID from 20 to 24
./multi_agents.py -n 5 -s 20
```

# Install

* Build pacakge

```bash
cd ~/RMT/build
make package
# it'll create rmt_library_0.9.0_amd64.deb in build folder
```

* Install package

```bash
sudo apt install ./rmt_library_0.9.0_amd64.deb
```

* Uninstall

```bash
sudo apt remove ./rmt_library
```

# Test

## CUnit:

```bash
cd ~/RMT/build
ctest
# verbose
ctest -V
```

## Python Wrapper Test:

* Run few agents before testing

```bash
cd ~/RMT/build/agent
./agent_example
```

* Start testing

```bash
cd ~/RMT/build/swig
python3 python_example.py
```
