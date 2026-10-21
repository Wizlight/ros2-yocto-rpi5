SUMMARY = "Configure Wi-Fi from Raspberry Pi boot partition"
LICENSE = "CLOSED"

SRC_URI = " \
    file://wifi-setup.sh \
    file://wifi-setup.service \
    file://25-wlan0.network \
"

S = "${WORKDIR}"

RDEPENDS:${PN} += "wpa-supplicant"

inherit systemd

SYSTEMD_SERVICE:${PN} = "wifi-setup.service"
SYSTEMD_AUTO_ENABLE = "enable"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/wifi-setup.sh \
        ${D}${bindir}/wifi-setup

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/wifi-setup.service \
        ${D}${systemd_system_unitdir}/wifi-setup.service

    install -d ${D}${sysconfdir}/systemd/network
    install -m 0644 ${WORKDIR}/25-wlan0.network \
        ${D}${sysconfdir}/systemd/network/25-wlan0.network
}
