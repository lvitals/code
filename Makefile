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
#   make check-langs  the languages listed here are those of src/app/langs
#   make uninstall
#   make clean
#
# A POSIX make (IEEE Std 1003.1): the make of Linux (GNU), of macOS, of
# FreeBSD, OpenBSD and NetBSD alike.  Any C99 compiler: make CC=clang,
# make CC=alefc ...  The objects are made next to their sources (.c.o);
# the programs in build/.
#
# The sources:
#   src/core      the text and what is done to it (no screen)
#   src/tui       the screen buffer and the widgets (button, list, ...)
#   src/app       the editor: menus, text area, status bar, dialogs
#   src/app/langs the languages, a file each (listed in langs/all.h, and
#                 here: LANGS)
#   src/platform  the files (posix/files.c), the terminal (posix/term.c),
#                 the window (glfw/gui.c, with stb_truetype.h), the colours,
#                 the icon

.POSIX:
.SUFFIXES:
.SUFFIXES: .c .o

CC       = cc
CFLAGS   = -O2
# the search of the workspace's text: a thread on each core (POSIX)
LDFLAGS  = -pthread
WFLAGS   = -std=c99 -Wall -Wextra -pedantic
PREFIX   = /usr/local
BINDIR   = $(PREFIX)/bin
DATADIR  = $(PREFIX)/share
MANDIR   = $(DATADIR)/man
# the window's libraries (GLFW, OpenGL), asked when they are needed
GUI_CFLAGS = `sh tools/gui-flags --cflags`
GUI_LIBS   = `sh tools/gui-flags --libs`

# the editor, then its two faces: cedit (a terminal) and xcedit (a window)
CORE = src/core/mem.o src/core/buf.o src/core/enc.o src/core/regex.o \
	src/tui/screen.o src/tui/ui.o src/tui/uilist.o \
	src/app/main.o src/app/start.o src/app/editor.o src/app/menu.o \
	src/app/menus.o src/app/status.o src/app/complete.o src/app/finder.o \
	src/app/walk.o src/app/fuzzy.o src/app/scan.o src/app/jumps.o \
	src/app/filter.o src/app/lang.o src/app/search.o src/app/dialogs.o \
	src/app/picker.o src/app/pickfile.o src/app/paths.o src/app/settings.o \
	src/app/syntax.o src/app/langs.o \
	src/platform/posix/files.o src/platform/posix/run.o \
	src/platform/posix/thread.o src/platform/colours.o
