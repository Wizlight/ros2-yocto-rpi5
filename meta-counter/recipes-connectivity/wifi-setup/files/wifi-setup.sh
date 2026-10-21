#!/bin/sh
set -eu

SRC="/boot/wpa_supplicant.conf"
DST="/etc/wpa_supplicant/wpa_supplicant-wlan0.conf"

if [ ! -f "${SRC}" ]; then
    echo "wifi-setup: no wpa_supplicant.conf on /boot"
    exit 0
fi

mkdir -p /etc/wpa_supplicant

cp "${SRC}" "${DST}"
chmod 600 "${DST}"

echo "wifi-setup: Wi-Fi configuration installed"

systemctl restart wpa_supplicant@wlan0.service
systemctl restart systemd-networkd.service
