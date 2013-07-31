-- Mylly GUI renderer

project "Lib-MGUI-Renderer-OpenGL"
	kind "StaticLib"
	language "C"
	files { "*.h", "*.c", "premake4.lua" }
	includedirs { ".", "..", "../..", "../../.." }
	vpaths { [""] = { "../Libraries/MGUI/Renderer/OpenGL" } }
	location ( "../../../../Projects/" .. os.get() .. "/" .. _ACTION )

	-- Linux specific stuff
	configuration "linux"
		targetextension ".a"
		buildoptions { "-fms-extensions" } -- Unnamed struct/union fields within structs/unions
		configuration "Debug" targetname "mguiopengld"
		configuration "Release" targetname "mguiopengl"
	
	-- Windows specific stuff
	configuration "windows"
		targetextension ".lib"
		buildoptions { "/wd4201 /wd4206" } -- C4201: nameless struct/union, C4206: translation unit is empty
		configuration "Debug" targetname "mguirendopengld"
		configuration "Release" targetname "mguirendopengl"
