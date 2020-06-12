LIBS = "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" \
	"advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" \
	"odbc32.lib" "odbccp32.lib"
SRCS = kuroko-cli.cpp Makefile

all: build_debug

build_debug: $(SRCS)
	cl /EHsc /Zi /DEBUG /MTd /c kuroko-cli.cpp
	link /DEBUG $(LIBS) kuroko-cli.obj
	@echo ==== Build Successful ====

build_release: $(SRCS)
	cl /O2 /EHsc /MT /c kuroko-cli.cpp
	link $(LIBS) kuroko-cli.obj
	@echo ==== Build Successful ====

run: build_debug
	kuroko-cli.exe -b test.pdf

clean:
	del *.obj *.exe *.ilk *.pdb

pack:
	./build_shell.bat nmake build_release
	zip -r kuroko-cli.zip kuroko-cli.exe *.md docs
