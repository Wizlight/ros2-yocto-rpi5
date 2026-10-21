SUMMARY = "Trained NCNN model for Yuna detection"
LICENSE = "CLOSED"

SRC_URI = " \
    file://model.ncnn.param \
    file://model.ncnn.bin \
    file://metadata.yaml \
"

S = "${WORKDIR}"

do_install() {
    install -d ${D}${datadir}/yuna-detector

    install -m 0644 ${WORKDIR}/model.ncnn.param \
        ${D}${datadir}/yuna-detector/model.ncnn.param

    install -m 0644 ${WORKDIR}/model.ncnn.bin \
        ${D}${datadir}/yuna-detector/model.ncnn.bin

    install -m 0644 ${WORKDIR}/metadata.yaml \
        ${D}${datadir}/yuna-detector/metadata.yaml
}

FILES:${PN} += "${datadir}/yuna-detector"