# the languages: src/app/langs/NAME.c, each in langs/all.h too (a new one:
# a line there and one here; make check-langs tells of one forgotten)
LANGS = src/app/langs/actionscript.o src/app/langs/ada.o \
	src/app/langs/alef.o src/app/langs/antlr.o src/app/langs/apdl.o \
	src/app/langs/apl.o src/app/langs/applescript.o \
	src/app/langs/asm.o src/app/langs/asp.o src/app/langs/autohotkey.o \
	src/app/langs/autoit.o src/app/langs/awk.o src/app/langs/bash.o \
	src/app/langs/batch.o src/app/langs/bibtex.o src/app/langs/boo.o \
	src/app/langs/c.o src/app/langs/caml.o src/app/langs/chuck.o \
	src/app/langs/clojure.o src/app/langs/cmake.o \
	src/app/langs/cobol.o src/app/langs/coffeescript.o \
	src/app/langs/container.o src/app/langs/context.o \
	src/app/langs/cpp.o src/app/langs/crystal.o src/app/langs/csharp.o \
	src/app/langs/css.o src/app/langs/cuda.o src/app/langs/d.o \
	src/app/langs/dart.o src/app/langs/desktop.o src/app/langs/diff.o \
	src/app/langs/django.o src/app/langs/dockerfile.o \
	src/app/langs/dot.o src/app/langs/dsv.o src/app/langs/eiffel.o \
	src/app/langs/elixir.o src/app/langs/elm.o src/app/langs/erlang.o \
	src/app/langs/factor.o src/app/langs/fantom.o \
	src/app/langs/faust.o src/app/langs/fennel.o src/app/langs/fish.o \
	src/app/langs/forth.o src/app/langs/fortran.o \
	src/app/langs/fsharp.o src/app/langs/fstab.o src/app/langs/gap.o \
	src/app/langs/gemini.o src/app/langs/gettext.o \
	src/app/langs/gherkin.o src/app/langs/git_rebase.o \
	src/app/langs/gleam.o src/app/langs/glsl.o src/app/langs/gnuplot.o \
	src/app/langs/go.o src/app/langs/groovy.o src/app/langs/gtkrc.o \
	src/app/langs/hare.o src/app/langs/haskell.o src/app/langs/html.o \
	src/app/langs/icon.o src/app/langs/idl.o src/app/langs/inform.o \
	src/app/langs/ini.o src/app/langs/io_lang.o src/app/langs/janet.o \
	src/app/langs/java.o src/app/langs/javascript.o src/app/langs/jq.o \
	src/app/langs/json.o src/app/langs/jsp.o src/app/langs/julia.o \
	src/app/langs/latex.o src/app/langs/ledger.o src/app/langs/less.o \
	src/app/langs/lilypond.o src/app/langs/lisp.o \
	src/app/langs/litcoffee.o src/app/langs/logtalk.o \
	src/app/langs/lua.o src/app/langs/makefile.o src/app/langs/man.o \
	src/app/langs/markdown.o src/app/langs/matlab.o \
	src/app/langs/mediawiki.o src/app/langs/meson.o \
	src/app/langs/moonscript.o src/app/langs/myrddin.o \
	src/app/langs/nemerle.o src/app/langs/networkd.o \
	src/app/langs/nim.o src/app/langs/nix.o src/app/langs/nsis.o \
	src/app/langs/objeck.o src/app/langs/objective_c.o \
	src/app/langs/org.o src/app/langs/output.o src/app/langs/pascal.o \
	src/app/langs/perl.o src/app/langs/php.o src/app/langs/pico8.o \
	src/app/langs/pike.o src/app/langs/pkgbuild.o src/app/langs/pony.o \
	src/app/langs/powershell.o src/app/langs/prolog.o \
	src/app/langs/props.o src/app/langs/protobuf.o src/app/langs/ps.o \
	src/app/langs/pure.o src/app/langs/python.o src/app/langs/r.o \
	src/app/langs/rails.o src/app/langs/rc.o src/app/langs/reason.o \
	src/app/langs/rebol.o src/app/langs/rest.o src/app/langs/rexx.o \
	src/app/langs/rhtml.o src/app/langs/routeros.o \
	src/app/langs/rpmspec.o src/app/langs/ruby.o src/app/langs/rust.o \
	src/app/langs/sass.o src/app/langs/scala.o src/app/langs/scheme.o \
	src/app/langs/smalltalk.o src/app/langs/sml.o \
	src/app/langs/snobol4.o src/app/langs/spin.o src/app/langs/sql.o \
	src/app/langs/strace.o src/app/langs/systemd.o \
	src/app/langs/taskpaper.o src/app/langs/tcl.o src/app/langs/tex.o \
	src/app/langs/texinfo.o src/app/langs/todotxt.o \
	src/app/langs/toml.o src/app/langs/troff.o \
	src/app/langs/txt2tags.o src/app/langs/typescript.o \
	src/app/langs/vala.o src/app/langs/vb.o src/app/langs/vcard.o \
	src/app/langs/verilog.o src/app/langs/vhdl.o src/app/langs/wsf.o \
	src/app/langs/xml.o src/app/langs/xs.o src/app/langs/xtend.o \
	src/app/langs/yaml.o src/app/langs/zig.o
