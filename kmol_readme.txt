
stop proxy setup

pip install numpy

pip install scipy

pip install matplotlib

pip install transforms3d

msys2_portable

Before Python 3.12.3

    1. use python.exe to execute tmp/get-pip.py

    need to close system proxy setup first.

    cd tmp

    python get-pip.py

    complete the pip.exe installation.
    
    2. pip install flask flask_cors lxml bs4 pelican markdown leo
    
After Python 3.12.3

    python -m pip install

3. execute Y:\msys64_20240507\mingw64.exe

pacman -Syu
pacman -Sy
pacman -S mingw-w64-x86_64-toolchain

4. compile lua

Y:\lua-5.4.3>mingw32-make mingw

5. compile wscite

https://github.com/mirror/scite

https://github.com/mirror/scintilla

利用 MSYS2 編譯 SciTE 編輯器:

首先必須確認 y:\portable\bin 中 sh.exe 已經改名, 因為 shell 指令將取自 MSYS2 系統中的對應指令.

分別 git clone scite 與 scintilla, 兩個目錄並列.

git clone https://github.com/mirror/scite.git 

git clone https://github.com/mirror/scintilla.git

cd scintilla\win32

mingw32-make

get Scintilla.dll and SciLexer.dll under scintilla\bin

cd scite\win32

mingw32-make

get SciTE.exe under scite\bin

need extra libgcc_s_seh-1.dll, libstdc++-6.dll and libwinpthread-1.dll along with Scintilla.dll and SciLexer.dll to execute SciTE.exe

截至目前 msys2 容量約 1.5 GB

compile solvespace

git clone --recurse-submodules https://github.com/solvespace/solvespace.git

cd solvespace
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
mingw32-make

compile V-rep (coppeliasim)

https://2019wcm.blogspot.com/2019/03/v-rep.html