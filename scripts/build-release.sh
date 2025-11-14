#!/usr/bin/env bash
set -ex

COMMON_BUILD_ARGS=("target=template_release" "production=yes" "optimize=speed" "lto=full")


EMSDK_DIR="$HOME/src/emsdk"
source "${EMSDK_DIR}/emsdk_env.sh"
export PATH="${EMSDK_DIR}:${PATH}"
export PATH="${EMSDK_DIR}/node/22.16.0_64bit/bin:${PATH}"
export PATH="${EMSDK_DIR}/upstream/emscripten:${PATH}"

for platform in web linux windows; do
    scons ${COMMON_BUILD_ARGS[@]} platform="${platform}" --clean
    scons ${COMMON_BUILD_ARGS[@]} platform="${platform}"
done
