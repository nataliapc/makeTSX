#
# Makefile to compile and run tests
#

.PHONY: test test-win32 test-win64 clean

TEST_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
ROOT_DIR := $(abspath $(TEST_DIR)/..)
OBJ_DIR := $(ROOT_DIR)/obj

TEST_TZX_BIN := $(OBJ_DIR)/test_tzx_blocks
TEST_WAV_BIN := $(OBJ_DIR)/test_wav
TEST_SYNC_BIN := $(OBJ_DIR)/test_sync
TEST_TZX_BIN_WIN32 := $(OBJ_DIR)/test_tzx_blocks_win32.exe
TEST_WAV_BIN_WIN32 := $(OBJ_DIR)/test_wav_win32.exe
TEST_SYNC_BIN_WIN32 := $(OBJ_DIR)/test_sync_win32.exe
TEST_TZX_BIN_WIN64 := $(OBJ_DIR)/test_tzx_blocks_win64.exe
TEST_WAV_BIN_WIN64 := $(OBJ_DIR)/test_wav_win64.exe
TEST_SYNC_BIN_WIN64 := $(OBJ_DIR)/test_sync_win64.exe
TEST_TZX_SOURCES := $(TEST_DIR)/test_tzx_blocks.cpp $(ROOT_DIR)/ByteBuffer.cpp $(ROOT_DIR)/TZX_Blocks.cpp
TEST_WAV_SOURCES := $(TEST_DIR)/test_wav.cpp $(ROOT_DIR)/WAV.cpp
TEST_SYNC_SOURCES := $(TEST_DIR)/test_sync.cpp $(ROOT_DIR)/BlockRipper.cpp $(ROOT_DIR)/WAV.cpp $(ROOT_DIR)/ByteBuffer.cpp $(ROOT_DIR)/TZX_Blocks.cpp $(ROOT_DIR)/rippers/B10_Standard_Ripper.cpp $(ROOT_DIR)/rippers/B11_Custom_Ripper.cpp
TEST_FLAGS := -std=gnu++11 -Wall -Wextra -I$(ROOT_DIR)/includes -I$(ROOT_DIR) -Wignored-qualifiers

test:
	@mkdir -p "$(OBJ_DIR)"
	$(CXX) $(TEST_FLAGS) $(TEST_TZX_SOURCES) -o $(TEST_TZX_BIN)
	$(CXX) $(TEST_FLAGS) $(TEST_WAV_SOURCES) -o $(TEST_WAV_BIN)
	$(CXX) $(TEST_FLAGS) $(TEST_SYNC_SOURCES) -o $(TEST_SYNC_BIN)
	@cd "$(ROOT_DIR)" && "$(TEST_TZX_BIN)"
	@cd "$(ROOT_DIR)" && "$(TEST_WAV_BIN)"
	@cd "$(ROOT_DIR)" && "$(TEST_SYNC_BIN)"

test-win32:
	@mkdir -p "$(OBJ_DIR)"
	i686-w64-mingw32-g++ $(TEST_FLAGS) -static -static-libgcc -static-libstdc++ $(TEST_TZX_SOURCES) -o $(TEST_TZX_BIN_WIN32)
	i686-w64-mingw32-g++ $(TEST_FLAGS) -static -static-libgcc -static-libstdc++ $(TEST_WAV_SOURCES) -o $(TEST_WAV_BIN_WIN32)
	i686-w64-mingw32-g++ $(TEST_FLAGS) -static -static-libgcc -static-libstdc++ $(TEST_SYNC_SOURCES) -o $(TEST_SYNC_BIN_WIN32)
	@cd "$(ROOT_DIR)" && WINEDEBUG=-all wine "$(TEST_TZX_BIN_WIN32)"
	@cd "$(ROOT_DIR)" && WINEDEBUG=-all wine "$(TEST_WAV_BIN_WIN32)"
	@cd "$(ROOT_DIR)" && WINEDEBUG=-all wine "$(TEST_SYNC_BIN_WIN32)"

test-win64:
	@mkdir -p "$(OBJ_DIR)"
	x86_64-w64-mingw32-g++ $(TEST_FLAGS) -static -static-libgcc -static-libstdc++ $(TEST_TZX_SOURCES) -o $(TEST_TZX_BIN_WIN64)
	x86_64-w64-mingw32-g++ $(TEST_FLAGS) -static -static-libgcc -static-libstdc++ $(TEST_WAV_SOURCES) -o $(TEST_WAV_BIN_WIN64)
	x86_64-w64-mingw32-g++ $(TEST_FLAGS) -static -static-libgcc -static-libstdc++ $(TEST_SYNC_SOURCES) -o $(TEST_SYNC_BIN_WIN64)
	@cd "$(ROOT_DIR)" && WINEDEBUG=-all wine "$(TEST_TZX_BIN_WIN64)"
	@cd "$(ROOT_DIR)" && WINEDEBUG=-all wine "$(TEST_WAV_BIN_WIN64)"
	@cd "$(ROOT_DIR)" && WINEDEBUG=-all wine "$(TEST_SYNC_BIN_WIN64)"

clean:
	rm -f $(TEST_TZX_BIN) $(TEST_WAV_BIN) $(TEST_SYNC_BIN) $(TEST_TZX_BIN_WIN32) $(TEST_WAV_BIN_WIN32) $(TEST_SYNC_BIN_WIN32) $(TEST_TZX_BIN_WIN64) $(TEST_WAV_BIN_WIN64) $(TEST_SYNC_BIN_WIN64)
