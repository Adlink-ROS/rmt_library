# Introduction

The tutorial will guide you how to use RMT with zenoh.
Prepare two hosts (one for server and one for agent) and they can communicate with each other without multicast.

# Before you start

* You need to disable the multicast function of the network interface both in server and agent.
  - Assume your interface is `eno1`

```bash
# disable multicast
sudo ip link set eno1 multicast off
```

* Then create `cylconedds.xml` under home directory. This will limit the DDS traffic will only go through the interface.
  - Assume your interface is `eno1`

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<CycloneDDS xmlns="https://cdds.io/config" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="https://cdds.io/config https://raw.githubusercontent.com/eclipse-cyclonedds/cyclonedds/master/etc/cyclonedds.xsd">
    <Domain id="any">
        <General>
            <NetworkInterfaceAddress>eno1</NetworkInterfaceAddress>
        </General>
    </Domain>
</CycloneDDS>
```

* Create `rmt.conf` under home directory. Refer to the following settings.
  - Assume your interface is `eno1`

```
interface: eno1
domain: 0
switch_interface: 0
logfile: stderr
reply_timeout: 3
device_id: 6166
datainfo_size: 256
devinfo_size: 1024
support_zenoh: 1
```

* Modify `option(SUPPORT_ZENOH "Support Zenoh" OFF)` from OFF to ON, and then rebuild RMT.
  - Also note that `devinfo_server_del_device_callback_robot_id` use `devinfo` to check the device alive or not. This is only for FARobot, so you can modify it to `hostname`.

# Run

## Agent side

* terminal 1: Run zenoh bridge

```bash
# Clone code
git clone https://github.com/atolab/zenoh-plugin-dds.git -b FAR
cd zenoh-plugin-dds/zenoh-bridge-dds
# Build
cargo build --release
# Setup environmental variables
export CYCLONEDDS_URI=file://$HOME/cyclonedds.xml
export RUST_LOG=debug
# Run: assume the server IP is 172.16.8.47
./target/release/zenoh-bridge-dds --dds-group-member-id ros-ROScube-I --scope R --dds-group-lease 10 -m peer -e tcp/172.16.8.47:7447 -d 0 --rest-plugin
```

* terminal 2: Run RMT agent

```bash
export RMT_CONFIG=$HOME/rmt.conf
cd rmt_library/build
./examples/rmt-agent/agent/rmt-agent --id 6166
```

## Server side

* terminal 1: Run zenoh bridge

```bash
# Clone code
git clone https://github.com/atolab/zenoh-plugin-dds.git -b FAR
cd zenoh-plugin-dds/zenoh-bridge-dds
# Build
cargo build --release
# Setup environmental variables
export CYCLONEDDS_URI=file://$HOME/cyclonedds.xml
export RUST_LOG=debug
# Run
./target/release/zenoh-bridge-dds --dds-group-member-id FLM --scope R --dds-group-lease 10 -m peer -l tcp/0.0.0.0:7447 -d 0 --rest-plugin
```

* terminal 2: Run RMT server

```bash
export RMT_CONFIG=$HOME/rmt.conf
cd rmt_library/build
./server/server_example
```
