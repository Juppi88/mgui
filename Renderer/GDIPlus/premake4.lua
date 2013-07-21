-- Mylly GUI renderer

project "Lib-MGUI-Renderer-GDIPlus"
	kind "StaticLib"
	language "C++"
	files { "*.h", "*.c", "*.cpp", "premake4.lua" }
	includedirs { ".", "..", "../..", "../../.." }
	vpaths { [""] = { "../Libraries/MGUI/Renderer/GDIPlus" } }
	location ( "../../../../Projects/" .. os.get() .. "/" .. _ACTION )
	
	-- Windows specific stuff
	configuration "windows"
		targetextension ".lib"
		defines { "MGUI_GDIPLUS", "__MYLLY_USE_GDIPLUS" }
		buildoptions { "/wd4201 /wd4996" } -- -- C4201: nameless struct/union, C4996: This function or variable may be unsafe (mbstowcs)
		configuration "Debug" targetname "mguigdid"
		configuration "Release" targetname "mguigdi"
