#!/usr/bin/env python

env = SConscript("godot-cpp/SConstruct")

env.Append(CXXFLAGS='-std=c++2b')

# Generate compilation database
env.Tool('compilation_db')
env.CompilationDatabase("compile_commands.json")

# For reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

sources = SConscript("src/SConstruct", variant_dir="build", exports="env", duplicate=0)

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "Project/bin/libfishbytes.{}.{}.framework/libfishbytes.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "Project/bin/libfishbytes{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

Default(library)
