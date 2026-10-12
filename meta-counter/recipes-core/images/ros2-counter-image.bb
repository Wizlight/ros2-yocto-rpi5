require recipes-core/images/core-image-base.bb

SUMMARY = "ROS 2 Jazzy image with counter nodes for Raspberry Pi 5"
DESCRIPTION = "Yocto image containing ROS 2 Jazzy and custom counter publisher/subscriber nodes."

LICENSE = "MIT"

inherit ros_distro_jazzy
inherit ros2_image

IMAGE_INSTALL:append = " \
    ros-core \
    counter-nodes \
"