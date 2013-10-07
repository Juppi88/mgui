-- Mylly GUI renderer

project "Lib-MGUI-Renderer-Xlib"
	kind "StaticLib"
	language "C"
	files { "*.h", "*.c", "premake4.lua" }
	includedirs { ".", "..", "../..", "../../.." }
	vpaths { [""] = { "../Libraries/MGUI/Renderer/Xlib" } }
	location ( "../../../../Projects/" .. os.get() .. "/" .. _ACTION )

	-- Linux specific stuff
	configuration "linux"
		targetextension ".a"
		buildoptions { "-fms-extensions" } -- Unnamed struct/union fields within structs/unions
		configuration "Debug" targetname "mguixlib"
		configuration "Release" targetname "mguixlib"
