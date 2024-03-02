TOP = $(shell realpath ../../)

CC = $(TOP)/host/bin/mipsel-linux-gcc
CXX = $(TOP)/host/bin/mipsel-linux-g++
CCACHE = ccache
CFLAGS = -Wall -Wextra -Wno-unused-parameter -O2 -DNO_OPENSSL=1
CXXFLAGS = $(CFLAGS) -std=c++20
LDFLAGS = -lrt
LIBS = -limp -lalog -lliveMedia -lgroupsock -lBasicUsageEnvironment -lUsageEnvironment -lconfig++

LIVE555_INCLUDE_DIR = $(TOP)/staging/usr/include
LIVE555_LIB_DIR = $(TOP)/target/usr/lib
BLOB_INCLUDE_DIR = ./blob/include

SRC_DIR = ./src
OBJ_DIR = ./obj
BIN_DIR = ./bin

SOURCES = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.cpp)) \
          $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.c))

$(info $(OBJECTS))

TARGET = $(BIN_DIR)/prudynt

$(info Using LIVE555 inc dir: $(LIVE555_INCLUDE_DIR))
$(info Using BLOB_INCLUDE_DIR: $(BLOB_INCLUDE_DIR))

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CCACHE) $(CXX) $(CXXFLAGS) -I$(BLOB_INCLUDE_DIR) -I$(LIVE555_INCLUDE_DIR) -I$(LIVE555_INCLUDE_DIR)/liveMedia -I$(LIVE555_INCLUDE_DIR)/groupsock -I$(LIVE555_INCLUDE_DIR)/UsageEnvironment -I$(LIVE555_INCLUDE_DIR)/BasicUsageEnvironment -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CCACHE) $(CC) $(CFLAGS) -I$(BLOB_INCLUDE_DIR) -c $< -o $@

$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CCACHE) $(CXX) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS) -L$(LIVE555_LIB_DIR)

.PHONY: all clean

all: $(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
