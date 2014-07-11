# Makefile for building libot.js.
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
VENDORS=
TESTS=$(wildcard test/*.js)

# The path to the libot source.
LIBOT=libot

# Output directory where JS files will be placed.
BIN=bin

# Filename to use for the library.
LIB=libot.js

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

$(BIN)/debug/libot-emscripten.js: $(LIBOT)/bin/debug/libot.a \
exported-functions.json
	mkdir -p $(BIN)/debug
	$(CC) $(CFLAGS) -g4 \
	-s EXPORTED_FUNCTIONS=@exported-functions.json \
	-s RESERVED_FUNCTION_POINTERS=4 \
	-o $(BIN)/debug/libot-emscripten.js $(LIBOT)/bin/debug/libot.a

debug: $(BIN)/debug/libot.js

# Release targets #

$(BIN)/release/libot-emscripten.js: $(LIBOT)/bin/release/libot.a \
exported-functions.json
	mkdir -p $(BIN)/release
	$(CC) $(CFLAGS) -DNDEBUG -O2 \
	-s EXPORTED_FUNCTIONS=@exported-functions.json \
	--closure 1 \
	-o $(BIN)/release/libot-emscripten.js $(LIBOT)/bin/release/libot.a

release: $(BIN)/release/libot.js

# Test targets #

.PHONY: test
test: $(BIN)/debug/libot.js $(LIBOT)/bin/debug/test.js
	eslint test/*.js
	mocha -R spec

# Misc. targets #

$(BIN)/%/libot.js: $(BIN)/%/concat.js $(BIN)/%/concat-vendors.js \
$(BIN)/%/libot-emscripten.js
	sed -e "/\/\* {{lib}} \*\// r $(BIN)/$*/concat.js" \
	-e "/\/\* {{lib}} \*\// d" \
	-e "/\/\* {{emscripten}} \*\// r $(BIN)/$*/libot-emscripten.js" \
	-e "/\/\* {{emscripten}} \*\// d" \
	-e "/\/\* {{vendors}} \*\// r $(BIN)/$*/concat-vendors.js" \
	-e "/\/\* {{vendors}} \*\// d" \
	main.js > $(BIN)/$*/libot.js

$(BIN)/%/concat.js: $(SOURCES) main.js
	eslint $(SOURCES)
	cat $(SOURCES) > $(BIN)/$*/concat.js

$(BIN)/%/concat-vendors.js: $(VENDORS)
ifdef VENDORS
	cat $(VENDORS) > $(BIN)/$*/concat-vendors.js
else
	touch $(BIN)/$*/concat-vendors.js
endif

$(BIN)/docs/index.html: $(SOURCES)
	jsdoc --pedantic --destination $(BIN)/docs lib/*.js main.js

docs: $(BIN)/docs/index.html

clean: libot-clean
	rm -rf $(BIN)
