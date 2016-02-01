# pebble-fctx-demo

This is a demonstration app for the [pebble-fctx](https://www.github.com/jrmobley/pebble-fctx) library.

This project uses submodules.  Use `git submodule update` to get them.

Optionally, if you want to run the fctx-compiler, you will need to install its dependencies with `cd tools/fctx-compiler; npm install`.  To run the tool, from the root project directory, run `tools/fctx-compiler/fctx-compiler font/din-condensed.svg`.  This will write `din-condensed.ffont` to the `resources` directory.  This file is already part of the repo, so you don't need to do this just to build the demo.

![Aplite](http://jrmobley.github.io/pebble-fctx-demo/images/fctx-demo-aplite.png)
![Basalt](http://jrmobley.github.io/pebble-fctx-demo/images/fctx-demo-basalt.png)
![Chalk](http://jrmobley.github.io/pebble-fctx-demo/images/fctx-demo-chalk.png)
