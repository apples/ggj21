$MSYS2_URL = 'https://github.com/msys2/msys2-installer/releases/download/2020-06-29/msys2-base-x86_64-20200629.sfx.exe'
$PKG = "$PSScriptRoot\.pkg"
$TMP = "$PSScriptRoot\.tmp"

if (!(Test-Path "$PKG")) {
    New-Item -Path "$PSScriptRoot" -Name ".tmp" -ItemType "directory"
    New-Item -Path "$PSScriptRoot" -Name ".pkg" -ItemType "directory"
    wget "$MSYS2_URL" -OutFile "$TMP\msys2.exe"
    & "$TMP\msys2.exe" -y "-o$PKG\"

    & "$PKG\msys64\usr\bin\bash" -lc ' '
    & "$PKG\msys64\usr\bin\bash" -lc 'pacman -Syuu'
    & "$PKG\msys64\usr\bin\bash" -lc 'pacman -Syuu'
    & "$PKG\msys64\usr\bin\bash" -lc 'pacman -S --noconfirm mingw-w64-x86_64-SDL2 mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-gcc'
}

echo Done
