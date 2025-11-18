#!/usr/bin/env bash
set -ex

COMMON_BUILD_ARGS=("target=template_release" "production=yes" "optimize=speed" "lto=full")


EMSDK_DIR="$HOME/src/emsdk"
source "${EMSDK_DIR}/emsdk_env.sh"
export PATH="${EMSDK_DIR}:${PATH}"
export PATH="${EMSDK_DIR}/node/22.16.0_64bit/bin:${PATH}"
export PATH="${EMSDK_DIR}/upstream/emscripten:${PATH}"

# Build for web with specific linkflags - https://www.flecs.dev/flecs/md_docs_2Quickstart.html#emscripten
WEB_LINKFLAGS=("-s ALLOW_MEMORY_GROWTH=1" "-s STACK_SIZE=1mb" "-s EXPORTED_RUNTIME_METHODS=cwrap" "-s MODULARIZE=1" "-s EXPORT_NAME='Game'")
scons "${COMMON_BUILD_ARGS[@]}" platform="web" linkflags="${WEB_LINKFLAGS[*]}"

# Build for other platforms
for platform in linux windows; do
    scons ${COMMON_BUILD_ARGS[@]} platform="${platform}"
done
