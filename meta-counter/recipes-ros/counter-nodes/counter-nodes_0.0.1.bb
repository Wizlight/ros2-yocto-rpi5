SUMMARY = "ROS 2 counter publisher and subscriber"
DESCRIPTION = "Simple ROS 2 publisher/subscriber example with a service for starting and stopping the counter."

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit ros_distro_jazzy
inherit ros_component

ROS_CN = "counter_nodes"
ROS_BPN = "counter_nodes"

ROS_BUILD_TYPE = "ament_python"

ROS_EXEC_DEPENDS = "\
    rclpy \
    std-msgs \
    std-srvs \
"

RDEPENDS:${PN} += "${ROS_EXEC_DEPENDS}"

FILESEXTRAPATHS:prepend := "${THISDIR}/../../../ros2_ws/src:"

SRC_URI = "file://counter_nodes"

S = "${WORKDIR}/counter_nodes"

inherit ros_${ROS_BUILD_TYPE}