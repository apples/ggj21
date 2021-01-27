$PKG = "$PSScriptRoot\.pkg"

& "$PKG\msys64\msys2_shell.cmd" -mingw64 -c "'/c/Program Files/Microsoft VS Code/Code.exe' '$PSScriptRoot' &"
