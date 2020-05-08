LIBS = "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" \
	"advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" \
	"odbc32.lib" "odbccp32.lib"
SRCS = kuroko.cpp Makefile

all: build_debug

build_debug: $(SRCS)
	cl /EHsc /Zi /DEBUG /MTd /c kuroko.cpp
	link /DEBUG $(LIBS) kuroko.obj
	@echo ==== Build Successful ====

build_release: $(SRCS)
	cl /O2 /EHsc /MT /c kuroko.cpp
	link $(LIBS) kuroko.obj
	@echo ==== Build Successful ====

run: build_debug
	kuroko.exe

clean:
	del *.obj *.exe *.ilk *.pdb
