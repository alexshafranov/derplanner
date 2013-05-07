project "tests"
    kind "ConsoleApp"
    flags { "ExtraWarnings", "FatalWarnings" }
    files { "../tests/*.cpp" }
    includedirs { "../deps/unittestpp", "../include" }
    links { "unittestpp", "planner" }
    configuration { "vs*" }
        defines { "_CRT_SECURE_NO_WARNINGS" }
    configuration {}
