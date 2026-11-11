# Code Editor - a text editor in portable C99, in two faces from the same
# sources: cedit, for the terminal, and xcedit, in a window of its own
# (GLFW and OpenGL)
#
#   make            build/cedit and build/xcedit
#   make cedit      the terminal one alone (no GLFW needed)
#   make xcedit     the window one
#   make icon       data/xcedit.svg, from src/platform/icon.h (tools/icon-svg.c)
#   make install    into $(PREFIX) (/usr/local; PREFIX=~/.local for a user),
#                   with the manual pages, the menu entry and the icon of
#                   xcedit; DESTDIR too
#   make man-lint   check the manual pages (docs/man) with mandoc
#   make uninstall
#   make clean
#
# Any C99 compiler with a POSIX system: make CC=clang, make CC=alefc ...
#
# The sources:
#   src/core      the text and what is done to it (no screen)
#   src/tui       the screen buffer and the widgets (button, list, ...)
#   src/app       the editor: menus, text area, status bar, dialogs
#   src/platform  the files (posix/files.c), the terminal (posix/term.c),
#                 the window (glfw/gui.c), the colours, the icon

CC      ?= cc
CFLAGS  ?= -O2
CFLAGS  += -std=c99 -Wall -Wextra -pedantic
# the search of the workspace's text: a thread on each core (POSIX)
LDFLAGS += -pthread
BUILD    = build

# the editor, then its two faces: cedit (a terminal) and xcedit (a window:
# GLFW and OpenGL, the text drawn with stb_truetype)
CORE     = src/core/mem.c src/core/buf.c src/core/enc.c src/core/regex.c \
           src/tui/screen.c src/tui/ui.c src/tui/uilist.c \
           src/app/main.c src/app/start.c src/app/editor.c src/app/menu.c src/app/menus.c \
           src/app/status.c src/app/complete.c src/app/finder.c src/app/walk.c src/app/fuzzy.c src/app/scan.c src/app/jumps.c src/app/filter.c src/app/lang.c src/app/search.c src/app/dialogs.c src/app/picker.c \
           src/app/pickfile.c src/app/paths.c src/app/settings.c src/app/syntax.c src/app/langs.c \
           src/platform/posix/files.c src/platform/posix/run.c src/platform/posix/thread.c \
           src/platform/colours.c \
           $(LANGS)
