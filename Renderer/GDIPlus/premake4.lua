-- Mylly GUI GDI+ renderer

project "Lib-MGUI-Renderer-GDIPlus"
	kind "StaticLib"
	language "C++"
	files { "*.h", "*.c", "*.cpp", "premake4.lua" }
	includedirs { ".", "..", "../..", "../../.." }
	location ( "../../../../Projects/" .. os.get() .. "/" .. _ACTION )
	
	vpaths {
		["Shared"] = { "../Shared/**" },
		[""] = { "./**" }
	}
	
	-- Windows specific stuff
	configuration "windows"
		targetextension ".lib"
		defines { "MYLLY_WINDOWS_EXTRAS" }
		files { "../Shared/Windows/*" } -- Shared font loading on Windows
		buildoptions { "/wd4201 /wd4996" } -- C4201: nameless struct/union, C4996: This function or variable may be unsafe.
		configuration "Debug" targetname "mguigdiplusd"
		configuration "Release" targetname "mguigdiplus"
