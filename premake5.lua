workspace "BDIXTester"
   architecture "x64"
   configurations { "Debug", "Release" }
   targetdir "test"

   outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

   project "BDIXTester"
      kind "WindowedApp"
      language "C++"
      cppdialect "C++20"
      targetdir "bin/%{cfg.buildcfg}"

      files {
         "src/**.h", 
         "src/**.c", 
         "src/**.cpp", 
         "include/**.h", 
         "include/**.c", 
         "include/**.cpp",  
         "resource/**",
         "resource/*.rc"
      }

      includedirs {
         "src",
         "include",
         "resource/",
      }
      
      libdirs {
         "lib"
      }

      links {
         "Shell32",      
         "libcurl",      
         "d3d9",         
         "d3d11"         
     }

      targetdir ("bin/" .. outputdir .. "/%{prj.name}")
      objdir ("bin/objs/" .. outputdir .. "/%{prj.name}")
      defines {
         "_CRT_SECURE_NO_WARNINGS"
      }

      filter "configurations:Debug"
         defines { "APP_DEBUG" }
         runtime "Debug"
         symbols "On"

      filter "configurations:Release"
         defines { "APP_RELEASE" }
         runtime "Release"
         optimize "On"