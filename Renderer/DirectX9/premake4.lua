-- Mylly GUI DirectX 9 renderer

project "Lib-MGUI-Renderer-DirectX9"
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
		files { "../Shared/Windows/*" } -- Shared font loading on Windows
		buildoptions { "/wd4201 /wd4996" } -- C4201: nameless struct/union, C4996: This function or variable may be unsafe.
		configuration "Debug" targetname "mguidx9d"
		configuration "Release" targetname "mguidx9"
