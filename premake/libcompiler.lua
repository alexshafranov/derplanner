project "libcompiler"
    kind "StaticLib"
    flags { "ExtraWarnings", "FatalWarnings", "NoExceptions" }
    files { "../source/compiler/**.cpp" }
    includedirs { "../include" }