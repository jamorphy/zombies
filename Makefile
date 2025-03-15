CC       = clang
EMCC     = emcc -sASSERTIONS=1 -sGL_DEBUG=1 -sDEMANGLE_SUPPORT=1 -sSTACK_OVERFLOW_CHECK=2 -sSAFE_HEAP=1
SHDC     = ./sokol-shdc

CFLAGS   = -std=c11 -O2 -Wall -Wextra
OBJC_FLAGS = -fobjc-arc
INCLUDES = -I$(CURDIR)/libs/sokol \
           -I$(CURDIR)/libs/nuklear \
           -I$(CURDIR)/libs/linmath \

FRAMEWORKS = -framework Metal -framework MetalKit \
             -framework Cocoa -framework AudioToolbox \
             -framework QuartzCore -framework Foundation

BUILD_DIR = build
OBJ_DIR   = $(BUILD_DIR)/obj

DEBUG_FLAGS  = -g -O0 -DDEBUG

#-----------------------------------------------------------------
# Source Files
#-----------------------------------------------------------------
SRC_C_FILES  = main.c ecs.c input.c gui.c transform.c render.c math_utils.c camera.c physics.c projectile.c event.c
SOKOL_FILES  = sokol.m        # for native Metal

SOKOL_C_FILES = sokol.c
SOKOL_C_PATHS = $(addprefix libs/sokol/,$(SOKOL_C_FILES))

SRC_C_PATHS  = $(addprefix src/,$(SRC_C_FILES))
SOKOL_PATHS  = $(addprefix libs/sokol/,$(SOKOL_FILES))

OBJS = $(SRC_C_FILES:%.c=$(OBJ_DIR)/src/%.o)
OBJS += $(SOKOL_FILES:%.m=$(OBJ_DIR)/libs/sokol/%.o)

SHADER_IN  = src/shaders/cube.glsl
SHADER_OUT = src/cube.glsl.h

# We want both metal_macos (native) and glsl300es (web)
SHDC_FLAGS = --slang=metal_macos:glsl300es

#-----------------------------------------------------------------
# Final Targets
#-----------------------------------------------------------------
NATIVE_TARGET   = $(BUILD_DIR)/demo_native
WEB_TARGET      = $(BUILD_DIR)/demo.html

.PHONY: all native web clean directories

all: native # default

debug: CFLAGS += $(DEBUG_FLAGS)
debug: native

$(SHADER_OUT): $(SHADER_IN) | directories
	$(SHDC) -i $< -o $@ $(SHDC_FLAGS)

#-----------------------------------------------------------------
# Build object files for native
#-----------------------------------------------------------------
$(OBJ_DIR)/src/%.o: src/%.c $(SHADER_OUT) | directories
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# We compile main.c as Objective-C if needed:
$(OBJ_DIR)/src/main.o: src/main.c $(SHADER_OUT) | directories
	$(CC) $(CFLAGS) $(INCLUDES) -x objective-c $(OBJC_FLAGS) -c $< -o $@

# .m -> .o (for sokol.m)
$(OBJ_DIR)/libs/sokol/%.o: libs/sokol/%.m | directories
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJC_FLAGS) -c $< -o $@

#-----------------------------------------------------------------
# Native Build (Metal)
#-----------------------------------------------------------------
native: $(NATIVE_TARGET)

$(NATIVE_TARGET): $(OBJS)
	$(CC) $(OBJS) $(FRAMEWORKS) -o $@

#-----------------------------------------------------------------
# Web Build (WASM + WebGL2)
#-----------------------------------------------------------------
web: directories $(SHADER_OUT)
	$(EMCC) $(CFLAGS) $(INCLUDES) \
	    -sUSE_WEBGL2=1 \
	    -DSOKOL_GLES3 \
		--shell-file=./src/shell.html \
	    $(SRC_C_PATHS) $(SOKOL_C_PATHS)  \
	    -o $(WEB_TARGET)

directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OBJ_DIR)/src
	@mkdir -p $(OBJ_DIR)/libs/sokol

clean:
	rm -rf $(BUILD_DIR)
