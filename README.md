# Acchording - Format Chord Sheets

This program takes a plain-text file formatted in a certain way that describes how to output a formatted chord sheet. The program can print the formatted chord sheet in plain text and generate a PDF.

# File Format

## Header

```
author: John Doe
title: Song Doe
capo: None
key: G
tuning: Standard
```

## Sections

### Plain Text

```
[Section Name]
Happy Birthday
to you
```

### Chords

Chords are inserted, in order, in new lines above the `>`s, which are removed.

```
[Section Name]
chords: C G
Happy >Birthday
to >you
```

### Special Sections

```
[!Section Name] (Hides section name in output)
[>Section Name] (Defines section to be reproduced later)
[<Section Name] (Reproduces section)
```

(You can, of course, also specify the chords already formatted in plain text.)

## Example

See `battlehymn.txt`.

# Usage

## Plain Text

```
$ acchording song.txt
$ acchording song.txt > song-chords.txt
```

## PDF

Supply font (`.ttf` format) with `-f`, font size with `-s`. If you want to print UTF-8 encoded characters (like Cyrillic ones), you have to pass `-u`.

```
$ acchording -p song.txt # Outputs song.pdf
$ acchording -f "Ubuntu Mono:Regular" -s 12 song.txt
```

# Building and Requirements

## Libraries

- [fmtlib](https://github.com/fmtlib/fmt)
- [libHaru](http://libharu.org/)
- [fontconfig](https://www.freedesktop.org/wiki/Software/fontconfig/)

## Fonts

Fonts are fetched with fontconfig. Supply TrueType fonts by names that it will find.

## Building with Make

You can configure some default values by copying `src/config.def.hpp` into `src/config.hpp` and editing that. If you don't, the default values from `src/config.def.hpp` will be used.

```
$ make
```

# License

Licensed under the GNU General Public License Version 3, see LICENSE.
