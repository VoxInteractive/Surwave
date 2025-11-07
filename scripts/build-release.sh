#!/usr/bin/env bash
set -ex

EMSDK_DIR="$HOME/src/emsdk"
source "${EMSDK_DIR}/emsdk_env.sh"
export PATH="${EMSDK_DIR}:${PATH}"
export PATH="${EMSDK_DIR}/node/22.16.0_64bit/bin:${PATH}"
export PATH="${EMSDK_DIR}/upstream/emscripten:${PATH}"

for platform in web linux windows; do
    scons platform="${platform}" target=template_release
done
