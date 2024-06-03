CC = ${CROSS_COMPILE}gcc
CXX = ${CROSS_COMPILE}g++

CFLAGS = -Wall -Wextra -Wno-unused-parameter -O2 -DNO_OPENSSL=1
ifeq ($(KERNEL_VERSION_4),y)
CFLAGS += -DKERNEL_VERSION_4
endif
CXXFLAGS = $(CFLAGS) -std=c++20
LDFLAGS = -lrt
LIBS = -limp -lalog -lsysutils -lmuslshim -lliveMedia -lgroupsock -lBasicUsageEnvironment -lUsageEnvironment -lconfig++ -lfreetype

ifneq (,$(findstring -DPLATFORM_T31,$(CFLAGS)))
    LIBIMP_INC_DIR = ./include/T31
else ifneq (,$(or $(findstring -DPLATFORM_T20,$(CFLAGS)), $(findstring -DPLATFORM_T10,$(CFLAGS))))
    LIBIMP_INC_DIR = ./include/T20
else ifneq (,$(findstring -DPLATFORM_T21,$(CFLAGS)))
    LIBIMP_INC_DIR = ./include/T21
else ifneq (,$(findstring -DPLATFORM_T23,$(CFLAGS)))
    LIBIMP_INC_DIR = ./include/T23
else ifneq (,$(findstring -DPLATFORM_T30,$(CFLAGS)))
    LIBIMP_INC_DIR = ./include/T30
else
    LIBIMP_INC_DIR = ./include/T31
endif

SRC_DIR = ./src
OBJ_DIR = ./obj
BIN_DIR = ./bin

SOURCES = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.cpp)) \
          $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.c)) \

$(info $(OBJECTS))

TARGET = $(BIN_DIR)/prudynt

ifndef commit_tag
commit_tag=$(shell git rev-parse --short HEAD)
endif

VERSION_FILE = $(LIBIMP_INC_DIR)/version.hpp

$(VERSION_FILE): $(SRC_DIR)/version.tpl.hpp
	@if ! grep -q "$(commit_tag)" version.h > /dev/null 2>&1; then \
		echo "Updating version.h to $(commit_tag)"; \
		sed 's/COMMIT_TAG/"$(commit_tag)"/g' $(SRC_DIR)/version.tpl.hpp > $(VERSION_FILE); \
	fi

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(VERSION_FILE)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -I$(LIBIMP_INC_DIR) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(VERSION_FILE)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(LIBIMP_INC_DIR) -c $< -o $@

$(TARGET): $(OBJECTS) $(VERSION_FILE)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

.PHONY: all clean

all: $(TARGET)

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(LIBIMP_INC_DIR)/version.hpp

distclean: clean
	rm -rf $(BIN_DIR)
