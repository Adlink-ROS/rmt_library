# Introduction

The repo contains several components:

* RESTful API server
* RMT Server (Server Side)
* RMT Agent (Client Side)

# Install necessary packages

```bash
sudo apt install libcunit1-dev
```

# Donwload

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

# Test

## CTEST:
```bash
cd ~/RMT/RMT_core/build
ctest
# verbose
ctest -V
```

## Python Wrapper Test:
Run few agents before testing
```bash
cd ~/RMT/RMT_core/build/agent
./agent_example
```

Start testing
```bash
cd ~/RMT/RMT_core/build/swig
python3 python_example.py
```
