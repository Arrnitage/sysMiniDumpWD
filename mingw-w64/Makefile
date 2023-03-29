CC_x64 := x86_64-w64-mingw32-gcc
OPTIONS := -masm=intel -Wall -ldbghelp

all:
	$(CC_x64) src/lsassdmp.c src/sysMiniDumpWD.c -o dst/sysMiniDumpWD.x64.exe $(OPTIONS)

.PHONY: clean
clean:
	rm -rf dst/*