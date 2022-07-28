project "word_counter"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files { "**.h", "**.cpp" }
    includedirs { "%{wks.location}/map_reduce" }
    links { "map_reduce" }

    pchheader "pch.h"
    pchsource "pch.cpp"
