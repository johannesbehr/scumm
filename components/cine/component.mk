#
# component makefile.
#
# This Makefile can be left empty. By default, it will take the sources in the
# src/ directory, compile them and link them into lib(subdirectory_name).a
# in the build directory. This behaviour is entirely configurable,
# please read the ESP-IDF documents if you need to do this.
#
COMPONENT_SRCDIRS := .
COMPONENT_ADD_INCLUDEDIRS += ./..
CPPFLAGS += -Wno-error=reorder -Wno-reorder -Wno-error=unknown-pragmas -Wno-unknown-pragmas -Wno-multichar