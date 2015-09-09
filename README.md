# pebble-fctx-demo

This is a demonstration app for the pebble-fctx library.

This project uses submodules.  Use `git submodule update` to get them.

Optionally, if you want to run the fctx-compiler, you will need to install its dependencies with `cd tools/fctx-compiler; npm install`.  To run the tool, from the root project directory, run `tools/fctx-compiler/fctx-compiler resources.svg`.  This will write `DINdigits.ffont` to the `resources` directory.  This file is already part of the repo, so you don't need to do this just to build the demo.