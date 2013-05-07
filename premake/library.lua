project "planner"
    kind "StaticLib"
    flags { "ExtraWarnings", "FatalWarnings" }
    files { "../source/derplanner/**.cpp" }
    includedirs { "../include" }
