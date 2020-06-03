BASEDIR=$1
CLOSURE_COMPILER_DIR="$BASEDIR/3rdparty/closure-compiler"
CLOSURE_COMPILER_JAR="$CLOSURE_COMPILER_DIR/compiler.jar"
INPUT="$BASEDIR/templates/xlsxmagic.js"
OUTPUT_FN="xlsxmagic.min.js"
OUTPUT="$BASEDIR/templates/$OUTPUT_FN"

(echo "Building $OUTPUT_FN with closure-compiler..." && \
    java -jar "$CLOSURE_COMPILER_JAR" \
         --compilation_level ADVANCED_OPTIMIZATIONS \
         --warning_level VERBOSE \
         --js "$INPUT" \
         --js_output_file "$OUTPUT" && \
    echo 'Done.') || \
(echo 'Failed. Read `3rdparty/closure-compiler/README` for more detail.' && \
echo 'Using the uncompressed version.' && \
cat "$INPUT" > "$OUTPUT")
