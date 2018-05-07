Gigatron Simulator
==================

The simulator loads the ROM using an `XmlHttpRequest`, so the application must be served up over HTTP.

The `package.json` can install an HTTP server, just do:

    npm install

Download [`theloop.2.rom`](https://github.com/kervinck/gigatron-rom/raw/master/theloop.2.rom) into the `src` directory.

Start the HTTP server

    npm start

Point your browser at [localhost:8000/src](localhost:8000/src).

Use the cursor keys for the D-pad.

## GT1 Files

GT1 files can be loaded by starting the `Loader` and dropping a `.gt1` file onto the VGA display from File Explorer or Finder.
