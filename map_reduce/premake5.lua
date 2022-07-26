project "map_reduce"
    kind "None"
    language "C++"
    cppdialect "C++20"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    includedirs { "%{prj.location}" }
    files { "**.h", "**.cpp" }
