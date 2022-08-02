project "client"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files { "**.h", "**.cpp" }

    includedirs { "%{wks.location}/map_reduce" }
    links { "map_reduce" }

    postbuildcommands {
        {"{ECHO} Copying %{cfg.buildtarget.name} to examples dir"},
        {"{COPY} %{cfg.buildtarget.abspath} %{wks.location}/examples/"}
    }
