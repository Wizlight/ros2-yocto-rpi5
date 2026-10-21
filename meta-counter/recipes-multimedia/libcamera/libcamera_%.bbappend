# Raspberry Pi fork required for the Pi 5 CFE/PiSP camera stack
SRC_URI = "git://github.com/RPi-Distro/libcamera.git;protocol=https;branch=pios/bookworm"
SRCREV = "c12864890ddd47b5965a03bb5c190bc4bc504a9b"

DEPENDS:append:rpi = " libpisp"

PACKAGECONFIG_CONFARGS:remove:rpi = "-Dpipelines=rpi/vc4 -Dipas=rpi/vc4"
PACKAGECONFIG_CONFARGS:append:rpi = " -Dpipelines=rpi/vc4,rpi/pisp -Dipas=rpi/vc4,rpi/pisp"
