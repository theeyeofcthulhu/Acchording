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

Chords are inserted, in order, in new lines above the `<`s, which are removed.

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

Supply font (`.ttf` format) with `-f`, font size with `-s`.

```
$ acchording -p song.txt # Outputs song.pdf
$ acchording -f Font-Regular.ttf -s 12 song.txt
```

# Building and Requirements

## Libraries

- [fmtlib](https://github.com/fmtlib/fmt)
- [libHaru](http://libharu.org/)

## Fonts

Fonts folder contains [Ubuntu Mono](https://fonts.google.com/specimen/Ubuntu+Mono) and [Inconsolata](https://fonts.google.com/specimen/Inconsolata) (default when generating PDF) along with their licenses.

## Building with CMake

```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

# License

Licensed under the GNU General Public License Version 3, see LICENSE.
