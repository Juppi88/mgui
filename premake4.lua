-- Mylly GUI library

project "Lib-MGUI"
	kind "StaticLib"
	language "C"
	files { "*.c", "*.h", "Elements/*", "Input/*", "Renderer/*", "Skin/*", "MGUI/*", "premake4.lua" }
	vpaths { [""] = { "../Libraries/MGUI" } }
	includedirs { ".", "..", "Elements", "Input", "Renderer", "Skin" }
	location ( "../../Projects/" .. os.get() .. "/" .. _ACTION )
	
	-- Linux specific stuff
	configuration "linux"
		targetextension ".a"
		buildoptions { "-fms-extensions" } -- Unnamed struct/union fields within structs/unions
		configuration "Debug" targetname "libmguid"
		configuration "Release" targetname "libmgui"
	
	-- Windows specific stuff
	configuration "windows"
		targetextension ".lib"
		buildoptions { "/wd4201 /wd4996" } -- C4201: nameless struct/union, C4996: This function or variable may be unsafe.
		configuration "Debug" targetname "mguid"
		configuration "Release" targetname "mgui"
