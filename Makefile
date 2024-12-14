CC = ${CROSS_COMPILE}gcc
CXX = ${CROSS_COMPILE}g++

DEBUG=n

CXXFLAGS += $(CFLAGS) -std=c++20 -Wall -Wextra -Wno-unused-parameter
LDFLAGS += -lrt -lpthread

CFLAGS ?= -Wall -Wextra -Wno-unused-parameter -O2 -DNO_OPENSSL=1
ifeq ($(KERNEL_VERSION_4),y)
CFLAGS += -DKERNEL_VERSION_4
endif

ifneq ($(filter -DBINARY_STATIC -DBINARY_HYBRID -DBINARY_DYNAMIC,$(CFLAGS)),)
else
override CFLAGS += -DBINARY_DYNAMIC
endif

ifneq ($(MAKECMDGOALS),clean)
ifneq (,$(findstring -DBINARY_STATIC,$(CFLAGS)))
override LDFLAGS += -static -static-libgcc -static-libstdc++
LIBS = -l:libimp.a -l:libalog.a -l:libsysutils.a -l:libmuslshim.a \
	-l:libliveMedia.a -l:libgroupsock.a -l:libBasicUsageEnvironment.a -l:libUsageEnvironment.a \
	-l:libconfig++.a -l:libwebsockets.a -l:libschrift.a -l:libopus.a -l:libfaac.a
else ifneq (,$(findstring -DBINARY_HYBRID,$(CFLAGS)))
override LDFLAGS += -static-libstdc++
LIBS = -Wl,-Bdynamic -l:libimp.so -l:libalog.so -l:libsysutils.so -l:libmuslshim.so -l:libaudioProcess.so -l:libaudioshim.so -l:libwebsockets.so \
	-Wl,-Bstatic -l:libliveMedia.a -l:libgroupsock.a -l:libBasicUsageEnvironment.a -l:libUsageEnvironment.a \
	-l:libconfig++.a -l:libschrift.a -l:libopus.a -l:libfaac.a \
	-Wl,-Bdynamic
else ifneq (,$(findstring -DBINARY_DYNAMIC,$(CFLAGS)))
LIBS = -Wl,-Bdynamic -l:libimp.so -l:libalog.so -l:libaudioProcess.so -l:libsysutils.so -l:libmuslshim.so \
	-l:libliveMedia -l:libgroupsock -l:libBasicUsageEnvironment -l:libUsageEnvironment.so \
	-l:libconfig++.so -l:libwebsockets.so -l:libschrift.so -l:libopus.so -l:libfaac.so
else
$(error No valid binary type defined in CFLAGS. Please specify -DBINARY_STATIC, -DBINARY_HYBRID, or -DBINARY_DYNAMIC)
endif
endif

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

LIBWEBSOCKETS = ./3rdparty/install/include/
VERSION_FILE  = $(LIBIMP_INC_DIR)/version.hpp

$(VERSION_FILE): $(SRC_DIR)/version.tpl.hpp
	@if ! grep -q "$(commit_tag)" version.h > /dev/null 2>&1; then \
		echo "Updating version.h to $(commit_tag)"; \
		sed 's/COMMIT_TAG/"$(commit_tag)"/g' $(SRC_DIR)/version.tpl.hpp > $(VERSION_FILE); \
	fi

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(VERSION_FILE)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -I$(LIBIMP_INC_DIR) -I$(LIBWEBSOCKETS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(VERSION_FILE) 
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(LIBIMP_INC_DIR) -I$(LIBWEBSOCKETS) -c $< -o $@

$(TARGET): $(OBJECTS) $(VERSION_FILE)
	@mkdir -p $(@D)
	$(CCACHE) $(CXX) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS) -s

.PHONY: all clean

all: $(TARGET)

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(LIBIMP_INC_DIR)/version.hpp

distclean: clean
	rm -rf $(BIN_DIR)
