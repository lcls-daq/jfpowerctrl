if [ -d /cds/group/pcds/package/external/blackfin/2014R1 ]; then
    BFIN_DEF_DIR=/cds/group/pcds/package/external/blackfin/2014R1
else
    BFIN_DEF_DIR=""
fi

BFIN_DIR="${BFIN_DIR-$BFIN_DEF_DIR}"

if [ -d "${BFIN_DIR}/opt/uClinux/bfin-uclinux/bin" ]; then
    echo "Using bfin install under ${BFIN_DIR}"
    export PATH="${BFIN_DIR}/opt/uClinux/bfin-uclinux/bin:$PATH"
    unset CROSS
else
    export CROSS=""
fi
