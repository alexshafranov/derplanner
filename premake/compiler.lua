project "derplannerc"
    kind "ConsoleApp"
    flags { "ExtraWarnings", "FatalWarnings" }
    files { "../compiler/*.cpp" }
    includedirs { "../include" }
    links { "derplanner-compiler" }
    configuration { "vs*" }
        defines { "_CRT_SECURE_NO_WARNINGS" }
    configuration {}
