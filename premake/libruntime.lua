project "derplanner-runtime"
    kind "StaticLib"
    flags { "FatalWarnings", "NoExceptions" }
    warnings "Extra"
    files { "../source/runtime/*.cpp" }
    includedirs { "../include" }
