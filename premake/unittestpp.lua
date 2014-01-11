project "unittestpp"
    kind "StaticLib"
    flags { "FatalWarnings" }
    warnings "Extra"
    files { "../deps/unittestpp/src/*.cpp" }
    configuration { "linux or macosx" }
        files { "../deps/unittestpp/src/Posix/**.cpp" }
    configuration { "vs*" }
        defines { "_CRT_SECURE_NO_WARNINGS" }
    configuration { "vs*" }
        files { "../deps/unittestpp/src/Win32/**.cpp" }
    configuration {}