TERM   = src/platform/posix/term.o
WINDOW = src/platform/glfw/gui.o
HDRS = src/app/app.h src/app/finder.h src/app/lang_def.h src/app/langs/all.h \
	src/app/syntax.h src/core/buf.h src/core/enc.h src/core/mem.h \
	src/core/regex.h src/platform/colours.h src/platform/icon.h \
	src/platform/platform.h src/tui/screen.h src/tui/ui.h

all: build/cedit build/xcedit

cedit: build/cedit
xcedit: build/xcedit

.c.o:
	$(CC) $(WFLAGS) $(CFLAGS) -c -o $@ $<

$(CORE) $(LANGS) $(TERM) $(WINDOW): $(HDRS)

build/cedit: $(CORE) $(LANGS) $(TERM)
	mkdir -p build
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(CORE) $(LANGS) $(TERM)

# the window: its file with the flags of GLFW; stb_truetype's code (not
# ours: no warnings); the font, a C array made by tools/bin2c
src/platform/glfw/gui.o: src/platform/glfw/gui.c src/platform/glfw/stb_truetype.h
	$(CC) $(WFLAGS) $(CFLAGS) $(GUI_CFLAGS) -c -o $@ src/platform/glfw/gui.c

src/platform/glfw/stb.o: src/platform/glfw/stb.c src/platform/glfw/stb_truetype.h
	$(CC) -std=c99 -O2 -w -c -o $@ src/platform/glfw/stb.c

build/bin2c: tools/bin2c.c
	mkdir -p build
	$(CC) $(WFLAGS) $(CFLAGS) -o $@ tools/bin2c.c

build/font.c: build/bin2c data/fonts/FantasqueSansMono-Regular.ttf
	build/bin2c font_regular data/fonts/FantasqueSansMono-Regular.ttf > $@

build/font.o: build/font.c
	$(CC) -std=c99 -O2 -c -o $@ build/font.c

build/xcedit: $(CORE) $(LANGS) $(WINDOW) src/platform/glfw/stb.o build/font.o
	mkdir -p build
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(CORE) $(LANGS) $(WINDOW) \
	    src/platform/glfw/stb.o build/font.o $(GUI_LIBS)

# the icon, from its data (as tools/mascot-svg.c of inkaur): in the
# sources, made again when its data or its tool change
icon: data/xcedit.svg

build/icon-svg: tools/icon-svg.c src/platform/icon.h
	mkdir -p build
	$(CC) $(WFLAGS) $(CFLAGS) -o $@ tools/icon-svg.c

data/xcedit.svg: build/icon-svg
	build/icon-svg > $@.tmp && mv $@.tmp $@

# each language of src/app/langs is in LANGS, and each of LANGS is there
check-langs:
	@ok=1; \
	for f in src/app/langs/*.c; do \
		o="$${f%.c}.o"; \
		case " `echo $(LANGS)` " in *" $$o "*) ;; *) echo "not in LANGS: $$o"; ok=0;; esac; \
	done; \
	for o in $(LANGS); do \
		[ -f "$${o%.o}.c" ] || { echo "no source: $${o%.o}.c"; ok=0; }; \
	done; \
	[ $$ok = 1 ] && echo "LANGS: as src/app/langs"

# the programs; for xcedit its entry in the menus of the system and its
# icon (the caches of the desktop brought up to date, when there)
install: all data/xcedit.svg
	mkdir -p $(DESTDIR)$(BINDIR) $(DESTDIR)$(DATADIR)/applications \
	    $(DESTDIR)$(MANDIR)/man1 $(DESTDIR)$(MANDIR)/pt_BR/man1 \
	    $(DESTDIR)$(DATADIR)/icons/hicolor/scalable/apps $(DESTDIR)$(DATADIR)/licenses/cedit
	install -m 755 build/cedit build/xcedit $(DESTDIR)$(BINDIR)/
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
	rm -f $(CORE) $(LANGS) $(TERM) $(WINDOW) src/platform/glfw/stb.o
	rm -rf build

.PHONY: all cedit xcedit icon install uninstall man-lint check-langs clean
