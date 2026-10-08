#!/bin/sh
# Build MuPDF for the WebAssembly build of Vau, in the build directory
# (after ". devel/emenv.sh [build directory]"):
#
#   sh devel/build-mupdf-wasm.sh [build directory] [version]
#
# The slim build (build/wasm/slim) is the one linked: MuPDF's own fonts are
# left out but the standard 14 (Vau has the fonts of TeXmacs), and its
# readers of documents but PDF, SVG and the images, its JavaScript, its
# writers of docx/odt, its OCR, barcodes and hyphenation. Then configure Vau
# with -DMUPDF_SOURCE_DIR=<build directory>/mupdf-<version>-source.
# (Adapted from misc/wasm/build-mupdf.sh of TeXmacs.)
set -e
B=${1:-build-wasm}
V=${2:-1.28.5}
mkdir -p "$B"
cd "$B"
[ -d mupdf-$V-source ] || {
  curl -LO https://mupdf.com/downloads/archive/mupdf-$V-source.tar.gz
  tar xzf mupdf-$V-source.tar.gz
}
cd mupdf-$V-source
F="-DTOFU -DTOFU_CJK -DTOFU_SIL -DTOFU_EMOJI -DTOFU_HISTORIC -DTOFU_SYMBOL \
 -DFZ_ENABLE_XPS=0 -DFZ_ENABLE_CBZ=0 -DFZ_ENABLE_HTML=0 -DFZ_ENABLE_FB2=0 \
 -DFZ_ENABLE_MOBI=0 -DFZ_ENABLE_EPUB=0 -DFZ_ENABLE_OFFICE=0 -DFZ_ENABLE_TXT=0 \
 -DFZ_ENABLE_MD=0 -DFZ_ENABLE_HTML_ENGINE=0 -DFZ_ENABLE_OCR_OUTPUT=0 \
 -DFZ_ENABLE_DOCX_OUTPUT=0 -DFZ_ENABLE_ODT_OUTPUT=0 -DFZ_ENABLE_BROTLI=0 \
 -DFZ_ENABLE_JS=0 -DFZ_ENABLE_BARCODE=0 -DFZ_ENABLE_HYPHEN=0"
emmake make -j8 OS=wasm build=release OUT=build/wasm/slim XCFLAGS="$F" brotli=no libs
