# ROS 2 Jazzy + Yocto for Raspberry Pi 5

Custom Yocto Linux image for Raspberry Pi 5 with ROS 2 Jazzy and a simple publisher/subscriber example.

This project demonstrates:

- ROS 2 installation and official demo nodes on the development machine
- a custom ROS 2 publisher and subscriber
- communication through the `/counter` topic
- inspection of ROS 2 nodes, topics and services
- an optional service for pausing/resuming the counter
- integration of ROS 2 Jazzy into a custom Yocto image
- deployment and verification on Raspberry Pi 5

## Hardware

- Raspberry Pi 5
- microSD card
- Ethernet connection

## Software

- Ubuntu 24.04 / WSL2
- ROS 2 Jazzy
- Yocto Project / Poky Scarthgap
- meta-raspberrypi
- meta-openembedded
- meta-ros
- Python 3 / rclpy

## Repository structure

```text
ros2-yocto-rpi5/
├── layers/
│   ├── poky/
│   ├── meta-openembedded/
│   ├── meta-raspberrypi/
│   └── meta-ros/
├── meta-counter/
│   ├── conf/
│   │   └── layer.conf
│   ├── recipes-core/
│   │   └── images/
│   │       └── ros2-counter-image.bb
│   └── recipes-ros/
│       └── counter-nodes/
│           └── counter-nodes_0.0.1.bb
├── ros2_ws/
│   └── src/
│       └── counter_nodes/
│           ├── counter_nodes/
│           │   ├── publisher.py
│           │   └── subscriber.py
│           ├── package.xml
│           └── setup.py
├── setup-build.sh
└── README.md
```

External Yocto layers are included as Git submodules.

## ROS 2 nodes

The project contains two custom ROS 2 nodes.

### Publisher

`counter_publisher` publishes an incrementing integer once per second to:

```text
/counter
```

Message type:

```text
std_msgs/msg/Int32
```

Example output:

```text
Publishing: 1
Publishing: 2
Publishing: 3
```

### Subscriber

`counter_subscriber` subscribes to `/counter` and prints every received value.

Example output:

```text
Received: 1
Received: 2
Received: 3
```

## Optional service

The publisher also provides:

```text
/counter/set_enabled
```

Service type:

```text
std_srvs/srv/SetBool
```

Pause publishing:

```bash
ros2 service call /counter/set_enabled std_srvs/srv/SetBool "{data: false}"
```

Resume publishing:

```bash
ros2 service call /counter/set_enabled std_srvs/srv/SetBool "{data: true}"
```

The node keeps running while publishing is paused.

---

## Running on the development machine

ROS 2 Jazzy must be installed first.

Source ROS 2:

```bash
source /opt/ros/jazzy/setup.bash
```

Verify the active distribution:

```bash
echo $ROS_DISTRO
```

Expected:

```text
jazzy
```

### Official ROS 2 demo

Terminal 1:

```bash
source /opt/ros/jazzy/setup.bash
ros2 run demo_nodes_cpp talker
```

Terminal 2:

```bash
source /opt/ros/jazzy/setup.bash
ros2 run demo_nodes_cpp listener
```

### Build the custom package

```bash
cd ros2_ws
source /opt/ros/jazzy/setup.bash
colcon build --symlink-install
source install/setup.bash
```

Run the publisher:

```bash
ros2 run counter_nodes counter_publisher
```

Run the subscriber in another terminal:

```bash
source /opt/ros/jazzy/setup.bash
source ros2_ws/install/setup.bash
ros2 run counter_nodes counter_subscriber
```

### Inspect ROS 2 communication

```bash
ros2 node list
ros2 topic list
ros2 topic info /counter
ros2 topic echo /counter
ros2 service list
ros2 service type /counter/set_enabled
```

Expected nodes:

```text
/counter_publisher
/counter_subscriber
```

Expected service type:

```text
std_srvs/srv/SetBool
```

---

# Yocto image

The custom Yocto layer is:

```text
meta-counter
```

The image recipe is:

```text
meta-counter/recipes-core/images/ros2-counter-image.bb
```

The image is based on `core-image-base` and adds:

```text
ros-core
counter-nodes
```

The resulting system contains:

- Yocto/Poky Scarthgap
- ROS 2 Jazzy
- systemd
- Dropbear SSH
- custom `counter_nodes` ROS 2 package

## Clone the repository

Clone with all Git submodules:

