#!/usr/bin/env bash

set -e

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT_DIR"

source layers/poky/oe-init-build-env build >/dev/null

add_layer() {
    local layer="$1"

    if ! grep -Fq "$layer" conf/bblayers.conf; then
        bitbake-layers add-layer "$layer"
    fi
}

add_layer "$ROOT_DIR/layers/meta-openembedded/meta-oe"
add_layer "$ROOT_DIR/layers/meta-openembedded/meta-python"
add_layer "$ROOT_DIR/layers/meta-raspberrypi"
add_layer "$ROOT_DIR/layers/meta-ros/meta-ros-common"
add_layer "$ROOT_DIR/layers/meta-ros/meta-ros2"
add_layer "$ROOT_DIR/layers/meta-ros/meta-ros2-jazzy"
add_layer "$ROOT_DIR/meta-counter"

append_config() {
    local line="$1"

    if ! grep -Fqx "$line" conf/local.conf; then
        echo "$line" >> conf/local.conf
    fi
}

append_config 'MACHINE = "raspberrypi5"'
append_config 'INIT_MANAGER = "systemd"'
append_config 'LICENSE_FLAGS_ACCEPTED = "synaptics-killswitch"'
append_config 'EXTRA_IMAGE_FEATURES:append = " ssh-server-dropbear"'
append_config 'BB_NUMBER_THREADS = "6"'
append_config 'PARALLEL_MAKE = "-j 4"'
append_config 'INHERIT += "rm_work"'

echo
echo "Yocto build environment configured."
echo "Build image with:"
echo "  bitbake ros2-counter-image"
