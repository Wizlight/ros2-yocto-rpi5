SUMMARY = "Yuna object detector using NCNN"
LICENSE = "CLOSED"

SRC_URI = "file://yuna-detect.cpp"

S = "${WORKDIR}"

DEPENDS = "ncnn"

do_compile() {
    ${CXX} ${CXXFLAGS} ${LDFLAGS} \
        -I${STAGING_INCDIR}/ncnn \
        ${S}/yuna-detect.cpp \
        -o yuna-detect \
        -lncnn
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 yuna-detect ${D}${bindir}/yuna-detect
}
