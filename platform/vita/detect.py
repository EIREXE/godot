import os
import platform
import sys
import os.path

def is_active():
    return True

def get_name():
    return "Vita"

def can_build():
    # Check the minimal dependencies
    if "DOLCESDK" not in os.environ:
        print("VITASDK not defined in environment.. vita diabled.")
        return False
    return True

def get_opts():
    
    from SCons.Variables import BoolVariable, EnumVariable

    return [
        BoolVariable('use_sanitizer', 'Use LLVM compiler address sanitizer', False),
        BoolVariable('use_leak_sanitizer', 'Use LLVM compiler memory leaks sanitizer (implies use_sanitizer)', False),
        EnumVariable('debug_symbols', 'Add debugging symbols to release builds', 'yes', ('yes', 'no', 'full')),
        BoolVariable('separate_debug_symbols', 'Create a separate file containing debugging symbols', False),
        BoolVariable('touch', 'Enable touch events', True),
        ]

def get_flags():
    return [
        ("tools", False),
        ('builtin_bullet', False),
        ('builtin_enet', False), # Not in portlibs.
        ('builtin_freetype', False),
        ('builtin_libogg', False),
        ('builtin_libpng', False),
        ('builtin_libtheora', False),
        ('builtin_libvorbis', False),
        ('builtin_libvpx', False),
        ('builtin_libwebp', True),
        ('builtin_libwebsockets', True), # Not in portlibs.
        ('builtin_mbedtls', False),
        ('builtin_miniupnpc', False),
        ('builtin_opus', False),
        ('builtin_pcre2', False),
        ('builtin_squish', True), # Not in portlibs.
        ('builtin_zlib', False),
        ('builtin_zstd', True), # Not in portlibs.
        ('module_websocket_enabled', False),
        ('module_mbedtls_enabled', False),
        ('module_upnp_enabled', False),
        ('module_enet_enabled', False),
        ('module_gdnative_enabled', False),
        ('module_regex_enabled', False),
        ('module_webm_enabled', False),
        ('module_mobile_vr_enabled', False),
        ('module_csg_enabled', False),
        ('module_dds_enabled', False),
        ('module_visual_script_enabled', False),
        ('disable_3d', True),
        ('minizip', False),
        ('disable_advanced_gui', True),
        ('deprecated', False),
        ('module_gdnavigation_enabled', False),
        ('module_gridmap_enabled', False),
        ('module_hdr_enabled', False),
        ('module_opensimplex_enabled', False),
        ('module_visual_script_enabled', False),
        ('module_bullet_enabled', False),
        ('module_webxr_enabled', False),
        ('module_jsonrpc_enabled', False),
        ('module_jpg_enabled', False),
        ('module_tga_enabled', False)
        ]


