PREFIX    := /usr/local
BINPREFIX := $(DESTDIR)$(PREFIX)/bin
MANPREFIX := $(DESTDIR)$(PREFIX)/share/man/man1

SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)
DEP := $(OBJ:.o=.d)
PRE := $(wildcard sh/prev/*)
GEN := gen/previews.h gen/server.h gen/helpers.h

LIBS := magic crypto

ALL_CFLAGS  := -O2 -MMD -Wall -Wextra -Wno-unused-parameter $(CFLAGS) $(CPPFLAGS)
ALL_LDFLAGS := $(addprefix -l,$(LIBS)) $(CFLAGS) $(LDFLAGS)

MKDIR        := mkdir -p
INSTALL      := install
INSTALL_EXE  := $(INSTALL)
INSTALL_DATA := $(INSTALL) -m 0644

all: ctpv

options:
	@echo "CC      = $(CC)"
	@echo "CFLAGS  = $(ALL_CFLAGS)"
	@echo "LDFLAGS = $(ALL_LDFLAGS)"

install: install.bin install.man

install.bin: ctpv quit/ctpvquit ctpvclear
	$(MKDIR) $(BINPREFIX)
	$(INSTALL_EXE) $^ $(BINPREFIX)

install.man: doc/ctpv.1
	$(MKDIR) $(MANPREFIX)
	$(INSTALL_DATA) $^ $(MANPREFIX)

uninstall:
	$(RM) $(BINPREFIX)/ctpv $(BINPREFIX)/ctpvquit $(BINPREFIX)/ctpvclear \
		$(MANPREFIX)/ctpv.1

clean:
	$(RM) ctpv $(OBJ) $(DEP) $(GEN)
	$(MAKE) -C embed clean
	$(MAKE) -C quit clean

docs: README.md doc/ctpv.1
	deptable/list.awk $(PRE) | deptable/markdown.sed | deptable/insert.awk README.md
	deptable/list.awk $(PRE) | deptable/roff.sed | deptable/insert.awk doc/ctpv.1

ctpv: $(OBJ)
	$(CC) -o $@ $+ $(ALL_LDFLAGS)

.c.o:
	$(CC) -o $@ $< -c $(ALL_CFLAGS)

# Exclicit rules for generated header files
src/ctpv.o: gen/previews.h
src/shell.o: gen/helpers.h
src/server.o: gen/server.h

gen/previews.h: $(PRE) embed/embed
	embed/embed -p prev_scr_ $(PRE) > $@

gen/server.h: sh/clear.sh sh/end.sh embed/embed
	embed/embed -p scr_ sh/clear.sh sh/end.sh > $@

gen/helpers.h: sh/helpers.sh embed/embed
	embed/embed -p scr_ sh/helpers.sh > $@

$(GEN): | gen

gen:
	$(MKDIR) $@

embed/embed: .force
	$(MAKE) -C embed

quit/ctpvquit: .force
	$(MAKE) -C quit

-include $(DEP)

.PHONY: all options install install.bin install.man uninstall \
	clean docs .force

.DELETE_ON_ERROR:
