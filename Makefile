#
# Makefile to compile all versions
#

.PHONY: all clean test test-win32 test-win64

TEST_BIN = obj/test_tzx_blocks
TEST_BIN_WIN32 = obj/test_tzx_blocks_win32.exe
TEST_BIN_WIN64 = obj/test_tzx_blocks_win64.exe
TEST_SOURCES = tests/test_tzx_blocks.cpp ByteBuffer.cpp TZX_Blocks.cpp
TEST_FLAGS = -std=gnu++11 -Wall -Wextra -I./includes -I. -Wignored-qualifiers

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

test:
	@mkdir -p obj
	$(CXX) $(TEST_FLAGS) $(TEST_SOURCES) -o $(TEST_BIN)
	@$(TEST_BIN)

test-win32:
	@mkdir -p obj
	i686-w64-mingw32-g++ $(TEST_FLAGS) -static -static-libgcc -static-libstdc++ $(TEST_SOURCES) -o $(TEST_BIN_WIN32)
	@WINEDEBUG=-all wine $(TEST_BIN_WIN32)

test-win64:
	@mkdir -p obj
	x86_64-w64-mingw32-g++ $(TEST_FLAGS) -static -static-libgcc -static-libstdc++ $(TEST_SOURCES) -o $(TEST_BIN_WIN64)
	@WINEDEBUG=-all wine $(TEST_BIN_WIN64)

clean:
	rm -f obj/*.o $(TEST_BIN) $(TEST_BIN_WIN32) $(TEST_BIN_WIN64) *~ core $(IDIR)/*~
