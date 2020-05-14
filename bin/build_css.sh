BASEDIR=$1
YUI_DIR="$BASEDIR/3rdparty/yuicompressor"
YUI_JAR="$YUI_DIR/yuicompressor-2.4.8.jar"

build () {
    INPUT="$BASEDIR/templates/$1"
    OUTPUT="$BASEDIR/templates/$2"
    (echo "Building $OUTPUT with YUI Compressor" && \
        java -jar "$YUI_JAR" \
             --charset utf-8 \
             -o "$OUTPUT" \
             "$INPUT" && \
        echo 'Done.') || \
    (echo 'Failed. ' && \
    echo 'Using the uncompressed version.' && \
    cat "$INPUT" > "$OUTPUT")
}

build "base.css" "base.min.css"
