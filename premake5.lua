workspace "derplanner"
    platforms { "x86_64", "x86_32" }
    configurations { "release", "debug" }
    language "C++"

    flags { "NoPCH", "FatalWarnings" }
    rtti "Off"
    warnings "Extra"
    objdir ".build/obj"

    filter "debug"
        flags "Symbols"

    filter "release"
        flags "Symbols"

    filter "release"
        optimize "Speed"
        defines { "NDEBUG" }

    filter { "debug", "architecture:x86_32" }
        targetdir "bin/x86_32/debug"

    filter { "release", "architecture:x86_32" }
        targetdir "bin/x86_32/release"

    filter { "debug", "architecture:x86_64" }
        targetdir "bin/x86_64/debug"

    filter { "release", "architecture:x86_64" }
        targetdir "bin/x86_64/release"

    filter { "action:gmake" }
        buildoptions { "-Wno-missing-field-initializers" }

    filter { "action:vs*" }
        defines { "_CRT_SECURE_NO_WARNINGS" }
        buildoptions { "/wd4456" } -- declaration of '...' hides previous local declaration.
        buildoptions { "/wd4577" } -- 'noexcept' used with no exception handling mode specified; termination on exception is not guaranteed.

    project "derplanner-compiler"
        kind "StaticLib"
        exceptionhandling "Off"
        files { "source/compiler/*.cpp" }
        includedirs { "include" }

    project "derplanner-runtime"
        kind "StaticLib"
        exceptionhandling "Off"
        files { "source/runtime/*.cpp" }
        includedirs { "include" }

    project "derplannerc"
        kind "ConsoleApp"
        files { "app/main.compiler.cpp" }
        includedirs { "include" }
        links { "derplanner-compiler" }

    project "tests"
        kind "ConsoleApp"
        files { "test/*.cpp", "test/unittestpp/src/*.cpp" }
        includedirs { "test/unittestpp", "include", "source" }
        links { "derplanner-compiler", "derplanner-runtime" }
        filter { "system:linux or system:macosx" }
            files { "test/unittestpp/src/Posix/*.cpp" }
        filter { "action:vs*" }
            files { "test/unittestpp/src/Win32/*.cpp" }

    project "domain-travel"
        kind "SharedLib"
        files { "examples/travel.cpp" }
        includedirs { "include" }
        filter { "action:vs*" }
            defines { "PLNNR_DOMAIN_API=__declspec(dllexport)" }

    project "example-travel"
        kind "ConsoleApp"
        files { "examples/travel.main.cpp" }
        includedirs { "include" }
        links { "derplanner-runtime", "domain-travel" }
        filter { "action:vs*" }
            defines { "PLNNR_DOMAIN_API=__declspec(dllimport)" }