# the languages: a file each (src/app/langs/NAME.c), listed in langs/all.h
LANGS    = $(sort $(wildcard src/app/langs/*.c))
CEDIT    = $(CORE) src/platform/posix/term.c
XCEDIT   = $(CORE) src/platform/glfw/gui.c
HDRS     = $(wildcard src/*/*.h src/platform/*/*.h src/app/langs/*.h)

GUI_PKGS = glfw3 gl
GUI_CFLAGS = $(shell pkg-config --cflags $(GUI_PKGS)) -I/usr/include/stb
GUI_LIBS = $(shell pkg-config --libs $(GUI_PKGS)) -lm

PREFIX  ?= /usr/local
BINDIR   = $(PREFIX)/bin
DATADIR  = $(PREFIX)/share
MANDIR   = $(DATADIR)/man

all: $(BUILD)/cedit $(BUILD)/xcedit

cedit: $(BUILD)/cedit
xcedit: $(BUILD)/xcedit

$(BUILD)/obj/%.o: src/%.c $(HDRS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/cedit: $(patsubst src/%.c,$(BUILD)/obj/%.o,$(CEDIT))
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# the window: its file with the flags of GLFW, stb (not ours: no warnings)
# and the font, a C array made by tools/bin2c
$(BUILD)/obj/platform/glfw/gui.o: src/platform/glfw/gui.c $(HDRS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(GUI_CFLAGS) -c -o $@ $<

$(BUILD)/obj/platform/glfw/stb.o: src/platform/glfw/stb.c
	@mkdir -p $(dir $@)
	$(CC) -std=c99 -O2 $(GUI_CFLAGS) -w -c -o $@ $<

$(BUILD)/bin2c: tools/bin2c.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD)/font.c: $(BUILD)/bin2c data/fonts/FantasqueSansMono-Regular.ttf
	$(BUILD)/bin2c font_regular data/fonts/FantasqueSansMono-Regular.ttf > $@

$(BUILD)/obj/font.o: $(BUILD)/font.c
	@mkdir -p $(dir $@)
	$(CC) -std=c99 -O2 -c -o $@ $<

$(BUILD)/xcedit: $(patsubst src/%.c,$(BUILD)/obj/%.o,$(XCEDIT)) $(BUILD)/obj/platform/glfw/stb.o \
    $(BUILD)/obj/font.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(GUI_LIBS)

# the icon, from its data (as tools/mascot-svg.c of inkaur): in the
# sources, made again when its data or its tool change
icon: data/xcedit.svg

$(BUILD)/icon-svg: tools/icon-svg.c src/platform/icon.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ tools/icon-svg.c

data/xcedit.svg: $(BUILD)/icon-svg
	$(BUILD)/icon-svg > $@.tmp && mv $@.tmp $@

# the programs; for xcedit its entry in the menus of the system and its
# icon (the caches of the desktop brought up to date, when there)
install: all data/xcedit.svg
	mkdir -p $(DESTDIR)$(BINDIR) $(DESTDIR)$(DATADIR)/applications \
	    $(DESTDIR)$(MANDIR)/man1 $(DESTDIR)$(MANDIR)/pt_BR/man1 \
	    $(DESTDIR)$(DATADIR)/icons/hicolor/scalable/apps $(DESTDIR)$(DATADIR)/licenses/cedit
	install -m 755 $(BUILD)/cedit $(BUILD)/xcedit $(DESTDIR)$(BINDIR)/
	install -m 644 docs/man/cedit.1 $(DESTDIR)$(MANDIR)/man1/
	ln -sf cedit.1 $(DESTDIR)$(MANDIR)/man1/xcedit.1
	install -m 644 docs/man/pt_BR/cedit.1 $(DESTDIR)$(MANDIR)/pt_BR/man1/
	ln -sf cedit.1 $(DESTDIR)$(MANDIR)/pt_BR/man1/xcedit.1
	install -m 644 data/xcedit.desktop $(DESTDIR)$(DATADIR)/applications/
	install -m 644 data/xcedit.svg $(DESTDIR)$(DATADIR)/icons/hicolor/scalable/apps/
	install -m 644 LICENSE $(DESTDIR)$(DATADIR)/licenses/cedit/
	-[ -n "$(DESTDIR)" ] || update-desktop-database -q $(DATADIR)/applications 2>/dev/null
	-[ -n "$(DESTDIR)" ] || gtk-update-icon-cache -q -t $(DATADIR)/icons/hicolor 2>/dev/null
	-[ -n "$(DESTDIR)" ] || makewhatis $(MANDIR) 2>/dev/null || mandb -q 2>/dev/null

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/cedit $(DESTDIR)$(BINDIR)/xcedit \
	    $(DESTDIR)$(DATADIR)/applications/xcedit.desktop \
	    $(DESTDIR)$(DATADIR)/icons/hicolor/scalable/apps/xcedit.svg \
	    $(DESTDIR)$(MANDIR)/man1/cedit.1 $(DESTDIR)$(MANDIR)/man1/xcedit.1 \
	    $(DESTDIR)$(MANDIR)/pt_BR/man1/cedit.1 $(DESTDIR)$(MANDIR)/pt_BR/man1/xcedit.1
	rm -rf $(DESTDIR)$(DATADIR)/licenses/cedit
	-[ -n "$(DESTDIR)" ] || update-desktop-database -q $(DATADIR)/applications 2>/dev/null
	-[ -n "$(DESTDIR)" ] || makewhatis $(MANDIR) 2>/dev/null || mandb -q 2>/dev/null

# the manual pages, as mandoc reads them (its warnings and errors)
man-lint:
	mandoc -Tlint -Wwarning docs/man/cedit.1 docs/man/pt_BR/cedit.1

clean:
	rm -rf $(BUILD)

.PHONY: all cedit xcedit icon install uninstall man-lint clean
