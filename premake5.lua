workspace "map_reduce"
    architecture "x64"
    configurations { "Debug", "Release" }
    startproject "client"

    filter "configurations:Debug"
        defines { "_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "map_reduce"
include "client"