def configure(env):
    env["CC"] = "arm-dolce-eabi-gcc"
    env["CXX"] = "arm-dolce-eabi-g++"
    env["LD"] = "arm-dolce-eabi-ld"
    env["AR"] = "arm-dolce-eabi-ar"
    env["STRIP"] = "arm-dolce-eabi-strip"
    env["RANLIB"] = "arm-dolce-eabi-ranlib"
    ## Build type

    vita_sdk_path = os.environ.get("DOLCESDK")

    pkg_config_path = "{}/arm-dolce-eabi/lib/pkgconfig/pkgconfig"

    os.environ["PKG_CONFIG_PATH"] = pkg_config_path
    env['ENV']['PKG_CONFIG_PATH'] = pkg_config_path


    env.Prepend(CPPPATH=['{}/arm-dolce-eabi/include'.format(os.environ.get("DOLCESDK"))])
    env.Prepend(CPPPATH=['{}/arm-dolce-eabi/include/freetype2'.format(os.environ.get("DOLCESDK"))])
    env.Prepend(CPPPATH=['{}/arm-dolce-eabi/include/bullet'.format(os.environ.get("DOLCESDK"))])
    env.Prepend(CPPPATH=['{}/share/gcc-arm-dolce-eabi/samples/common'.format(os.environ.get("DOLCESDK"))])
    env.Append(LIBPATH=['{}/arm-dolce-eabi/lib'.format(os.environ.get("DOLCESDK"))])
    env.Prepend(LINKFLAGS=["-Wl,-q"])
    print(env.get("CCFLAGS"))

    env.Prepend(CCFLAGS=['-Wl,-q', '-D_POSIX_TIMERS', '-DNO_THREADS', '-DUNIX_SOCKET_UNAVAILABLE', '-DVITA_ENABLED', '-DPOSH_COMPILER_GCC', '-DPOSH_OS_VITA', '-DPOSH_OS_STRING=\\"vita\\"'])


    if (env["target"] == "release"):
        # -O3 -ffast-math is identical to -Ofast. We need to split it out so we can selectively disable
        # -ffast-math in code for which it generates wrong results.
        if (env["optimize"] == "speed"): #optimize for speed (default)
            env.Prepend(CCFLAGS=['-O3', '-ffast-math'])
        else: #optimize for size
            env.Prepend(CCFLAGS=['-Os'])
     
        if (env["debug_symbols"] == "yes"):
            env.Prepend(CCFLAGS=['-g1'])
        if (env["debug_symbols"] == "full"):
            env.Prepend(CCFLAGS=['-g2'])

    elif (env["target"] == "release_debug"):
        if (env["optimize"] == "speed"): #optimize for speed (default)
            env.Prepend(CCFLAGS=['-O2', '-ffast-math', '-DDEBUG_ENABLED'])
        else: #optimize for size
            env.Prepend(CCFLAGS=['-Os', '-DDEBUG_ENABLED', '-DDEBUG_MEMORY_ENABLED'])

        if (env["debug_symbols"] == "yes"):
            env.Prepend(CCFLAGS=['-g1'])
        if (env["debug_symbols"] == "full"):
            env.Prepend(CCFLAGS=['-g2'])

    elif (env["target"] == "debug"):
        env.Prepend(CCFLAGS=['-g3', '-DDEBUG_ENABLED', '-DDEBUG_MEMORY_ENABLED'])
        #env.Append(LINKFLAGS=['-rdynamic'])

        #env.Append(LINKFLAGS=['-rdynamic'])
    ## Architecture

    env["bits"] = "32"

    ## Flags

    # Linkflags below this line should typically stay the last ones
    #if not env['builtin_zlib']:
    #    env.ParseConfig('aarch64-none-elf-pkg-config zlib --cflags --libs')

    env.Append(CPPPATH=['#platform/vita'])
    env.Append(CPPFLAGS=['-DLIBC_FILEIO_ENABLED', '-DOPENGL_ENABLED', '-DGLES_ENABLED'])
    env.Append(CPPFLAGS=['-DPTHREAD_NO_RENAME'])
    env.Append(CCFLAGS=['-mtune=cortex-a9', '-mfpu=neon', '-ftree-vectorize'])
    env.Append(LIBS=[
        "pib",
        "SceLibKernel_stub",
        "SceAppMgrUser_stub",
        "SceIofilemgr_stub",
        "SceGxm_stub",
        "SceDisplay_stub",
        "SceFios2_stub",
        "png",
        "freetype",
        "opus",
        "vorbis",
        "ogg",
        "z",
        "theora",
        "pthread"
    ])
    print(env.get("LIBS"))

"""
        "libpib",
        "SceLibKernel_stub",
        "ScePvf_stub",
        "SceAppMgr_stub",
        "SceAppUtil_stub",
        "ScePgf_stub",
        "jpeg",
        "SceCommonDialog_stub",
        "SceGxm_stub",
        "SceDisplay_stub",
        "SceSysmodule_stub",
        "vitashark",
        "SceShaccCg_stub",
        "pthread"
"""
#-lglad -lEGL -lglapi -ldrm_nouveau 
