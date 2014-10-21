Polyphony JS
============

Polyphony JS is a library for real-time text editing. It provides a thin wrapper
API around libot, which is compiled to JavaScript using Emscripten. It can be
used with node.js or within a browser.

Getting Started
---------------

If you're using polyphony from within a browser, simply include
polyphony-debug.js or polyphony-release.js. The release version of Polyphony has
various asm.js performance optimizations and is also minified.

```js
<script type="text/javascript" src="polyphony-release.js"></script>
```

If you're using polyphony with node.js, simply install and require the polyphony
npm module.

```bash
$ npm install polyphony
```

```js
var polyphony = require("polyphony");
```

### Creating a Client

The easiest way to create a Polyphony client is by using the Quill adapter. This
adapter automatically turns a [Quill.js editor](http://quilljs.com) into a
real-time editor.

```js
var client = new polyphony.Client("ws://localhost:51015");
var editor = new Quill("#editor");
var adapter = new polyphony.QuillAdapter(editor, client);
```

### Creating a Server

A server is created by implementing the [`SocketServer`](lib/socket-server.js)
and [`Socket`](lib/socket.js) interfaces. For a simple implementation of these
interfaces, see the Polyphony site source.

```js
var socketServer = ...;
var server = new polyphony.Server(socketServer);
```

### Sample Project

The [demo site](http://polyphony-ot.com) for Polyphony serves as an excellent
example of how to use Polyphony JS. It was built using a minimal amount of code
and also has a bunch of walkthrough guides describing how everything works.

The code can be found at <https://github.com/polyphony-ot/polyphony-site>.

Building
--------

Building Polyphony JS requires that [Emscripten be installed][1] and included in
your PATH. Once you have Emscripten installed, you can build Polyphony JS by
running `make all`.

After running make, there will be two JavaScript files in the `bin` directory.
`bin/debug/polyphony.js` is a debug build with no optimizations and comments
linking the asm.js code to the original C code. `bin/release/polyphony.js` has
various optimizations and is minified. The release build should always be used
in production, as it's _significantly_ faster and smaller.

[1]: http://kripken.github.io/emscripten-site/docs/getting_started/downloads.html