```bash
git clone --recurse-submodules https://github.com/Wizlight/ros2-yocto-rpi5.git
cd ros2-yocto-rpi5
```

If the repository was cloned without submodules:

```bash
git submodule update --init --recursive
```

## Configure the Yocto build

Run:

```bash
source setup-build.sh
```

The script:

- initializes the Yocto build directory
- adds the required external layers
- adds `meta-counter`
- configures `raspberrypi5`
- enables systemd
- enables Dropbear SSH
- limits build parallelism for WSL2
- enables `rm_work` to reduce disk usage

Important build settings:

```text
MACHINE = "raspberrypi5"
INIT_MANAGER = "systemd"
BB_NUMBER_THREADS = "6"
PARALLEL_MAKE = "-j 4"
INHERIT += "rm_work"
```

## Build the image

After running `source setup-build.sh`:

```bash
bitbake ros2-counter-image
```

The generated files are placed under:

```text
build/tmp/deploy/images/raspberrypi5/
```

The compressed disk image has a name similar to:

```text
ros2-counter-image-jazzy-raspberrypi5.rootfs-*.wic.bz2
```

> A clean Yocto + ROS 2 build can require a large amount of disk space. `rm_work` is enabled to keep temporary build data under control.

---

# Flashing Raspberry Pi 5

The release image is compressed as `.wic.bz2`.

Extract it:

```bash
bzip2 -dk ros2-counter-image-jazzy-rpi5.wic.bz2
```

This produces:

```text
ros2-counter-image-jazzy-rpi5.wic
```

Write the resulting image to a microSD card with Raspberry Pi Imager or another disk imaging tool.

Insert the microSD card into Raspberry Pi 5 and boot it.

The image includes Dropbear SSH.

Connect with:

```bash
ssh root@<RPI_IP>
```

---

# Verification on Raspberry Pi 5

Verify the operating system:

```bash
cat /etc/os-release
```

Example from the tested image:

```text
ID=poky
NAME="Poky (Yocto Project Reference Distro)"
VERSION="5.0.21 (scarthgap)"
VERSION_ID=5.0.21
VERSION_CODENAME="scarthgap"
```

Load the ROS 2 environment:

```sh
. /opt/ros/jazzy/setup.sh
```

Verify ROS 2:

```bash
echo $ROS_DISTRO
```

Expected:

```text
jazzy
```

Verify that the custom package is installed:

```bash
ros2 pkg list | grep counter
```

Expected:

```text
counter_nodes
```

## Run the publisher

SSH terminal 1:

```sh
. /opt/ros/jazzy/setup.sh
ros2 run counter_nodes counter_publisher
```

Example:

```text
Publishing: 1
Publishing: 2
Publishing: 3
```

## Run the subscriber

SSH terminal 2:

```sh
. /opt/ros/jazzy/setup.sh
ros2 run counter_nodes counter_subscriber
```

Example:

```text
Received: 1
Received: 2
Received: 3
```

## Inspect nodes and topic

SSH terminal 3:

```sh
. /opt/ros/jazzy/setup.sh
ros2 node list
ros2 topic list
ros2 topic info /counter
ros2 topic echo /counter
```

Expected nodes:

```text
/counter_publisher
/counter_subscriber
```

Example topic output:

```text
data: 15
---
data: 16
---
data: 17
---
```

## Test the service

```bash
ros2 service list | grep counter
```

Expected:

```text
/counter/set_enabled
```

Pause publishing:

```bash
ros2 service call /counter/set_enabled std_srvs/srv/SetBool "{data: false}"
```

Resume publishing:

```bash
ros2 service call /counter/set_enabled std_srvs/srv/SetBool "{data: true}"
```

---

# Prebuilt image

A tested Raspberry Pi 5 image is available in GitHub Releases:

https://github.com/Wizlight/ros2-yocto-rpi5/releases/tag/v1.0.0

Release file:

```text
ros2-counter-image-jazzy-rpi5.wic.bz2
```

SHA-256:

```text
f8fe98dd2ce72984d9b7a0bc82bc200a382b7796d483a24bb5cfb6dc2f7a7c83
```

The image was tested on Raspberry Pi 5 with:

- ROS 2 Jazzy
- `counter_publisher`
- `counter_subscriber`
- `/counter` topic communication
- `/counter/set_enabled` service
- SSH access through Dropbear

## License

MIT
