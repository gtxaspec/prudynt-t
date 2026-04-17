# =============================================================================
# Prudynt-T Makefile
# =============================================================================

# Compiler Configuration
# ----------------------
CC                      = ${CROSS_COMPILE}gcc
CXX                     = ${CROSS_COMPILE}g++

# Compiler Flags
# --------------
CFLAGS                 ?= -Wall -Wextra -Wno-unused-parameter -O2 -DNO_OPENSSL=1
CXXFLAGS               += $(CFLAGS) -std=c++20 -Wall -Wextra -Wno-unused-parameter
LDFLAGS                += -lrt -lpthread -latomic

# Kernel Version Support
# ----------------------
ifeq ($(KERNEL_VERSION_4),y)
CFLAGS                 += -DKERNEL_VERSION_4
endif

# Binary Type Configuration
# -------------------------
# Default to dynamic linking unless explicitly specified
ifneq ($(filter -DBINARY_STATIC -DBINARY_HYBRID,$(CFLAGS)),)
# Static or hybrid build explicitly requested
else
override CFLAGS        += -DBINARY_DYNAMIC
endif

# Library Configuration
# =====================
# Check for libc type from CFLAGS, default to musl if not specified
# We add libmuslshim only when using musl (default if no libc type specified)

ifneq ($(MAKECMDGOALS),clean)

# Static Binary Configuration
# ---------------------------
ifneq (,$(findstring -DBINARY_STATIC,$(CFLAGS)))
override LDFLAGS       += -static -static-libgcc -static-libstdc++
LIBS                    = -l:libimp.a \
                          -l:libalog.a \
                          -l:libsysutils.a \
                          -l:libliveMedia.a \
                          -l:libgroupsock.a \
                          -l:libBasicUsageEnvironment.a \
                          -l:libUsageEnvironment.a \
                          -l:libwebsockets.a \
                          -l:libschrift.a \
                          -l:libopus.a \
                          -l:libfaac.a \
                          -l:libhelix-aac.a \
                          -l:libjson-c.a

ifneq (,$(findstring -DLIBC_GLIBC,$(CFLAGS)))
	# GLIBC - no additional libraries needed
else ifneq (,$(findstring -DLIBC_UCLIBC,$(CFLAGS)))
	# uClibc - no additional libraries needed
else
	# Default to musl
LIBS                   += -l:libmuslshim.a
endif

# Hybrid Binary Configuration
# ---------------------------
else ifneq (,$(findstring -DBINARY_HYBRID,$(CFLAGS)))
override LDFLAGS       += -static-libstdc++
LIBS                    = -Wl,-Bdynamic \
                          -l:libimp.so \
                          -l:libalog.so \
                          -l:libsysutils.so \
                          -l:libaudioProcess.so \
                          -l:libaudioshim.so \
                          -l:libwebsockets.so \
                          -Wl,-Bstatic \
                          -l:libliveMedia.a \
                          -l:libgroupsock.a \
                          -l:libBasicUsageEnvironment.a \
                          -l:libUsageEnvironment.a \
                          -l:libschrift.a \
                          -l:libopus.a \
                          -l:libfaac.a \
                          -l:libhelix-aac.a \
                          -Wl,-Bdynamic \
                          -l:libjson-c.so

ifneq (,$(findstring -DLIBC_GLIBC,$(CFLAGS)))
	# GLIBC - no additional libraries needed
else ifneq (,$(findstring -DLIBC_UCLIBC,$(CFLAGS)))
	# uClibc - no additional libraries needed
else
	# Default to musl
LIBS                   := $(LIBS:-Wl,-Bdynamic=-Wl,-Bdynamic -l:libmuslshim.so)
endif

# Dynamic Binary Configuration
# ----------------------------
else ifneq (,$(findstring -DBINARY_DYNAMIC,$(CFLAGS)))
# Force dynamic linking and prevent static fallback
override LDFLAGS       += -Wl,-Bdynamic -Wl,--as-needed
LIBS                    = -limp \
                          -lalog \
                          -laudioProcess \
                          -lsysutils \
                          -lliveMedia \
                          -lgroupsock \
                          -lUsageEnvironment \
                          -lBasicUsageEnvironment \
                          -lwebsockets \
                          -lschrift \
                          -lopus \
                          -lfaac \
                          -lhelix-aac \
                          -ljson-c \
                          -latomic

ifneq (,$(findstring -DLIBC_GLIBC,$(CFLAGS)))
	# GLIBC - no additional libraries needed
else ifneq (,$(findstring -DLIBC_UCLIBC,$(CFLAGS)))
	# uClibc - no additional libraries needed
else
	# Default to musl
LIBS                   += -l:libmuslshim.so \
                          -latomic
endif

