# GGJ21 (Running on Ember Engine)

## Easy Setup

Just run `setup.ps1` and everything should mostly be taken care of.
Software packages will be installed to `.pkg`.

Then run `launch_vscode.ps1` to run VS Code with everything set up.

Alternatively, follow the setup steps below for a more controlled setup.

## Prerequisites

- C++ compiler (GCC recommended)
- CMake
- Ninja
- SDL2

Note: You can use any build tool other than Ninja, but Ninja is recommended due to its speed and simplicity.

An easy way to get these installed is via [MSYS2](https://www.msys2.org/):
```
pacman -S \
    mingw64/mingw-w64-x86_64-gcc \
    mingw64/mingw-w64-x86_64-cmake \
    mingw64/mingw-w64-x86_64-ninja \
    mingw64/mingw-w64-x86_64-SDL2 \
```

Otherwise, they can be installed manually, just make sure that they're available on your PATH.

## VSCode Setup

1. Install the "C/C++" and "CMake Tools" extensions.
2. Ensure that CMake and Ninja are available in your PATH and/or you've configured VS Code to know where they are.
    - In either your global or project settings,
        set the `"cmake.cmakePath"` setting to the path to your CMake executable.
3. Adjust your `"cmake.buildDirectory"` setting to your liking,
    for example `${workspaceRoot}/build/${buildKit}-${buildType}`.
4. Configure the project with CMake Tools, using the Emscripten kit in Release mode.
    - This will create a `build/` directory for you.
5. Set up your C/C++ extension settings to find the compiler.
    - Normally this will happen automatically when configuring CMake, but some issues may arise.
    - If Intellisense is having trouble finding even local files,
        ensure that your C/C++ configuration provider is set to `ms-vscode.cmake-tools`.

NOTE: Be patient with Intellisense while it's initializing, it can take a while to fully parse the project.
Look for the flame icon in the status bar.
