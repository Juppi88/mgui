-- Mylly GUI DirectX 8 renderer

project "Lib-MGUI-Renderer-DirectX8"
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
		configuration "Debug" targetname "mguidx8d"
		configuration "Release" targetname "mguidx8"
