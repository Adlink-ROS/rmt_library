# Introduction

The repository is Robot Management Tool library.
The library provides the communication way between server and agent.

Here are some related repository:

* Server side: https://github.com/Adlink-ROS/rmt_web_devkit
* Agent side: https://github.com/Adlink-ROS/RMT_example
* Documentation: https://github.com/Adlink-ROS/RMT-User-Manual

# Official Release Flow (Internal Use)

When we want to update the Robot Management Tool, we should follow the steps here:

1. Add tag in RMT repo and it'll create deb binary to release page on GitHub automatically.
2. Copy the deb binary to rmt_web_devkit and RMT_example. After some modification, add tag to them.
3. Copy the deb library and doxygen result to RMT-User-Manual.
4. Update the documentation on RMT-User-Manual.

# Install necessary packages

```bash
sudo apt install -y libcunit1-dev swig doxygen
# Used by CycloneDDS
sudo apt install bison
# Used by agent example
sudo apt install libnm-dev
```

# Build

```bash
cd $HOME
git clone https://github.com/Adlink-ROS/RMT.git
cd ~/RMT
# If you want to build agent with ROS, run "cmake -Bbuild -H. -DUSE_ROS=ON" instead
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

# Release

* If you want to release RMT Library, you should build with Release mode, which includes license verification in librmt_agent.so.

```bash
cmake -Bbuild -H. -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --build build --target package
```

* License verification
  - If agent is running in ADLINK device, you can use directly.
  - Otherwise, you should create license with neuron-license-manager.

```bash
# Create the license
/opt/neuron-license-manager/bin/license-generator -s <signature> -o NeuronSDK.key NeuronSDK
# $PWD is where you put the license
export ADLINK_LICENSE=$PWD
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

# API Documents

To generate the API documents, please make sure you have installed **doxygen** and **pydoc3** in your system before the next step.

Then, rebuild the source codes with below commands:

```bash
cmake -Bbuild -H. -DBUILD_RMT_DOCS=ON
cmake --build build --target docs
cmake --build build --target all
cmake --build build --target pydoc
```

* C API docs will be placed at `~/RMT/build/docs`

* Python API docs will be placed at `~/RMT/build/swig`

