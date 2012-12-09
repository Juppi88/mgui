-- Mylly GUI renderer

project "Lib-MGUI Renderer DirectX8"
	kind "StaticLib"
	language "C++"
	files { "*.h", "*.c", "*.cpp", "premake4.lua" }
	includedirs { ".", "..", "../..", "../../.." }
	vpaths { [""] = { "../Libraries/MGUI/Renderer/DirectX8" } }
	location ( "../../../../Projects/" .. os.get() .. "/" .. _ACTION )
	
	-- Windows specific stuff
	configuration "windows"
		targetextension ".lib"
		buildoptions { "/wd4201 /wd4996" } -- C4201: nameless struct/union, C4996: This function or variable may be unsafe.
		configuration "Debug" targetname "mguidx8"
		configuration "Release" targetname "mguidx8"
