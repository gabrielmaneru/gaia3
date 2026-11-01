@echo off
echo Starting clang-format...

:: Define the path to clang-format
set "CF=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\Llvm\x64\bin\clang-format.exe"

:: Recursively search and format the files
FOR /R %%F IN (*.h *.hpp *.cpp *.cxx) DO (
    "%CF%" -i "%%F"
)

echo Process completed.
pause