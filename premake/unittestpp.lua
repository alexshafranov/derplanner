project "unittestpp"
    kind "StaticLib"
    flags { "ExtraWarnings", "FatalWarnings" }
    files { "../deps/unittestpp/src/*.cpp" }
    configuration { "linux or macosx" }
        files { "../deps/unittestpp/src/Posix/**.cpp" }
    configuration { "vs*" }
        defines { "_CRT_SECURE_NO_WARNINGS" }
    configuration { "windows" }
        files { "../deps/unittestpp/Win32/**.cpp" }
    configuration {}
