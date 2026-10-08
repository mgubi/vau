// The resources of Vau in the browser (a --pre-js of the WebAssembly build).
//
// devel/package.py writes the resources as packages, with a manifest
// (vau-files.json) giving for each file its package, offset and size. Before
// Vau starts, the whole tree is created under /Vau, every file as a
// placeholder of its size, and the boot package is loaded into it. Once Vau
// runs, the other packages are loaded one after the other, in the
// background, and fill in their placeholders. A placeholder which is read
// before its package has come fetches the whole package now (a synchronous
// request: Vau reads its files synchronously, and it runs in a worker,
// where such requests are allowed), so that Vau never finds a file of its
// tree missing.
//
// The fonts and the example documents (the manifest's "lazy" files) are in
// no package: each is a file of its own, fetched whole when Vau first reads
// it. A font which is never used is never fetched.
//
// The names of the packages and of the files carry a digest of their
// contents: the browser keeps them in its cache from one visit to the next.
//
// Under node there is nothing to fetch: the tests mount the resources of
// the source tree instead (test-node.mjs).
//
// (Adapted from misc/wasm/packages.js of TeXmacs.)

var vauPackages = (function () {
  if (typeof XMLHttpRequest === 'undefined' || typeof fetch === 'undefined' ||
      typeof self === 'undefined' || !self.location)
    return null;

  var manifest = null;
  var stats = { start: 0, bootMs: 0, loaded: 0, onDemand: 0, lazy: 0, lazyBytes: 0,
                background: false };
  var pending = {};   // package name -> [node] still to fill
  var lazyNodes = {}; // the url of a lazy file -> its placeholders
  var filesRead = []; // the files read so far, in order (for the boot list)

  function url (name) { return new URL (name, self.location.href).href; }
  function status (text) { if (Module['setStatus']) Module['setStatus'] (text); }
  function mb (n) { return (n / 1048576).toFixed (1); }

  // the bytes of a package (onBytes (n): the bytes which came). Its gzip
  // copy when the browser can decompress it: the servers of static files
  // (GitHub Pages) do not compress the packages themselves
  var gunzip = typeof DecompressionStream !== 'undefined';
  async function fetchPackage (pkg, onBytes) {
    var gz = gunzip && pkg.gz;
    var resp = await fetch (url (gz ? pkg.gz : pkg.url));
    if (!resp.ok) throw new Error ('cannot load ' + pkg.name + ': ' + resp.status);
    var body = resp.body;
    if (body && onBytes) {
      // count the bytes as they come from the network
      var got = 0;
      body = body.pipeThrough (new TransformStream ({
        transform: function (chunk, controller) {
          got += chunk.length; onBytes (got); controller.enqueue (chunk);
        } }));
    }
    if (gz && body) {
      // a server may have sent the copy with Content-Encoding: gzip, and
      // then the browser decompressed it already
      var enc = resp.headers.get ('Content-Encoding');
      if (!enc || enc === 'identity')
        body = body.pipeThrough (new DecompressionStream ('gzip'));
    }
    var bytes = new Uint8Array (await new Response (body).arrayBuffer ());
    if (bytes.length !== pkg.size)
      throw new Error ('package ' + pkg.name + ' has ' + bytes.length + ' bytes, not ' + pkg.size);
    return bytes;
  }

  // a request now (synchronous)
  function getNow (address, size, what) {
    var xhr = new XMLHttpRequest ();
    xhr.open ('GET', address, false);
    xhr.responseType = 'arraybuffer';
    xhr.send (null);
    if (xhr.status !== 200 || !xhr.response || xhr.response.byteLength !== size)
      throw new Error ('cannot load ' + what + ': ' + xhr.status);
    return new Uint8Array (xhr.response);
  }

  // the bytes of one file, now: its own file, or its whole package, all of
  // whose files are then installed
  function fetchNow (node) {
    var pkg = node.vauPackage, t = performance.now ();
    if (pkg.lazy) {
      var u = url (pkg.url);
      var bytes = getNow (u, pkg.size, node.vauPath);
      stats.lazy++;
      stats.lazyBytes += pkg.size;
      console.log ('Vau: ' + node.vauPath + ' loaded on demand (' + mb (pkg.size) + ' MB, ' +
                   Math.round (performance.now () - t) + ' ms)');
      (lazyNodes[u] || []).forEach (function (n) { if (n !== node && n.vauPackage) fill (n, bytes); });
      return bytes;
    }
    var all = getNow (url (pkg.url), pkg.size, node.vauPath);
    installNow (pkg, all);
    stats.onDemand++;
    console.log ('Vau: ' + node.vauPath + ' loaded on demand, with its package ' + pkg.name +
                 ' (' + mb (pkg.size) + ' MB, ' + Math.round (performance.now () - t) + ' ms)');
    return all.subarray (node.vauOffset, node.vauOffset + node.vauSize);
  }

  function fill (node, bytes) {
    node.contents = bytes;
    node.vauPackage = null;
  }
  function materialize (node) {
    if (!node.vauRead) { node.vauRead = true; filesRead.push (node.vauPath); }
    if (node.vauPackage) fill (node, fetchNow (node));
  }

  // a file of the tree, before its bytes: its size is known, a read brings
  // its bytes if its package has not yet
  function placeholder (dir, name, pkg, offset, size) {
    var node = FS.createFile (dir, name, {}, true, false);
    node.contents = null;
    node.vauPackage = pkg;
    node.vauOffset = offset;
    node.vauSize = size;
    node.vauPath = dir + '/' + name;
    Object.defineProperty (node, 'usedBytes', {
      get: function () { return this.contents ? this.contents.length : this.vauSize; },
      set: function (v) {}, configurable: true
    });
    var base = node.stream_ops, ops = {};
    for (var k in base) ops[k] = base[k];
    ops.read = function (stream, buffer, offset, length, position) {
      materialize (node);
      return base.read (stream, buffer, offset, length, position);
    };
    if (base.mmap) ops.mmap = function () {
      materialize (node);
      return base.mmap.apply (null, arguments);
    };
    node.stream_ops = ops;
    return node;
  }

  // the time of the files: one per build, the same at each visit
  function buildTime () {
    var s = manifest.stamp || '', h = 0;
    for (var i = 0; i < s.length; i++) h = (h * 31 + s.charCodeAt (i)) >>> 0;
    return Date.UTC (2020, 0, 1) + (h % (5 * 365 * 86400)) * 1000;
  }
  function setTime (node, t) {
    node.timestamp = node.atime = node.mtime = node.ctime = t;
  }

  function createTree () {
    var made = {}, root = manifest.root, time = buildTime ();
    function mkdir (d) {
      if (made[d]) return;
      made[d] = true;
      try { FS.mkdirTree (d); } catch (e) {}
    }
    function add (rel, pkg, offset, size) {
      var p = root + '/' + rel, i = p.lastIndexOf ('/'), dir = p.slice (0, i);
      mkdir (dir);
      var node = placeholder (dir, p.slice (i + 1), pkg, offset, size);
      setTime (node, time);
      return node;
    }
    manifest.packages.forEach (function (pkg) {
      pending[pkg.name] = pkg.files.map (function (f) { return add (f[0], pkg, f[1], f[2]); });
    });
    // the lazy files: a placeholder each, whose "package" is the file itself
    (manifest.lazy || []).forEach (function (f) {
      var pkg = { name: f[0], url: f[1], size: f[2], lazy: true };
      var u = url (f[1]);
      (lazyNodes[u] = lazyNodes[u] || []).push (add (f[0], pkg, 0, f[2]));
    });
    // the directories last (a file made in one changes its time)
    Object.keys (made).forEach (function (d) {
      for (; d.length >= root.length; d = d.slice (0, d.lastIndexOf ('/')))
        try { setTime (FS.lookupPath (d).node, time); } catch (e) {}
    });
  }

  // the bytes of a package into its placeholders
  function installNow (pkg, bytes) {
    if (pkg.vauInstalled) return;
    var nodes = pending[pkg.name];
    for (var i = 0; i < nodes.length; i++) {
      var n = nodes[i];
      if (n.vauPackage) fill (n, bytes.subarray (n.vauOffset, n.vauOffset + n.vauSize));
    }
    pending[pkg.name] = [];
    pkg.vauInstalled = true;
    stats.loaded++;
  }

  async function background () {
    var rest = manifest.packages.filter (function (p) { return !p.boot; });
    for (var i = 0; i < rest.length; i++) {
      if (rest[i].vauInstalled) continue; // fetched on demand
      try { installNow (rest[i], await fetchPackage (rest[i])); }
      catch (e) { console.error ('Vau: package ' + rest[i].name + ': ' + e.message); }
    }
    stats.background = true;
    console.log ('Vau: all the packages are there (' + stats.loaded + ' in ' +
                 Math.round (performance.now () - stats.start) + ' ms, ' + stats.onDemand +
                 ' on demand before)');
  }

  // The manifest and the bytes of the boot package are fetched as soon as
  // this runs, while the program comes and compiles; only their
  // installation, which needs the file system, waits for preRun.
  var early = null;
  function fetchBoot () {
    if (early) return early;
    stats.start = performance.now ();
    early = fetch (url ('vau-files.json'), { cache: 'no-cache' }).then (function (r) {
      if (!r.ok) throw new Error ('cannot load vau-files.json: ' + r.status);
      return r.json ();
    }).then (async function (m) {
      var boot = m.packages.filter (function (p) { return p.boot; });
      var total = 0, done = 0, bytes = [];
      boot.forEach (function (p) { total += gunzip && p.gz ? p.gzsize : p.size; });
      for (var i = 0; i < boot.length; i++) {
        bytes.push (await fetchPackage (boot[i], function (n) {
          status ('Loading Vau… ' + mb (done + n) + ' of ' + mb (total) + ' MB');
        }));
        done += gunzip && boot[i].gz ? boot[i].gzsize : boot[i].size;
      }
      return { manifest: m, boot: boot, bytes: bytes };
    });
    return early;
  }
  fetchBoot ().catch (function () {}); // reported by preRun

  Module['preRun'] = Module['preRun'] || [];
  Module['preRun'].push (function () {
    addRunDependency ('vau-files');
    fetchBoot ().then (function (r) {
      manifest = r.manifest;
      createTree ();
      for (var i = 0; i < r.boot.length; i++) {
        installNow (r.boot[i], r.bytes[i]);
        r.bytes[i] = null;
      }
      stats.bootMs = Math.round (performance.now () - stats.start);
      console.log ('Vau: boot files in ' + stats.bootMs + ' ms');
      status ('Starting Vau…');
      removeRunDependency ('vau-files');
      // the rest once Vau runs; ?no-background leaves it to the demand, to
      // test that path
      if (!new URLSearchParams (self.location.search).has ('no-background'))
        setTimeout (background, 500);
    }).catch (function (e) {
      console.error ('Vau: cannot load its files', e);
      if (Module['onAbort']) Module['onAbort'] ('Cannot load the files of Vau: ' + e.message);
    });
  });

  var api = { stats: stats, filesRead: function () { return filesRead.slice (); } };
  Module['vauPackages'] = api;
  return api;
})();
