#
# Makefile to compile all versions
#

.PHONY: all clean

all: compileAll

compileAll:
	@echo ================================ Linux64
	@make -f Makefile.linux64 clean
	@make -f Makefile.linux64 -j all
	@echo ================================ Linux32
	@make -f Makefile.linux32 clean
	@make -f Makefile.linux32 -j all
	@echo ================================ Win64
	@make -f Makefile.win64 clean
	@make -f Makefile.win64 -j all
	@echo ================================ Win32
	@make -f Makefile.win32 clean
	@make -f Makefile.win32 -j all

clean:
	rm -f obj/*.o *~ core $(IDIR)/*~
