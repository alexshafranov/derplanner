solution "derplanner"
    platforms { "x32", "x64" }
    configurations { "debug", "release" }
    language "C++"

    flags
    {
        "NoPCH",
        "NoRTTI",
        "NoExceptions",
        "ExtraWarnings",
        "FatalWarnings",
        "Symbols",
    }

    dofile "premake/unittestpp.lua"

    objdir ".build/obj"
