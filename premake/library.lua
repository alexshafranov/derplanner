project "planner"
    kind "StaticLib"
    flags { "ExtraWarnings", "FatalWarnings" }
    files { "../source/compiler/**.cpp" }
    includedirs { "../include" }
