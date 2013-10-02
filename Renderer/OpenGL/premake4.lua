-- Mylly GUI OpenGL renderer

project "Lib-MGUI-Renderer-OpenGL"
	kind "StaticLib"
	language "C"
	defines { "GLEW_STATIC" }
	files { "*.h", "*.c", "./glew/**", "premake4.lua" }
	includedirs { ".", "./glew/include", "..", "../..", "../../.." }
	location ( "../../../../Projects/" .. os.get() .. "/" .. _ACTION )
	
	vpaths {
		["Shared"] = { "../Shared/**" },
		["glew"] = { "./glew/**" },
		[""] = { "./**" }
	}

	-- Linux specific stuff
	configuration "linux"
		targetextension ".a"
		files { "../Shared/Linux/*" } -- Font loading methods on Linux
		buildoptions { "-fms-extensions" } -- Unnamed struct/union fields within structs/unions
		configuration "Debug" targetname "mguiopengld"
		configuration "Release" targetname "mguiopengl"
	
	-- Windows specific stuff
	configuration "windows"
		targetextension ".lib"
		files { "../Shared/Windows/*" } -- Font loading methods on Windows
		defines { "MYLLY_WINDOWS_EXTRAS" }
		buildoptions { "/wd4201 /wd4206" } -- C4201: nameless struct/union, C4206: translation unit is empty
		configuration "Debug" targetname "mguirendopengld"
		configuration "Release" targetname "mguirendopengl"
