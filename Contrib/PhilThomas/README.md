Gigatron Simulator
==================

The simulator loads the ROM using an `XmlHttpRequest`, so the application must be served up over HTTP.

The `package.json` can install an HTTP server, just do:

    npm install

Pick and download a [`*.rom`](https://github.com/kervinck/gigatron-rom/) file into the `src/` directory and rename it as `gigatron.rom`

Direct links:
[`ROM v1`](https://github.com/kervinck/gigatron-rom/raw/master/ROMv1.rom)
[`ROM v2`](https://github.com/kervinck/gigatron-rom/raw/master/ROMv2.rom)
[`ROM v3`](https://github.com/kervinck/gigatron-rom/raw/master/ROMv3.rom)

Start the HTTP server

    npm start

Point your browser at [localhost:8000/src](localhost:8000/src).

Use the cursor keys for the D-pad.

## GT1 Files

GT1 files can be loaded by dropping a `.gt1` file onto the VGA display from File Explorer or Finder.
