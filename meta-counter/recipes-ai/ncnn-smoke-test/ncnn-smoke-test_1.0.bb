SUMMARY = "Simple NCNN inference smoke test"
LICENSE = "CLOSED"

SRC_URI = "file://ncnn-smoke.cpp"

S = "${WORKDIR}"

DEPENDS = "ncnn"

do_compile() {
    ${CXX} ${CXXFLAGS} ${LDFLAGS} \
        -I${STAGING_INCDIR}/ncnn \
        ${S}/ncnn-smoke.cpp \
        -o ncnn-smoke \
        -lncnn
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ncnn-smoke ${D}${bindir}/ncnn-smoke
}
