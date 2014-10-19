# Makefile for building polyphony.js.
#
# This project depends on the libot C library. It uses Emscripten to compile the
# C code to JS and provides a thin wrapper for invoking the Emscripten
# functions. All build artifacts will be placed somewhere within the "bin"
# directory. The main debug and release targets build a JS file containing the
# entire library.
#
# Main targets:
# 	debug - performs a debug build with source maps and no optimizations.
# 	release - performs a release build with all optimizations enabled.
# 	test - performs a debug build and then runs all unit tests against it.
# 	clean

CC=emcc
CFLAGS=-std=c99 -Wall -funsigned-char -pedantic
AR=emar

SOURCES=$(wildcard lib/*.js)
TESTS=$(wildcard test/*.js)

# The path to the libot source.
LIBOT=native/libot

# Output directory where JS files will be placed.
BIN=bin

# List of all the native C sources.
NATIVE_SOURCES=$(wildcard native/*.c)

# List of object files corresponding to the native C sources.
DEBUG_OBJS=$(addprefix bin/debug/,$(notdir $(NATIVE_SOURCES:.c=.o)))
RELEASE_OBJS=$(addprefix bin/release/,$(notdir $(NATIVE_SOURCES:.c=.o)))

# Filename to use for the library.
LIB=polyphony.js

# Emscripten settings
EMCC_SETTINGS=\
	-s EXPORTED_FUNCTIONS='["_ot_new_op", "_ot_skip", "_ot_insert",            \
		"_ot_delete", "_ot_snapshot", "_ot_encode", "_ot_decode",              \
		"_ot_new_client", "_ot_client_open", "_ot_client_receive",             \
		"_ot_client_apply", "_ot_new_server", "_ot_server_open",               \
		"_ot_server_receive", "_ot_new_doc", "_ot_server_get_doc",             \
		"_ot_doc_get_composed", "_ot_doc_set_max_size", "_ot_client_set_id",   \
		"_malloc"]'                                                            \
	-s RESERVED_FUNCTION_POINTERS=4

all: debug release test docs

# libot targets #

# The following libot targets invoke the makefile in the libot C project. Since
# Emscripten is designed as a drop-in replacement for gcc/clang, we can simply
# override some variables to get JS output instead of native code.

$(LIBOT)/bin/debug/libot.a:
	OS=Emscripten CC=emcc AR=emar make -e -C $(LIBOT) debug

$(LIBOT)/bin/release/libot.a:
	OS=Emscripten CC=emcc AR=emar make -e -C $(LIBOT) release

$(LIBOT)/bin/debug/test.js:
	OS=Emscripten CC=emcc AR=emar EXESUFFIX=.js TESTRUNNER=node make -e \
	-C $(LIBOT) test

libot-clean:
	make -C $(LIBOT) clean

# Debug targets #

$(BIN)/debug/%.o: native/%.c
	mkdir -p $(BIN)/debug
	$(CC) $(CFLAGS) -g4 $(EMCC_SETTINGS) -o $@ $<

$(BIN)/debug/polyphony-emscripten.js: $(LIBOT)/bin/debug/libot.a \
$(DEBUG_OBJS)
	mkdir -p $(BIN)/debug
	$(AR) rs $(LIBOT)/bin/debug/libot.a $(BIN)/debug/*.o
	rm $(BIN)/debug/*.o
	$(CC) $(CFLAGS) -g4 $(EMCC_SETTINGS) \
	-o $(BIN)/debug/polyphony-emscripten.js $(LIBOT)/bin/debug/libot.a

debug: $(BIN)/debug/polyphony.js

# Release targets #

$(BIN)/release/%.o: native/%.c
	mkdir -p $(BIN)/release
	$(CC) $(CFLAGS) -DNDEBUG -O3 -g0 $(EMCC_SETTINGS) -o $@ $<

$(BIN)/release/polyphony-emscripten.js: $(LIBOT)/bin/release/libot.a \
$(RELEASE_OBJS)
	mkdir -p $(BIN)/release
	$(AR) rs $(LIBOT)/bin/release/libot.a $(BIN)/release/*.o
	rm $(BIN)/release/*.o
	$(CC) $(CFLAGS) -DNDEBUG -O3 -g0 $(EMCC_SETTINGS) \
	-o $(BIN)/release/polyphony-emscripten.js $(LIBOT)/bin/release/libot.a

release: $(BIN)/release/polyphony.js

# Test targets #

.PHONY: test
test: $(BIN)/debug/polyphony.js $(LIBOT)/bin/debug/test.js
	eslint test/*.js
	mocha -R spec

# Prepublish targets #

.PHONY: prepublish
prepublish: $(BIN)/debug/polyphony.js $(BIN)/release/polyphony.js
	cp $(BIN)/debug/polyphony.js polyphony-debug.js
	cp $(BIN)/release/polyphony.js polyphony-release.js

# Misc. targets #

$(BIN)/%/polyphony.js: $(BIN)/%/concat.js $(BIN)/%/polyphony-emscripten.js
	sed -e "/\/\* {{lib}} \*\// r $(BIN)/$*/concat.js" \
	-e "/\/\* {{lib}} \*\// d" \
	-e "/\/\* {{emscripten}} \*\// r $(BIN)/$*/polyphony-emscripten.js" \
	-e "/\/\* {{emscripten}} \*\// d" \
	main.js > $(BIN)/$*/polyphony.js

$(BIN)/%/concat.js: $(SOURCES) main.js
	eslint $(SOURCES)
	cat $(SOURCES) > $(BIN)/$*/concat.js

$(BIN)/docs/index.html: $(SOURCES)
	jsdoc --pedantic --destination $(BIN)/docs lib/*.js main.js

docs: $(BIN)/docs/index.html

clean: libot-clean
	rm -rf $(BIN)
