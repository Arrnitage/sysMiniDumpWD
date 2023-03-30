CC_x64 := x86_64-w64-mingw32-gcc
STRIP_x64 := x86_64-w64-mingw32-strip
OPTIONS := -masm=intel -Wall -ldbghelp -mwindows -m64 -Os

all:
	$(CC_x64) src/lsassdmp.c src/sysMiniDumpWD.c -o dst/temp.x64.exe $(OPTIONS)
	$(STRIP_x64) -s dst/temp.x64.exe -o dst/sysMiniDumpWD.x64.exe
	rm -f dst/temp.x64.exe


.PHONY: clean
clean:
	rm -rf dst/sysMiniDumpWD.x64.exe
