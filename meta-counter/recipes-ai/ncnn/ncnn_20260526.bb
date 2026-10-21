SUMMARY = "ncnn neural network inference framework"
HOMEPAGE = "https://github.com/Tencent/ncnn"

LICENSE = "BSD-3-Clause & BSD-2-Clause & Zlib"
LIC_FILES_CHKSUM = "file://LICENSE.txt;md5=d2141d1a2c978a089d3de7a67db0bbd2"

SRC_URI = "https://github.com/Tencent/ncnn/archive/refs/tags/${PV}.tar.gz"
SRC_URI[sha256sum] = "da1ade826bc22858a9fb87ae052789bbd614d042b3ec2c22e6544ca83db6bc04"

S = "${WORKDIR}/ncnn-${PV}"

inherit cmake

EXTRA_OECMAKE += " \
    -DNCNN_BUILD_TOOLS=OFF \
    -DNCNN_BUILD_EXAMPLES=OFF \
    -DNCNN_BUILD_BENCHMARK=OFF \
    -DNCNN_BUILD_TESTS=OFF \
    -DNCNN_VULKAN=OFF \
    -DNCNN_SHARED_LIB=ON \
    -DNCNN_OPENMP=ON \
    -DNCNN_INSTALL_SDK=ON \
"
