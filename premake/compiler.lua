project "compiler"
    kind "ConsoleApp"
    flags { "ExtraWarnings", "FatalWarnings" }
    files { "../compiler/*.cpp" }
    includedirs { "../include" }
    links { "libcompiler" }
    configuration { "vs*" }
        defines { "_CRT_SECURE_NO_WARNINGS" }
    configuration {}
