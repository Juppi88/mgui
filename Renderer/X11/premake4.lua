-- Mylly GUI renderer

project "Lib-MGUI-Renderer-X11"
	kind "StaticLib"
	language "C"
	files { "*.h", "*.c", "premake4.lua" }
	includedirs { ".", "..", "../..", "../../.." }
	vpaths { [""] = { "../Libraries/MGUI/Renderer/X11" } }
	location ( "../../../../Projects/" .. os.get() .. "/" .. _ACTION )

	-- Linux specific stuff
	configuration "linux"
		targetextension ".a"
		buildoptions { "-fms-extensions" } -- Unnamed struct/union fields within structs/unions
		configuration "Debug" targetname "mguix11d"
		configuration "Release" targetname "mguix11"
