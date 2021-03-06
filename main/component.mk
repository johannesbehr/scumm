#
# Main component makefile.
#
# This Makefile can be left empty. By default, it will take the sources in the
# src/ directory, compile them and link them into lib(subdirectory_name).a
# in the build directory. This behaviour is entirely configurable,
# please read the ESP-IDF documents if you need to do this.
#
COMPONENT_ADD_INCLUDEDIRS += ./..
COMPONENT_ADD_INCLUDEDIRS += ./../engines
COMPILEDATE:=\"$(shell date "+%Y%m%d")\"
GITREV:=\"$(shell git rev-parse HEAD | cut -b 1-10)\"

CFLAGS += -DCOMPILEDATE="$(COMPILEDATE)" -DGITREV="$(GITREV)"
CPPFLAGS += -Ofast -DCOMPILEDATE="$(COMPILEDATE)" -DGITREV="$(GITREV)"
CPPFLAGS += -Wno-error=reorder -Wno-reorder -Wno-error=unknown-pragmas -Wno-unknown-pragmas -Wno-multichar