#!/usr/bin/env python
import os
import sys

# This is done because Gemini code assist insists on running scons without passing parameters explicitly.
# Provide sensible defaults so running `scons` without arguments produces a debug build with symbols enabled.
# Only set these when the caller did not explicitly specify them on the command line (via ARGUMENTS or -Q).
# This preserves explicit overrides like `scons optimize=release`.
from SCons.Script import ARGUMENTS
if "debug_symbols" not in ARGUMENTS:
    ARGUMENTS["debug_symbols"] = "yes"
if "optimize" not in ARGUMENTS:
    ARGUMENTS["optimize"] = "debug"

env = SConscript("godot-cpp/SConstruct")

# For reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

env.Append(CPPPATH=["flecs/distr/", "."])
flecs_c_source = "flecs/distr/flecs.c"

sources = Glob("src/*.cpp") + Glob("src/flecs/*.cpp") + ["Game/cpp/systems.cpp"]

# Flecs
FLECS_COMMON_OPTS = [
    "FLECS_NDEBUG",
    "FLECS_CPP_NO_AUTO_REGISTRATION",
    "ecs_ftime_t=double",
]

FLECS_DEVELOPMENT_OPTS = []
FLECS_PRODUCTION_OPTS = [
    "FLECS_CUSTOM_BUILD",
    "FLECS_CPP",
    "FLECS_DISABLE_COUNTERS",
    "FLECS_LOG",
    "FLECS_META",
    "FLECS_PIPELINE",
    "FLECS_SCRIPT",
    "FLECS_SYSTEM",
    "FLECS_TIMER",
]
FLECS_OPTS = FLECS_DEVELOPMENT_OPTS if env["target"] == "template_debug" else FLECS_PRODUCTION_OPTS

FLECS_WINDOWS_OPTS = [f"/D{o}" for o in (FLECS_OPTS + FLECS_COMMON_OPTS)]
FLECS_UNIX_OPTS =    [f"-D{o}" for o in (FLECS_OPTS + FLECS_COMMON_OPTS)]

# Ensure all translation units (C and C++) see the same Flecs defines (e.g. ecs_ftime_t=double)
# Convert any "name=value" strings into (name, value) tuples so SCons emits the
# proper -D / /D forms and handles quoting correctly across platforms.
cppdefines_list = []
for opt in (FLECS_OPTS + FLECS_COMMON_OPTS):
    if "=" in opt:
        name, value = opt.split("=", 1)
        cppdefines_list.append((name, value))
    else:
        cppdefines_list.append(opt)
env.Append(CPPDEFINES=cppdefines_list)

# Always compile flecs.c as C code, and fix winsock header redefinition on Windows
if env["platform"] == "windows":
    env.Append(CXXFLAGS=["/std:c++17"])
    env.Append(LIBS=["Ws2_32"])
    flecs_c_obj = env.SharedObject(
        target="flecs_c_obj",
        source=[flecs_c_source],
        CCFLAGS=["/TC", "/DWIN32_LEAN_AND_MEAN", "/O2"] + FLECS_WINDOWS_OPTS,
    )
else:
    env.Append(CXXFLAGS=["-std=c++17"])
    flecs_c_obj = env.SharedObject(
        target="flecs_c_obj",
        source=[flecs_c_source],
        CFLAGS=["-std=gnu99", "-O2", "-DFLECS_NDEBUG"] + FLECS_UNIX_OPTS,
    )


all_objs = env.SharedObject(sources) + [flecs_c_obj]

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "Game/cpp/bin/libflecs.{}.{}.framework/libflecs.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=all_objs,
    )
elif env["platform"] == "ios":
    if env["ios_simulator"]:
        library = env.StaticLibrary(
            "Game/cpp/bin/libflecs.{}.{}.simulator.a".format(env["platform"], env["target"]),
            source=all_objs,
        )
    else:
        library = env.StaticLibrary(
            "Game/cpp/bin/libflecs.{}.{}.a".format(env["platform"], env["target"]),
            source=all_objs,
        )
else:
    library = env.SharedLibrary(
        "Game/cpp/bin/libflecs{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=all_objs,
    )

Default(library)
