@echo off
set CFLAGS=-EHsc -nologo -W4 -Zi -MTd -DDEBUG -fsanitize=address -std:clatest

cl.exe %CFLAGS% %1
del *.obj
del *.pdb