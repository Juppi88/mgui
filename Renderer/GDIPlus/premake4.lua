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
		configuration "Debug" targetname "mguigdid"
		configuration "Release" targetname "mguigdi"
