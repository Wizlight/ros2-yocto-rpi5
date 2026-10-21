SUMMARY = "Realtime Yuna detector using NCNN"
LICENSE = "CLOSED"

SRC_URI = " \
    file://yuna-live.cpp \
    file://yuna-live.service \
    file://yuna-camera.service \
"

S = "${WORKDIR}"

DEPENDS = "ncnn"

RDEPENDS:${PN} += "yuna-model libcamera"

inherit systemd

SYSTEMD_SERVICE:${PN} = "yuna-live.service yuna-camera.service"
SYSTEMD_AUTO_ENABLE = "enable"

do_compile() {
    ${CXX} ${CXXFLAGS} ${LDFLAGS} \
        -std=c++17 \
        -I${STAGING_INCDIR}/ncnn \
        ${S}/yuna-live.cpp \
        -o yuna-live \
        -lncnn
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 yuna-live ${D}${bindir}/yuna-live

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/yuna-live.service \
        ${D}${systemd_system_unitdir}/yuna-live.service
    install -m 0644 ${WORKDIR}/yuna-camera.service \
        ${D}${systemd_system_unitdir}/yuna-camera.service
}
