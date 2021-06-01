# Introduction

The repository is Robot Management Tool library.
The library provides the communication way between server and agent.

You can use the repo with other two projects

* Server side: https://github.com/Adlink-ROS/rmt_web_devkit
* Agent side: https://github.com/Adlink-ROS/RMT_example

# Install necessary packages

```bash
sudo apt install -y libcunit1-dev swig
# Used by CycloneDDS
sudo apt install maven default-jdk
# Used by agent example
sudo apt install libnm-dev
```

# Download

```bash
cd $HOME
git clone https://github.com/Adlink-ROS/RMT.git
```

# Build

```bash
cd ~/RMT
# If you don't want to build agent with ROS, run "cmake -Bbuild -H. -DUSE_ROS=off" instead
cmake -Bbuild -H.
cmake --build build
# If you want to build deb file, run the command and find deb file in build folder
cmake --build build --target package
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
