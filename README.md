# Roadrunner

A simple 2D software renderer written in C with no dependencies.

## Usage

1. Compile using the Makefile (This also compiles the `example.c` file)
2. Import `librr.a` into your project and link it
3. Import all the files inside `include/`.

**NOTE**: the Makefile might explain this sequence a little better

## Features

- Pure C99
- Shapes rendering (triangles, rectangles, circles), which can all be filled or not
- Line rendering
- Text rendering
- Simple image loading (with the help of stb_image(_write).h) and rendering
- A endianness switcher (from RGBA to ABGR)

## Problems

- Sometimes line rendering breaks the program
- Rotation is a bit faulty

Huge thanks to [Tigr](https://github.com/erkkah/tigr), a lot of Roadrunner's code snippets are from there!
