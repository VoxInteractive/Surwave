#!/usr/bin/bash
set -ex

BRANCH="4.5"
WORKING_DIR="$HOME/src/godot"
DESTINATION_DIR="$HOME/Apps/Godot" # The built binaries will be copied here

COMMON_BUILD_ARGS=("production=yes" "optimize=speed" "lto=full")
# COMMON_BUILD_ARGS=("ccflags=-march=x86-64-v2") # Note: The raycast module (Embree) fails to build when AVX instructions are enabled

# https://docs.godotengine.org/en/latest/contributing/development/compiling/optimizing_for_size.html#disabling-advanced-text-server
# COMMON_BUILD_ARGS+=("module_text_server_adv_enabled=no" "module_text_server_fb_enabled=yes")
# https://docs.godotengine.org/en/latest/contributing/development/compiling/optimizing_for_size.html#disabling-physics-engines
# COMMON_BUILD_ARGS+=("module_godot_physics_3d_enabled=no")
# https://docs.godotengine.org/en/latest/contributing/development/compiling/optimizing_for_size.html#disabling-unwanted-modules
# COMMON_BUILD_ARGS+=("module_astcenc_enabled=no")
# COMMON_BUILD_ARGS+=("module_basis_universal_enabled=no")
# COMMON_BUILD_ARGS+=("module_bcdec_enabled=no")
# COMMON_BUILD_ARGS+=("module_bmp_enabled=no")
# COMMON_BUILD_ARGS+=("module_camera_enabled=no")
# COMMON_BUILD_ARGS+=("module_csg_enabled=no")
# COMMON_BUILD_ARGS+=("module_dds_enabled=no")
# COMMON_BUILD_ARGS+=("module_enet_enabled=no")
# COMMON_BUILD_ARGS+=("module_etcpak_enabled=no")
# COMMON_BUILD_ARGS+=("module_fbx_enabled=no")
# COMMON_BUILD_ARGS+=("module_gltf_enabled=no")
COMMON_BUILD_ARGS+=("module_godot_physics_3d_enabled=no")
# COMMON_BUILD_ARGS+=("module_gridmap_enabled=no")
# COMMON_BUILD_ARGS+=("module_hdr_enabled=no")
# COMMON_BUILD_ARGS+=("module_interactive_music_enabled=no")
# COMMON_BUILD_ARGS+=("module_jsonrpc_enabled=no")
# COMMON_BUILD_ARGS+=("module_ktx_enabled=no")
# COMMON_BUILD_ARGS+=("module_mbedtls_enabled=no")
# COMMON_BUILD_ARGS+=("module_meshoptimizer_enabled=no")
# COMMON_BUILD_ARGS+=("module_minimp3_enabled=no")
COMMON_BUILD_ARGS+=("module_mobile_vr_enabled=no")
# COMMON_BUILD_ARGS+=("module_msdfgen_enabled=no")
# COMMON_BUILD_ARGS+=("module_multiplayer_enabled=no")
# COMMON_BUILD_ARGS+=("module_noise_enabled=no")
# COMMON_BUILD_ARGS+=("module_navigation_2d_enabled=no")
# COMMON_BUILD_ARGS+=("module_navigation_3d_enabled=no")
# COMMON_BUILD_ARGS+=("module_ogg_enabled=no")
COMMON_BUILD_ARGS+=("module_openxr_enabled=no")
# COMMON_BUILD_ARGS+=("module_raycast_enabled=no")
# COMMON_BUILD_ARGS+=("module_regex_enabled=no")
# COMMON_BUILD_ARGS+=("module_svg_enabled=no")
# COMMON_BUILD_ARGS+=("module_tga_enabled=no")
# COMMON_BUILD_ARGS+=("module_theora_enabled=no")
# COMMON_BUILD_ARGS+=("module_tinyexr_enabled=no")
# COMMON_BUILD_ARGS+=("module_upnp_enabled=no")
# COMMON_BUILD_ARGS+=("module_vhacd_enabled=no")
# COMMON_BUILD_ARGS+=("module_vorbis_enabled=no")
# COMMON_BUILD_ARGS+=("module_webrtc_enabled=no")
# COMMON_BUILD_ARGS+=("module_websocket_enabled=no")
COMMON_BUILD_ARGS+=("module_webxr_enabled=no")
# COMMON_BUILD_ARGS+=("module_zip_enabled=no")

# Other options to consider:
# precision=double
# lto=full - which crashes MSVC & LLVM compilers during linking on Windows
# OR "use_llvm=yes" "lto=thin" with LLVM

# Build without advanced text server to slim it down a bit
# module_text_server_adv_enabled=no
# module_text_server_fb_enabled=yes


cd "${WORKING_DIR}"
git fetch origin ${BRANCH}
git checkout ${BRANCH}
git pull

cd modules/limboai
git pull
cd ../..

#
### Build the Editor
#
scons ${COMMON_BUILD_ARGS[@]} platform=linuxbsd ccflags=-march=x86-64-v2
scons ${COMMON_BUILD_ARGS[@]} platform=windows  ccflags=-march=x86-64-v2


#
### Build the Export templates
#
# https://docs.godotengine.org/en/latest/contributing/development/compiling/optimizing_for_size.html#disabling-3d
COMMON_EXPORT_TEMPLATE_BUILD_OPTS=("target=template_release")
# also supported: "disable_3d=yes"

# Linux
scons ${COMMON_BUILD_ARGS[@]} ${COMMON_EXPORT_TEMPLATE_BUILD_OPTS[@]} platform=linuxbsd arch=x86_64 ccflags=-march=x86-64-v2

# Windows
scons ${COMMON_BUILD_ARGS[@]} ${COMMON_EXPORT_TEMPLATE_BUILD_OPTS[@]} platform=windows arch=x86_64 ccflags=-march=x86-64-v2
scons ${COMMON_BUILD_ARGS[@]} ${COMMON_EXPORT_TEMPLATE_BUILD_OPTS[@]} platform=windows arch=x86_32
# scons ${COMMON_BUILD_ARGS[@]} ${COMMON_EXPORT_TEMPLATE_BUILD_OPTS[@]} platform=windows arch=arm64

# Web
EMSDK_DIR="$HOME/src/emsdk"
source "${EMSDK_DIR}/emsdk_env.sh"
export PATH="${EMSDK_DIR}:${PATH}"
export PATH="${EMSDK_DIR}/node/22.16.0_64bit/bin:${PATH}"
export PATH="${EMSDK_DIR}/upstream/emscripten:${PATH}"
# We're using the option dlink_enabled=yes to enable GDExtension support
scons ${COMMON_BUILD_ARGS[@]} ${COMMON_EXPORT_TEMPLATE_BUILD_OPTS[@]} platform=web dlink_enabled=yes


#
### Copy the binaries etc.
#
# https://docs.godotengine.org/en/latest/contributing/development/compiling/optimizing_for_size.html#stripping-binaries
# strip -v bin/* # No change in size

# rm -rf "${DESTINATION_DIR}"/*.exe "${DESTINATION_DIR}"/*.lib
mv bin/* "${DESTINATION_DIR}/"

# TODO: copy the export templates to %APPDATA%\Godot\export_templates\<version>\ - https://docs.godotengine.org/en/latest/contributing/development/compiling/compiling_for_windows.html#creating-windows-export-templates
