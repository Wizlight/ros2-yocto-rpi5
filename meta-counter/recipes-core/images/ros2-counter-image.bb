require recipes-core/images/core-image-base.bb

SUMMARY = "ROS 2 Jazzy image with counter nodes and Yuna detection for Raspberry Pi 5"
DESCRIPTION = "Yocto image containing ROS 2 Jazzy, custom counter nodes, NCNN and Yuna object detection."

LICENSE = "MIT"

inherit ros_distro_jazzy
inherit ros2_image

IMAGE_INSTALL:append = " \
    ros-core \
    counter-nodes \
    ncnn \
    libgomp \
    yuna-live \
    yuna-model \
    libcamera \
    libpisp \
    wifi-setup \
"
