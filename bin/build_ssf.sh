BASEDIR=$1
SSF_DIR="$BASEDIR/3rdparty/ssf"
INPUT="$SSF_DIR/ssf.js"

build () {
    OUTPUT_FN="ssf-bin"
    OUTPUT="$BASEDIR/3rdparty/ssf/$OUTPUT_FN"
    (echo "Building $OUTPUT with PKG Compiler" && \
        pkg --out-path $OUTPUT $INPUT
        echo 'Done.') || \
    (echo 'Failed. ' && \
    echo 'Using the uncompressed version.' && \
    cat "$INPUT" > "$OUTPUT")
}

build
