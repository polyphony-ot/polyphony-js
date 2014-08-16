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
AR=ar

SOURCES=$(wildcard lib/*.js)
TESTS=$(wildcard test/*.js)

# The path to the libot source.
LIBOT=native/libot

# Output directory where JS files will be placed.
BIN=bin

# Filename to use for the library.
LIB=polyphony.js

# Emscripten settings
EMCC_SETTINGS=\
	-s EXPORTED_FUNCTIONS=@exported-functions.json \
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

$(BIN)/debug/polyphony-emscripten.js: $(LIBOT)/bin/debug/libot.a \
exported-functions.json
	mkdir -p $(BIN)/debug
	$(CC) $(CFLAGS) -g4 \
	$(EMCC_SETTINGS) \
	-o $(BIN)/debug/polyphony-emscripten.js $(LIBOT)/bin/debug/libot.a

debug: $(BIN)/debug/polyphony.js

# Release targets #

$(BIN)/release/polyphony-emscripten.js: $(LIBOT)/bin/release/libot.a \
exported-functions.json
	mkdir -p $(BIN)/release
	$(CC) $(CFLAGS) -DNDEBUG -O2 -g4 \
	$(EMCC_SETTINGS) \
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
