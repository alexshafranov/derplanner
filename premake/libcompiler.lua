project "derplanner-compiler"
    kind "StaticLib"
    flags { "FatalWarnings", "NoExceptions" }
    warnings "Extra"
    files { "../source/compiler/**.cpp" }
    includedirs { "../include" }