# Error Handling
# --------------
else
$(error No valid binary type defined in CFLAGS. Please specify -DBINARY_STATIC, -DBINARY_HYBRID, or -DBINARY_DYNAMIC)
endif

endif

# Platform-Specific Include Directories
# =====================================
ifneq (,$(findstring -DPLATFORM_C100,$(CFLAGS)))
	LIBIMP_INC_DIR          = ./include/C100/2.1.0/en
else ifneq (,$(or $(findstring -DPLATFORM_T20,$(CFLAGS)), $(findstring -DPLATFORM_T10,$(CFLAGS))))
	LIBIMP_INC_DIR          = ./include/T20/3.12.0/zh
else ifneq (,$(findstring -DPLATFORM_T21,$(CFLAGS)))
	LIBIMP_INC_DIR          = ./include/T21/1.0.33/zh
else ifneq (,$(findstring -DPLATFORM_T23,$(CFLAGS)))
	LIBIMP_INC_DIR          = ./include/T23/1.1.0/zh
else ifneq (,$(findstring -DPLATFORM_T30,$(CFLAGS)))
	LIBIMP_INC_DIR          = ./include/T30/1.0.5/zh
else ifneq (,$(findstring -DPLATFORM_T31,$(CFLAGS)))
	LIBIMP_INC_DIR          = ./include/T31/1.1.6/en
else ifneq (,$(findstring -DPLATFORM_T40,$(CFLAGS)))
	LIBIMP_INC_DIR          = ./include/T40/1.2.0/zh
else ifneq (,$(findstring -DPLATFORM_T41,$(CFLAGS)))
	LIBIMP_INC_DIR          = ./include/T41/1.2.0/zh
else
	# Default platform
	LIBIMP_INC_DIR          = ./include/T31/1.1.6/en
endif

# Directory Structure
# ===================
SRC_DIR                 = ./src
OBJ_DIR                 = ./obj
BIN_DIR                 = ./bin

# Source and Object Files
# =======================
SOURCES                 = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*.c)
OBJECTS                 = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.cpp)) \
                          $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.c))

$(info Building objects: $(OBJECTS))

# Target Configuration
# ====================
TARGET                  = $(BIN_DIR)/prudynt

# Version Management
# ==================
ifndef commit_tag
# Always use timestamp for development builds to track local changes
current_timestamp       = $(shell date +%s)
git_hash                = $(shell git rev-parse --short HEAD 2>/dev/null || echo "unknown")
commit_tag              = $(git_hash)-dev$(current_timestamp)
endif

VERSION_FILE            = $(LIBIMP_INC_DIR)/version.hpp
THIRDPARTY_INC_DIR      = ./3rdparty/install/include

# Build Options
# =============
STRIP_FLAG              := $(if $(filter 0,$(DEBUG_STRIP)),,"-s")

# =============================================================================
# Build Rules
# =============================================================================

# Version File Generation
# -----------------------
$(VERSION_FILE): $(SRC_DIR)/version.tpl.hpp
	@mkdir -p $(dir $(VERSION_FILE))
	@if ! grep -q "$(commit_tag)" $(VERSION_FILE) > /dev/null 2>&1; then \
		echo "Updating $(VERSION_FILE) to $(commit_tag)"; \
		sed 's/COMMIT_TAG/"$(commit_tag)"/g' $(SRC_DIR)/version.tpl.hpp > $(VERSION_FILE); \
	fi

# C++ Object Compilation
# ----------------------
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(VERSION_FILE)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) \
		-I$(LIBIMP_INC_DIR) \
		-I$(LIBIMP_INC_DIR)/imp \
		-I$(LIBIMP_INC_DIR)/sysutils \
		-isystem $(THIRDPARTY_INC_DIR) \
		-c $< -o $@

# C Object Compilation
# --------------------
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(VERSION_FILE)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) \
		-I$(LIBIMP_INC_DIR) \
		-I$(LIBIMP_INC_DIR)/imp \
		-I$(LIBIMP_INC_DIR)/sysutils \
		-isystem $(THIRDPARTY_INC_DIR) \
		-c $< -o $@

# Final Binary Linking
# --------------------
$(TARGET): $(OBJECTS) $(VERSION_FILE)
	@mkdir -p $(@D)
	$(CCACHE) $(CXX) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS) $(STRIP_FLAG)

# =============================================================================
# Phony Targets
# =============================================================================

.PHONY: all clean distclean

# Default Target
# --------------
all: $(TARGET)

# Clean Build Artifacts
# ---------------------
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(OBJ_DIR)
	rm -f $(LIBIMP_INC_DIR)/version.hpp

# Complete Clean
# --------------
distclean: clean
	@echo "Cleaning all generated files..."
	rm -rf $(BIN_DIR)
