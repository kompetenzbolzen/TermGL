CC      = /usr/bin/clang
CFLAGS  = -Wall -g -std=c++11 -shared -fPIC
DEBUGFLAGS = -Wall -g -std=c++11
LDFLAGS =
SONAME = engine
BUILDDIR = build
SOURCEDIR = src
TESTSOURCE = test
#VERSION
VERSION = 0
PATCHLEVEL = 4
OUTPUT = lib$(SONAME).so.$(VERSION).$(PATCHLEVEL)

OBJ = cObject.o cObjectHandler.o cRender.o cInput.o cWiremesh.o

build: dir genversion $(OBJ)
	$(CC) $(CFLAGS) -o $(BUILDDIR)/lib/$(OUTPUT) $(OBJ) $(LDFLAGS) -Wl,-soname=lib$(SONAME).so.$(VERSION)
	ln -sf $(OUTPUT) $(BUILDDIR)/lib/lib$(SONAME).so.$(VERSION)
	ln -sf $(OUTPUT) $(BUILDDIR)/lib/lib$(SONAME).so
	cp src/c*.h $(BUILDDIR)/inc
	cp src/version.h $(BUILDDIR)/inc

dir:
	mkdir -p $(BUILDDIR)
	mkdir -p $(BUILDDIR)/lib
	mkdir -p $(BUILDDIR)/inc


%.o: $(SOURCEDIR)/%.cpp
	@echo
	@echo Building $<
	@echo ==============
	@echo
	$(CC) $(CFLAGS) -c $<

%.o: example/%.cpp
	$(CC) $(CFLAGS) -I$(SOURCEDIR) -c $<


all: clean build

.PHONY: clean

clean:
	rm -df  $(OBJ) $(TESTSOURCE).o src/version.h
	rm -Rdf $(BUILDDIR)/lib $(BUILDDIR)/inc $(BUILDDIR)/test doc/

run: gentest
	./$(BUILDDIR)/test/test

memleak: gentest
	valgrind -v --track-origins=yes "./$(BUILDDIR)/test/test"

genversion:
	@echo Building Version
	@echo "//Generated by MAKEFILE. DO NOT Edit." > $(SOURCEDIR)/version.h
	@echo "#pragma once" >> $(SOURCEDIR)/version.h
	@echo "#define VERSION $(VERSION)" >> $(SOURCEDIR)/version.h
	@echo "#define PATCHLEVEL $(PATCHLEVEL)" >> $(SOURCEDIR)/version.h
	@echo "#define VERSTRING \"`git describe`\"" >> $(SOURCEDIR)/version.h
	@echo "#define DATE \"`date +'%d.%m.20%y'`\"" >> $(SOURCEDIR)/version.h
	@echo "#define TIME \"`date +'%H:%M:%S'`\"" >> $(SOURCEDIR)/version.h
	@echo "#define BUILDER \"`git config user.name`\"" >> $(SOURCEDIR)/version.h
	@echo "#define BUILDERMAIL \"`git config user.email`\"" >> $(SOURCEDIR)/version.h

gentest: build $(TESTSOURCE).o
	mkdir -p $(BUILDDIR)/test
	$(CC) $(DEBUGFLAGS) -o $(BUILDDIR)/test/test $(TESTSOURCE).o $(OBJ) $(LDFLAGS)

doc:
	mkdir -p doc
	doxygen .doxygen
