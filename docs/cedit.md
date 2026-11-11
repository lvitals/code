# Code Editor: user manual

*Leia em português: [cedit.pt_BR.md](cedit.pt_BR.md).  The manual page
is `man cedit` (in Portuguese: `man -M /usr/local/share/man/pt_BR cedit`,
or `man -L pt_BR cedit` with man-db; sources:
[man/cedit.1](man/cedit.1) and [man/pt_BR/cedit.1](man/pt_BR/cedit.1)).*

Code Editor is a text editor written in portable C99.  It has a menu
bar, mouse support and the usual keyboard shortcuts.  It comes as two
programs built from the same sources:

- **`cedit`** runs in a terminal;
- **`xcedit`** runs in a window of its own (GLFW and OpenGL), with the
  text drawn from a font built into the program (FantasqueSansMono) or
  one chosen in the settings.

Both have the same menus, keys, dialogs and settings.  Everything
described here applies to both unless it says otherwise.

## Contents

1. [Building and installing](#1-building-and-installing)
2. [Starting the editor](#2-starting-the-editor)
3. [The screen](#3-the-screen)
4. [Moving and selecting](#4-moving-and-selecting)
5. [Editing](#5-editing)
6. [Find and Replace](#6-find-and-replace)
7. [Going places](#7-going-places)
8. [Files: Open, Save As and the dialogs](#8-files-open-save-as-and-the-dialogs)
9. [The project: Open Project Folder, Open File in Project, Search in Project](#9-the-project)
10. [Encodings and line endings](#10-encodings-and-line-endings)
11. [The View menu](#11-the-view-menu)
12. [Syntax highlighting and languages](#12-syntax-highlighting-and-languages)
13. [Large files](#13-large-files)
14. [The window (xcedit)](#14-the-window-xcedit)
15. [Settings](#15-settings)
16. [Keyboard reference](#16-keyboard-reference)
17. [Files and environment](#17-files-and-environment)

---

## 1. Building and installing

Requirements:

- `cedit`: a C99 compiler and a POSIX system (termios, dirent, poll,
  POSIX threads).
- `xcedit`: also GLFW 3, OpenGL and the stb headers
  (`/usr/include/stb`).

```
make                 # build/cedit and build/xcedit
make cedit           # the terminal editor alone (no GLFW needed)
make xcedit          # the window editor
make CC=clang        # any other C99 compiler (CC=alefc works too)
make clean
```

Installing:

```
make install                    # into /usr/local (as root)
make install PREFIX=~/.local    # for one user
make install DESTDIR=/tmp/pkg   # staged, for a package
make uninstall
```

This installs `cedit` and `xcedit`, their manual page (`cedit(1)`, with
`xcedit(1)` as a link to it, in English and Portuguese), the menu entry of xcedit
(`share/applications/xcedit.desktop`, named "Code Editor") and its icon
(`share/icons/hicolor/scalable/apps/xcedit.svg`).

## 2. Starting the editor

```
cedit  [OPTIONS] [FILE|DIR]...
xcedit [OPTIONS] [FILE|DIR]...
cedit  --lex FILE [LANGUAGE]
```

Each FILE is opened in a document of its own.  A FILE that does not
exist is opened as a new, empty document with that name; it is created
when it is saved.  With no FILE an empty, unnamed document is opened.

A DIR is not opened as a file: it becomes the **project folder** (see
[section 9](#9-the-project)).  `cedit .` opens the editor with the current
folder as the project; it does not show a dialog.

Options:

| Option | Meaning |
|---|---|
| `-f`, `--foreground` | xcedit: keep the terminal until the window closes.  Use it when xcedit is `EDITOR` (for example for `git commit`, which waits for the editor). |
| `-g`, `--goto FILE:LINE[:COLUMN]` | Open FILE with the cursor at LINE (and COLUMN).  Lines and columns count from 1. |
| `-n`, `--no-line-numbers` | Open the files without line numbers.  They are then not counted at all, which helps with very large files. |
| `-h`, `--help` | Print the usage and exit. |
| `-v`, `--version` | Print the version and exit. |
| `--lex FILE [LANGUAGE]` | Print each line of FILE with, under it, the highlighting class of each character, then exit.  For people writing a language definition; see `docs/LANGUAGES.md`. |
| `--` | The arguments after it are file names, even if they start with `-`. |

The `--lex` letters are: `.` text, `K` keyword, `T` type or function,
`S` string, `C` comment, `N` number or constant, `P` preprocessor,
`D` deleted line (diffs).

**xcedit and the terminal.**  Started from a terminal, xcedit gives the
prompt back as soon as its window is open.  The window then runs apart
from the terminal, and closing the terminal does not close it.  If the
window cannot be opened, the reason is printed in the terminal.  With
`-f` the terminal waits until the window is closed.

Examples:

```
cedit notes.txt
cedit -g src/main.c:120:5
xcedit ~/projects/site          # that folder is the project
EDITOR="xcedit -f" git commit
cedit --lex test.lua lua
```

## 3. The screen

From top to bottom:

- **The menu bar**: File, Edit, View, Help.  Alt and the underlined
  letter opens a menu, as does F10.  In a menu, the arrows move, Enter
  or the item's letter chooses, and Esc closes.  Each item shows its
  shortcut.  Items that do not apply (for example Save with no
  document) are greyed.
- **The text area**: the line numbers in the margin, the text, the
  vertical scrollbar at the right and, without Word Wrap, a horizontal
  scrollbar under the text.
- **The Find bar**, when it is open (section 6).
- **The status bar**.

### The status bar

From left to right, as far as the width allows:

| Item | Meaning | A click |
|---|---|---|
| Language | The language of the document, for highlighting | |
| LF / CRLF | The line endings | Switch between them |
| Encoding | UTF-8, UTF-16 LE, Windows-1252, … | Opens the encoding list (section 10) |
| Indentation | Tabs, or spaces and how many, as guessed from the file | Change it |
| Line:Column | The cursor's position | |
| OVR | Overtype mode is on (Insert key) | Turn it off |
| `*` | The document has unsaved changes | |
| Project | The name of the project folder (at the right) | Opens Open Project Folder |
| File name | The document's name | Go to File (switch documents) |

On a narrow screen the items that do not fit are dropped by order of
importance, and shorter labels are used.  View > Focus Statusbar moves
the keyboard focus to the bar, so its buttons can be used without a
mouse.

### Dialogs

Every dialog works the same way.  Tab and Shift+Tab move between its
items (fields, lists, check boxes, buttons).  Enter does the default
action, Esc closes it, and the mouse works on every item.

## 4. Moving and selecting

| Key | Action |
|---|---|
| Arrows | Move one character or line |
| Ctrl+Left / Ctrl+Right | Move one word |
| Home | To the first non-blank character of the line; pressed again, to the start of the line |
| End | To the end of the line |
| PgUp / PgDn | One screen up or down |
| Ctrl+Home / Ctrl+End | To the start or end of the document |
| Ctrl+Up / Ctrl+Down | Scroll the view one line without moving the cursor |
| Shift + any of the above | Extend the selection |
| Ctrl+A | Select all |
| Ctrl+L | Select the current line (again: add the next one) |
| Shift+Alt+Right | Expand Selection: the word, inside the quotes, the quotes, inside the brackets, the brackets, the line, everything |
| Shift+Alt+Left | Shrink Selection: back one step |

The mouse:

- a click places the cursor; a drag selects, and scrolls when it passes
  the edge of the text area;
- a double click selects a word, a triple click the line;
- Shift+click extends the selection;
- the wheel scrolls; Shift+wheel, or a sideways wheel, scrolls the
  columns;
- the middle button pastes;
- the right button opens a context menu (Cut, Copy, Paste, Select All,
  Find);
- on a scrollbar, the arrows scroll, a click on the track jumps, and
  the thumb can be dragged.

## 5. Editing

### Typing

- Typed text is inserted, or, in overtype mode (Insert key, shown as
  OVR), replaces what is there.
- Enter keeps the indentation of the line (Auto Indent, on by default).
- Backspace in the indentation removes a whole level when "Backspace
  Removes a Whole Indent" is on; otherwise one character.
- Tab inserts a tab or spaces, as the document is indented.  With a
  selection over several lines, Tab indents them and Shift+Tab removes
  one level.
- Ctrl+Backspace and Ctrl+Delete delete a word.
- Ctrl+Shift+H deletes to the start of the line, Ctrl+Shift+G to its
  end.
- Alt+Up and Alt+Down move the current line (or the selected lines) up
  or down.

### Clipboard

| Key | Action |
|---|---|
| Ctrl+X, Shift+Delete | Cut (without a selection: the whole line) |
| Ctrl+C, Ctrl+Insert | Copy (without a selection: the whole line) |
| Ctrl+V, Shift+Insert | Paste |

In a terminal the editor copies with OSC 52, so the copied text also
reaches the system clipboard when the terminal supports it, and it
accepts bracketed paste.  In the window the system clipboard is used
directly.

### Undo and redo

Ctrl+Z undoes and Ctrl+Y (or Ctrl+Shift+Z) redoes.  Characters typed in a
row are undone together.

### Multiple cursors

- Alt+click adds a cursor, or removes one that is there.
- Ctrl+D selects the word at the cursor; pressed again, it adds a
  cursor at the next occurrence of the selection (Edit > Add Next
  Occurrence).
- Shift+Alt+Up and Shift+Alt+Down add a cursor on the line above or
  below.

Typing, Backspace, Delete, Enter, Tab, the arrows, Home, End, cut, copy
and paste act at every cursor.  Copying with N cursors and pasting with
N cursors puts one line at each.  Esc goes back to one cursor.

### Complete Word (Ctrl+Space)

Completes the word before the cursor from the words in the document,
the nearest first.  If there is only one candidate it is inserted at
once.  Otherwise a list opens at the cursor: typing narrows it, Up and
Down choose, Enter or Tab inserts, Esc closes.

### Filter Through Command (Alt+|)

Asks for a shell command and runs it with `sh -c`.  The selection is
given to it on standard input and replaced by what it writes.  Without a
selection, its output is inserted at the cursor.  Examples: `sort`,
`fmt -w 72`, `jq .`, `date`.  If the command fails nothing is changed and
its error output is shown.  One undo takes the whole change back.

### The mark

Alt+M sets a mark at the cursor; Alt+J jumps back to it.

## 6. Find and Replace

Ctrl+F opens the Find bar and Ctrl+R opens Find and Replace.  With text
selected (within one line), the selection becomes the text to find.

- **Find**: the search moves as you type.  Enter, or F3, finds the next
  occurrence.  The field changes colour when there is no match.
- **Replace**: Enter in the Replace field replaces the current match
  and goes to the next one.  Replace All (button, or Ctrl+Alt+Enter in
  the field) replaces every match; one undo takes it all back.
- **Match Case**, **Whole Word** and **Use Regex** are check boxes.
  They are remembered in the settings.
- **Close**, or Esc, closes the bar.

Without Match Case, letters match in both upper and lower case.

### Regular expressions

The editor has its own regular expression engine (`src/core/regex.c`).

| Syntax | Matches |
|---|---|
| `.` | any character except a newline |
| `[abc]`, `[a-z]`, `[^…]` | a character in (or not in) the set |
| `\d` `\w` `\s` | a digit, a word character, a blank; `\D` `\W` `\S` the opposite |
| `\b` `\B` | a word boundary, not a word boundary |
| `^` `$` | the start and end of a line (`$` also matches before a CR LF) |
| `*` `+` `?` | 0 or more, 1 or more, 0 or 1 |
| `{m}` `{m,}` `{m,n}` | exactly m, at least m, from m to n |
| `*?` `+?` `??` `{…}?` | the same, as few as possible (lazy) |
| `( )` | a group, captured (1 to 9) |
| `(?: )` | a group, not captured |
| `a\|b` | either |
| `\n` `\t` `\r` `\.` … | the character itself |

In the replacement, `$1` to `$9` (or `\1` to `\9`) insert the groups and
`$0` the whole match.

## 7. Going places

| Key | Action |
|---|---|
| Ctrl+G | Go to Line:Column (`120`, `120:5`) |
| Ctrl+P | Go to File: switch between the open documents |
| Ctrl+] | Go to the matching bracket, or the other end of a block (`if` … `end`, `#if` … `#endif`) |
| Alt+Left | Go Back: to where the cursor jumped from |
| Alt+Right | Go Forward |
| Alt+M / Alt+J | Set the mark / jump to it |
| F3 | Find next |

Go Back and Go Forward remember the jumps: Go to Line, a search,
Ctrl+Home and Ctrl+End, the matching bracket, the mark, a click far
away, and switching documents.  The positions follow the edits.

## 8. Files: Open, Save As and the dialogs

| Key | Menu | Action |
|---|---|---|
| Ctrl+N | File > New File | A new, empty document |
| Ctrl+O | File > Open File… | Browse for a file to open |
| Ctrl+S | File > Save | Save (asks for a name if the document has none) |
| Ctrl+Shift+S | File > Save As… | Save under another name |
| Ctrl+W | File > Close File | Close the document |
| Ctrl+Q | File > Exit | Leave the editor |

Close and Exit ask before discarding unsaved changes (Save, Don't Save,
Cancel).

### Open File (Ctrl+O)

The dialog shows a folder (at first the project folder), a **Search**
field and a list: `..`, then the folders, then the files.

- Typing in Search narrows the list with a fuzzy match: the letters
  typed must appear in order, not necessarily together.
- Up and Down (or PgUp, PgDn, Ctrl+P, Ctrl+N) choose an entry, even
  while the focus is in Search.
- Enter on a folder goes into it (`..` goes up); Enter on a file opens
  it.
- Backspace with an empty Search, or Alt+Up, goes to the parent folder.
- Typing `name/` goes into that folder, `~/` goes home and `/` goes to
  the root; a path can be typed as well.
- If nothing matches, the dialog says so and Enter opens a new file with
  the name typed.
- A double click opens; Tab moves between Search, the list and the
  Hidden check box.
- Alt+H (Ctrl+H in xcedit), or the **Hidden** check box, shows or hides
  hidden files (names starting with `.`).  The choice is kept in the
  settings.

### Save As

Save As has a **File name** field instead of a search, because the name
must be typed.  Below it, the names in the folder that start with what
is typed are listed.  Saving over an existing file asks first.

### How files are saved

A save writes a new file next to the old one and renames it over it,
keeping its permissions: the old file stays whole until the new one is
complete.  The text is written in the document's encoding and
line endings (section 10).

If another program changes a file that is open, the editor tells you
(once for each change) and, when the document has no unsaved changes,
asks whether to reload it.

## 9. The project

The **project folder** limits Open File in Project and Search in Project
and is where Open and Save As start.  It is shown on the status bar.

It is:

1. the folder given on the command line (`cedit DIR`), else
2. the folder chosen with File > Open Project Folder, else
3. the folder the editor was started in (from the system menu this is
   usually your home folder, so choose a project with Alt+O).

The **project** that the searches cover is the Git repository that
contains the project folder (the first folder above it with a `.git`),
or else the project folder itself.  Its name is shown on the frame of
the search dialogs.

### Open Project Folder (Alt+O)

Also opened by a click on the project in the status bar.  It works like
Open File, but lists folders only.  The first entry, `./ (open here)`,
stands for the folder being shown.

- Typing narrows the list; Up and Down choose.
- Enter on a folder goes into it, and on `..` goes up.
- Enter on `./` makes the folder shown the project.
- Alt+Enter makes the chosen folder the project without going into it.
- Backspace with an empty Search, or Alt+Up, goes up; `name/`, `~/`, `/`
  and typed paths navigate.
- The **Open This Folder** button makes the folder shown the project.

### Open File in Project (Alt+P)

A list of the project's files on the left and a
preview of the selected file on the right (highlighted; a binary file
is reported as such).

- Typing filters the list with a fuzzy match on the path: the letters
  in order, with blanks separating words that must all match.  Matches
  at the start of a name, letters together, and matches in the file name
  rank higher.  An upper-case letter makes the match case-sensitive.
- The files are found while the editor is idle, a slice at a time, so
  the list fills as they come in ("(running)" is shown meanwhile).
- Left out: hidden files and folders (`.git`, `.cache`, …); what the
  `.gitignore` and `.ignore` files of each folder, and
  `.git/info/exclude`, exclude; links to folders.

### Search in Project (Alt+/)

Searches the text of every file in the project
for a regular expression (or for plain text, if what is typed has no
special characters).  Each match is listed as `path:line`, and the
preview shows the line in the middle.

How it searches:

- On one thread per processor core (up to 8), so the editor stays
  responsive.  A new search cancels the old one at once.
- Each file is read a block at a time (256 KB; the first block 8 KB).
  Files are never loaded whole, never mapped and never written, so a
  file of any size can be searched.
- Each file's encoding is detected as when it is opened (a large file
  from its first block).  UTF-8 is searched as it is; other encodings
  are decoded a block at a time.  CR LF is kept, and `$` matches before
  it.
- A file is treated as binary, and skipped, from its first NUL byte.
- A regular expression that would take too long on one line (such as
  `.*` over megabytes) skips that line rather than stalling.

### Keys in both search dialogs

| Key | Action |
|---|---|
| Up, Down, PgUp, PgDn, Ctrl+P, Ctrl+N, the wheel, a click | Choose an entry |
| Enter, a double click | Open the file (at the matching line) |
| The wheel over the preview | Scroll the preview |
| Esc | Close |

No external program (grep, rg, find, git) is run: all of it is the
editor's own C99 code.

## 10. Encodings and line endings

The text is UTF-8 inside the editor.  A file in another encoding is
decoded when it is read and encoded again when it is saved.  Supported
encodings: UTF-8, UTF-8 with BOM, UTF-16 LE, UTF-16 BE, Windows-1252,
ISO-8859-1 and ISO-8859-15.

How the encoding is detected when a file is opened:

1. a BOM means UTF-8, UTF-16 LE or UTF-16 BE;
2. otherwise, UTF-16 without a BOM is recognised by its zero bytes, and
   it is saved without a BOM;
3. otherwise UTF-8, if the bytes are valid UTF-8 (for a file of 32 MB or
   more, its first 64 KB are checked);
4. otherwise Windows-1252, or ISO-8859-1 when no byte is in the range
   80h–9Fh.  ISO-8859-15 is never guessed; it can only be chosen.

A file opened and saved without changes is written back byte for byte
in any encoding.

The encoding is shown in the status bar.  Its button, or File >
Encoding…, opens a list with two actions:

- **Save with**: the next save writes that encoding;
- **Reopen with**: reads the file again in that encoding (only when
  there are no unsaved changes).

If the text has a character that an 8-bit encoding cannot represent,
the save writes nothing: the character is selected and its line is
reported.

The line endings (LF or CR LF) are detected per file and shown in the
status bar; a click on them switches.

## 11. The View menu

Each option is per document, and changing it also stores it in the
settings for the next documents and the next start.

| Item | Key | Default | Meaning |
|---|---|---|---|
| Go to File… | Ctrl+P | | Switch between open documents |
| Go to Line:Column… | Ctrl+G | | |
| Go to Matching Bracket | Ctrl+] | | |
| Go Back / Go Forward | Alt+Left / Alt+Right | | |
| Focus Statusbar | | | Keyboard focus to the status bar |
| Word Wrap | Alt+Z | off | Wrap long lines at the window's edge |
| Show Tabs | | off | Draw each tab as a line ending in an arrow |
| Show Indent Guides | | on | A thin line at each indentation level |
| Show Line Ends | | on | Mark the end of each line |
| Show End of File | | on | Mark the end of the text |
| Line Numbers | Alt+N | on | Numbers in the margin |
| Relative Line Numbers | Alt+R | off | The distance to the cursor's line |
| Syntax Highlighting | | on | |
| Highlight Current Line | | on | |
| Highlight Matching Brackets | | on | The bracket or block keyword at the cursor and its match |
| Zoom In / Out / Reset | Ctrl++ / Ctrl+- / Ctrl+0 | | xcedit only |

The Edit menu has two more options: **Auto Indent** (on) and
**Backspace Removes a Whole Indent** (on).

With line numbers turned off, lines are not counted at all; the status
bar then shows only the column, and the scrollbar shows the position in
bytes.

## 12. Syntax highlighting and languages

158 languages are highlighted, from C, Python and JavaScript to COBOL,
Ada, Fortran, Lisp and Alef.  The language of a document is chosen
from, in order: its file name, its suffix, the end of its path, its
`#!` line, and its first lines.  The status bar shows it.

Each language is a small C file in `src/app/langs/` that lists its
suffixes, keywords, types, comments, strings and blocks.  To add one,
copy `docs/lang_template.c`, add a line to `src/app/langs/all.h`, and
check the result with `cedit --lex FILE`.  The full guide is
[LANGUAGES.md](LANGUAGES.md).

There is no language server.

## 13. Large files

The editor opens files of any size quickly:

- A file of 32 MB or more is mapped into memory rather than read, so
  it opens at once and only the parts shown are read from the disk.  A
  file with CR LF line endings is converted, which copies it.
- The text is a piece table in a balanced tree: an edit, an undo and a
  lookup take O(log n) time, and nothing copies the text.
- Line numbers are counted lazily, in the background while the editor
  waits for keys.  Until the count reaches the top of the screen, the
  margin shows dimmed distances to the cursor and the status bar the
  progress (`…37%:1`).  Go to Line counts what it needs at once.
- Very long lines (megabytes on a single line) move, draw and scroll
  smoothly.
- Highlighting looks at most 64 KB before the screen.
- A mapped file is never written into: a save always writes a new file.
  If another program truncates the file while it is mapped, the lost
  pages read as zeros and the editor says so instead of crashing.

For the fastest opening of a huge file, use `-n` (no line numbers).

## 14. The window (xcedit)

- **Zoom**: Ctrl and + or - (or Ctrl and the wheel); Ctrl+0 resets it.
  The size is kept in the settings (`font_size`).
- **Font**: FantasqueSansMono is built in.  Another can be set in the
  settings (`font`): either a file (`"~/fonts/Iosevka.ttf"`; .ttf, .otf
  or .ttc) or a family name (`"JetBrains Mono"`, `"JetBrains Mono:Bold"`).
  Families are looked up by the editor itself, by reading the name
  tables of the fonts in `~/.local/share/fonts`, `~/.fonts`,
  `$XDG_DATA_HOME/fonts` and the `fonts` folder of each of
  `$XDG_DATA_DIRS`.  The cell is as wide as the font's `M`, so a
  monospaced font is expected.  If the font cannot be found or read the
  editor says so and keeps the current one.
- **Clipboard**: Ctrl+V and Shift+Insert paste what other programs
  copied; what the editor copies goes to the system clipboard.
- The keys and the mouse behave as in the terminal.  In dialogs, Ctrl+H
  toggles hidden files (Alt+H in a terminal).
- **The terminal**: see [section 2](#2-starting-the-editor) (`-f`).

## 15. Settings

The settings are stored in `$XDG_CONFIG_HOME/cedit/settings.json` (by
default `~/.config/cedit/settings.json`), shared by cedit and xcedit.
File > Preferences opens that file in the editor, creating it with the
current values if it does not exist.  When it is saved it is checked
and applied at once to the open documents.

Turning an option on or off in the menus or the Find bar writes the
file too.  Every write is verified: the JSON is parsed and every value
read back, before and after saving.  A file that is not valid JSON is
not used: the editor reports "Invalid Settings: Invalid JSON", uses the
defaults, and keeps a copy as `settings.json.invalid` before writing
over it.

| Name | Type | Default | Meaning |
|---|---|---|---|
| `word_wrap` | bool | false | View > Word Wrap |
| `auto_indent` | bool | true | Edit > Auto Indent |
| `show_tabs` | bool | false | View > Show Tabs |
| `show_line_ends` | bool | true | View > Show Line Ends |
| `show_end_of_file` | bool | true | View > Show End of File |
| `line_numbers` | bool | true | View > Line Numbers |
| `relative_line_numbers` | bool | false | View > Relative Line Numbers |
| `highlight_current_line` | bool | true | View > Highlight Current Line |
| `syntax_highlighting` | bool | true | View > Syntax Highlighting |
| `match_brackets` | bool | true | View > Highlight Matching Brackets |
| `backspace_unindents` | bool | true | Edit > Backspace Removes a Whole Indent |
| `indent_guides` | bool | true | View > Show Indent Guides |
| `find_match_case` | bool | false | Find: Match Case |
| `find_whole_word` | bool | false | Find: Whole Word |
| `find_use_regex` | bool | false | Find: Use Regex |
| `show_hidden_files` | bool | false | Open and Save As: show hidden files |
| `expand_tabs` | bool | true | Tab inserts spaces, in a text not yet indented |
| `tab_width` | 1–8 | 4 | Tab width, and the indentation of a text not yet indented |
| `ruler_column` | 0–9999 | 0 | Draw a ruler after this column (80: after the 80th; 0: none) |
| `font_size` | 0–200 | 0 | xcedit's text size in pixels (0: the default) |
| `font` | string | `""` | xcedit's font: a file or a family; `""` is the built-in one |

A file that already has an indentation keeps it: `expand_tabs` and
`tab_width` only decide how a new or unindented text is indented.

Example:

```json
{
  "word_wrap": false,
  "line_numbers": true,
  "relative_line_numbers": true,
  "tab_width": 8,
  "expand_tabs": false,
  "ruler_column": 80,
  "font_size": 18,
  "font": "JetBrains Mono"
}
```

## 16. Keyboard reference

In xcedit, Alt is shown as Alt; in some terminals Alt+letter must be
enabled ("meta sends escape").

### Files

| Key | Action |
|---|---|
| Ctrl+N | New File |
| Ctrl+O | Open File |
| Alt+O | Open Project Folder |
| Alt+P | Open File in Project |
| Ctrl+S | Save |
| Ctrl+Shift+S | Save As |
| Ctrl+W | Close File |
| Ctrl+Q | Exit |

### Editing

| Key | Action |
|---|---|
| Ctrl+Z / Ctrl+Y, Ctrl+Shift+Z | Undo / Redo |
| Ctrl+X / Ctrl+C / Ctrl+V | Cut / Copy / Paste |
| Shift+Delete / Ctrl+Insert / Shift+Insert | Cut / Copy / Paste |
| Ctrl+A | Select All |
| Ctrl+L | Select line |
| Shift+Alt+Right / Shift+Alt+Left | Expand / Shrink Selection |
| Ctrl+D | Add Next Occurrence |
| Shift+Alt+Up / Shift+Alt+Down | Add Cursor Above / Below |
| Alt+click | Add or remove a cursor |
| Esc | Back to one cursor |
| Ctrl+Space | Complete Word |
| Alt+\| | Filter Through Command |
| Ctrl+Backspace / Ctrl+Delete | Delete a word |
| Ctrl+Shift+H / Ctrl+Shift+G | Delete to line start / end |
| Alt+Up / Alt+Down | Move lines up / down |
| Tab / Shift+Tab | Indent / unindent (selected lines) |
| Insert | Overtype on or off |
| Alt+M / Alt+J | Set Mark / Jump to Mark |

### Search

| Key | Action |
|---|---|
| Ctrl+F | Find |
| Ctrl+R | Replace |
| F3, Enter in Find | Find Next |
| Enter in Replace | Replace and find next |
| Ctrl+Alt+Enter in Replace | Replace All |
| Alt+/ | Search in Project |
| Esc | Close the bar |

### Navigation and view

| Key | Action |
|---|---|
| Ctrl+G | Go to Line:Column |
| Ctrl+P | Go to File (open documents) |
| Ctrl+] | Go to Matching Bracket |
| Alt+Left / Alt+Right | Go Back / Go Forward |
| Ctrl+Up / Ctrl+Down | Scroll one line |
| Alt+Z | Word Wrap |
| Alt+N | Line Numbers |
| Alt+R | Relative Line Numbers |
| Ctrl++ / Ctrl+- / Ctrl+0 | Zoom In / Out / Reset (xcedit) |
| F10, Alt+letter | Open a menu |

## 17. Files and environment

| Path or variable | Use |
|---|---|
| `$XDG_CONFIG_HOME/cedit/settings.json` | The settings (default `~/.config/cedit/settings.json`) |
| `settings.json.invalid` | A copy of a settings file that was not valid JSON |
| `$HOME` | `~` in paths and the Open dialogs |
| `$XDG_DATA_HOME`, `$XDG_DATA_DIRS` | Where xcedit looks for font families |
| `XCEDIT_KEYS` | Testing: keys xcedit types at start (`~X` Alt+X, `^X` Ctrl+X, `\|` Enter, `!` Esc, `{x,y}` a click at a cell, `{x,y>x2,y2}` a drag) |
| `XCEDIT_SNAPSHOT=file.ppm` | Testing: xcedit writes each frame as an image |
| `XCEDIT_TRACE=file` | Testing: xcedit writes each event it receives |
| `.gitignore`, `.ignore`, `.git/info/exclude` | Files left out of the project searches |

### Exit status

0 on success; 1 when the terminal or the window cannot be opened, or
`--lex` fails.

### Source layout

| Folder | Contents |
|---|---|
| `src/core` | The text (piece table, undo), encodings, regular expressions, memory |
| `src/tui` | The screen buffer and the widgets of the immediate-mode UI |
| `src/app` | The editor: menus, text area, status bar, dialogs, search, languages |
| `src/app/langs` | One file per language, listed in `all.h` |
| `src/platform` | Files and folders, the terminal, threads (`posix/`), the window (`glfw/`) |
| `docs` | This manual, the man page, the language guide |
