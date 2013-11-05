project "tests"
    kind "ConsoleApp"
    flags { "ExtraWarnings", "FatalWarnings" }
    files { "../tests/*.cpp" }
    includedirs { "../deps/unittestpp", "../include", "../source" }
    links { "unittestpp", "derplanner-compiler", "derplanner-runtime" }
    configuration { "vs*" }
        defines { "_CRT_SECURE_NO_WARNINGS" }
    configuration {}
