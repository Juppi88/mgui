-- Mylly GUI renderer

project "MGUI Renderer GDI+"
	kind "StaticLib"
	language "C++"
	files { "*.h", "*.c", "*.cpp", "premake4.lua" }
	includedirs { ".", "..", "../..", "../../.." }
	vpaths { [""] = { "../Libraries/MGUI/Renderer/GDIPlus" } }
	location ( "../../../../Projects/" .. os.get() .. "/" .. _ACTION )
	
	-- Windows specific stuff
	configuration "windows"
		targetextension ".lib"
		buildoptions { "/wd4996" } -- C4996: This function or variable may be unsafe (mbstowcs)
		configuration "Debug" targetname "mguigdid"
		configuration "Release" targetname "mguigdi"
