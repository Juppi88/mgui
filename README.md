# Mylly GUI

Mylly GUI (MGUI) is a graphical user interface library written in C. The main goals of the project are to create a simple-to-use, lightweight and easily portable GUI library for games and game modifications. While the library is mainly targeted for games and mods, the modular nature and separated rendering functions also allow it to be used in standalone graphical applications as well. This is demonstrated with the [official test application](https://github.com/teejii88/mguitest). You can find screenshots displaying Mylly GUI on [imgur](http://imgur.com/a/X6gX6).

Some of the features MGUI currently offers are:
* Simple C API (one header)
* Textured skin support
* Modular rendering
* Drawing UI widgets in 3D space (using the DirectX renderer)

### Development notes

Current MGUI release is an alpha release. While it is already usable and quite stable, it does not have nearly all the features that have been planned for it, and it might have bugs that can only be found with larger scale testing.

An alpha release also means that at its current stage it is much easier to add new requested features or change some existing ones than it will be later on. In that regard, I will try to keep the API relatively stable and introduce no API changes (other than new functions) within major version branches (0.0.x, 0.1.x etc).

### Using MGUI

MGUI does not include any project files within the repository. The project files are automatically generated using the latest beta release of [premake4](http://industriousone.com/premake/download). The file/folder structure is assumed to be similar to the [example project](https://github.com/teejii88/mguitest), and modifying the file hierarchy might be difficult right now. This is likely to change in the near future.

### Dependencies

The core MGUI library depends on a few in-house support libraries for various tasks.

* [Lib-Types](https://github.com/teejii88/types) - A simple C library which implements basic data structures such as linked lists.
* [Lib-Math](https://github.com/teejii88/math) - Structs, functions and macros for basic math types (vectors, matrices).
* [Lib-Platform](https://github.com/teejii88/platform) - A collection of platform dependent functions and utilities (such as windowing system related functions).
* [Lib-Stringy](https://github.com/teejii88/stringy) - Functions and utilities to work on C strings, meant as a more suitable (and some cases also more lightweight) replacement for the standard C library.
* [Lib-Input](https://github.com/teejii88/input) - Platform independent user input handler. Currently a mandatory dependency, but support for injecting input events is planned in the future (which would make this dependency optional).

In addition, the reference renderer modules may require some extra libraries.

* DirectX 8: d3d8.lib, d3dx8.lib (DirectX 8 SDK)
* DirectX 9: d3d9.lib, d3dx9.lib (DirectX 9 SDK)
* GDI+: gdiplus.lib (Windows SDK)
* OpenGL (Windows): opengl32.lib, glu32.lib (Windows SDK), latest OpenGL headers (www.opengl.org)
* Xlib (Linux): X11 development libraries

### Renderers

MGUI strictly separates actual rendering from other UI logic. This will allow users to write their own renderers that will suit the project(s) they're working on. For example, a game engine will have functions to render the actual game, and it might be easiest to add the required GUI functionality to the existing rendering code. Alternatively a game modification might hook the game's rendering functions and implement a GUI renderer using those.

For other uses there are several standalone reference renderers within the MGUI repository. The DirectX renderers are the most complete ones implementing everything in the renderer API. Others may have some limitations that will probably be fixed in the future.

* OpenGL renderer does currently not support drawing 3D GUI elements. Furthermore only 32bit windows bitmaps can be used as textures.
* GDI+ renderer does not support drawing 3D elements (for obvious reasons).
* Xlib renderer does not support drawing 3D elements (for obvious reasons), textures or skins.

### Sample projects

A sample project for the GUI library and each reference renderer is [available on GitHub](https://github.com/teejii88/mguitest). The test application repository includes MGUI and its support libraries as submodules, so clone the repository recursively. You'll need the latest beta version of [premake4](http://industriousone.com/premake/download) to generate the project files. Test skin and images for the unit test app can be found from [imgur](http://imgur.com/a/oOgzn).

There is also an [experimental C# wrapper](https://github.com/teejii88/mguisharp) for the GUI. This allows you to write your applications in C# while keeping the performance boost of unmanaged code and hardware rendering. Keep in mind though that due to the experimental nature of the wrapper applications written using it may have bugs or be unstable.
