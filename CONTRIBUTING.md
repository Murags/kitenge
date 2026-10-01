# Contributing to Kitenge

## The one rule

Every pixel on the pattern canvas is written by an algorithm we implement
ourselves, through `Framebuffer::setPixel`. SDL2 only shows the finished
framebuffer and Dear ImGui only draws the control panel. Neither is ever
used to draw lines, shapes or fills on the canvas.

## Workflow

1. Pull the latest `main` before starting:
   ```
   git switch main && git pull
   ```
2. Create a branch for one piece of work, named `<area>/<short-topic>`:
   - `raster/bresenham-line`
   - `geometry/boundary-fill`
   - `symmetry/four-fold`
   - `ui/hsv-picker`
   - `motifs/leaf`
   - `gl/textured-surface`
   - `setup/...`, `docs/...` and `fix/...` for everything else
3. Commit in small steps that each build. Write messages in the imperative
   ("Add midpoint circle", not "added circle stuff"). Keep the subject under
   about 70 characters, and use the body to explain *why*.
4. Build and run the tests before pushing (see the README).
5. Push the branch and open a pull request into `main`. Fill in the template
   and ask the owner of any module you touched to review.
6. Merge with **Create a merge commit**, not squash or rebase, so each
   person's commits stay in the history as they wrote them.

## Code style

- C++17. Format with the repo's `.clang-format` (`clang-format -i <file>`);
  most editors pick it up automatically along with `.editorconfig`.
- Everything lives in `namespace kitenge`.
- Types are `PascalCase`, functions and variables `camelCase`, constants
  `kPascalCase`, and private members end in `_`.
- Headers use `#pragma once`. Includes are written relative to `src/`, for
  example `#include "core/Framebuffer.h"`.
- Each module owns its folder under `src/`. If you need a change in someone
  else's module, ask them or tag them on the PR.
