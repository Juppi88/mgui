-- Mylly GUI renderer

project "MGUI Renderer DirectX9"
	kind "StaticLib"
	language "C++"
	files { "*.h", "*.c", "*.cpp", "premake4.lua" }
	includedirs { ".", "..", "../..", "../../.." }
	vpaths { [""] = { "../Libraries/MGUI/Renderer/DirectX9" } }
	location ( "../../../../Projects/" .. os.get() .. "/" .. _ACTION )
	
	-- Windows specific stuff
	configuration "windows"
		targetextension ".lib"
		configuration "Debug" targetname "mguidx9"
		configuration "Release" targetname "mguidx9"
