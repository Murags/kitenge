# Kitenge

**A symmetry-based pattern studio.** Design one motif; the program builds the whole pattern.

Kitenge is a desktop application that generates repeating geometric patterns
from one small shape. You pick a motif, a symmetry rule, colours and a tile
size. The program draws every copy itself and redraws the canvas the moment
anything changes.

Every pixel on the canvas comes from an algorithm we wrote ourselves.
SDL2 only puts our framebuffer on screen and Dear ImGui only draws the
control panel; neither draws anything on the canvas.

The original proposal is in [`docs/proposal.pdf`](docs/proposal.pdf).

## Features

**Core**
- Our own pixel rasteriser
- Six built-in motifs
- Three symmetry rules: translate, 4-fold rotation, mirror
- Live canvas with tile size and spacing controls
- RGB / HSV colour picker and palette
- Animated assembly playback

**Stretch goals**
- Pattern texture-mapped onto a lit 3D surface (OpenGL)
- Bezier pen tool for drawing your own motif
- Colour reduction from an uploaded photo

## Building

You need CMake 3.20 or newer, a C++17 compiler (Clang, GCC or MSVC) and Git.
SDL2, Dear ImGui and doctest are downloaded automatically the first time
you configure. If SDL2 is already installed (for example
`sudo apt install libsdl2-dev` on Linux), the installed copy is used instead.

```sh
cmake -S . -B build
cmake --build build --parallel
./build/kitenge            # Windows: build\Debug\kitenge.exe
```

Run the tests:

```sh
ctest --test-dir build --output-on-failure
```

The first configure takes a minute or two while SDL2 is built from source;
after that, rebuilds are quick.

## Project structure

```
src/
  main.cpp          window, main loop
  core/             shared types: Framebuffer, Color, Vec2, PatternSettings
  raster/           lines (DDA, Bresenham), circles, ellipses, clipping
  geometry/         motif shapes, Bezier curves, boundary and polygon fill
  symmetry/         Mat3 transforms, symmetry rules, tiling, assembly animation
  ui/               control panel, canvas view, colour picker
  motifs/           the six built-in motifs, export (Phase 2)
  gl/               3D textured surface (Phase 4, stretch goal)
tests/              unit tests (doctest)
cmake/              dependency setup
docs/               proposal and design notes
```

New `.cpp` files under `src/` and `tests/test_*.cpp` are picked up by the
build automatically.

## Graphics techniques

| Technique | Where it is used |
|---|---|
| Bresenham and DDA lines | Straight edges of every motif and the tile guides |
| Midpoint circle and ellipse | Round elements inside the motifs |
| Polygons and boundary fill | Colouring the enclosed regions of each motif |
| Bezier curves | Motifs with curved sides, and the pen tool |
| Line clipping | Trimming tiles that run past the canvas edge |
| 2D transformations, homogeneous coordinates | Rotating, mirroring and placing every copy |
| Raster operations and double buffering | Flicker-free redraw and the animated assembly |
| RGB and HSV colour models | Palette picker and photo colour reduction |
| Texture mapping and lighting *(stretch)* | Wrapping the pattern onto a lit 3D surface |

## Team

| Member | Module | Covers |
|---|---|---|
| Janny Jonyo | Rasteriser and primitives | Framebuffer, Bresenham and DDA, midpoint circle and ellipse, line clipping |
| Kristina Kemoi | Curves and fill | Bezier evaluation, motif geometry, boundary fill and area attributes |
| Dennis Murage | Symmetry engine | Transformation matrices, tiling logic, the animated assembly |
| Neema Mapelu | Interface and colour | Layout and controls, RGB / HSV picker, palette handling |
| Jeff Kioko | Motif library and export | The six built-in motifs, image export, photo colour reduction |
| Cindy Ogutu | 3D, integration and testing | OpenGL surface, merging the modules, test harness, documentation |

See [CONTRIBUTING.md](CONTRIBUTING.md) for the branch and pull request workflow.

## Roadmap

Each phase uses the technique taught in that week's lab.

| Phase | Weeks | Focus |
|---|---|---|
| 1. Concept and setup | 1-3 | Repository, build, the shared framebuffer interface, app shell |
| 2. Core graphics | 4-5 | Lines, circles, polygons; motif geometry and fill; first shape on screen |
| 3. Advanced graphics | 7-9 | Bezier curves, symmetry engine, assembly animation |
| 4. Integration | 11-12 | OpenGL textured surface, testing and refinement, documentation |
