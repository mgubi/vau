#!/usr/bin/env python3
###############################################################################
# MODULE     : package.py
# DESCRIPTION: The resources of Vau for the browser build, in packages
# COPYRIGHT  : (C) 2026  Massimiliano Gubinelli
###############################################################################
# This software falls under the GNU general public license version 3 or later.
# It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
# in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
###############################################################################
#
#   package.py <resources dir> <output dir> <boot list>
#
# Writes <output dir>/vau-files.json and, in <output dir>/pkg, the packages
# it names. A package is the concatenation of its files; the manifest gives,
# for each file, its package, offset and size. The program
# (platform/wasm/vau_packages.js) loads the boot package before Vau starts
# and the others one after the other once it runs; a file read before its
# package has come brings its whole package at once.
#
# The boot package holds the files Vau opens when it starts and typesets a
# plain document (the boot list, platform/wasm/boot-files.txt) and some whole
# groups which are small and read at unforeseeable times (the Scheme code,
# the styles, the encodings of the fonts).
#
# The fonts (LAZY: the OpenType files, two thirds of the whole) and the
# example documents are not in a package: each is a file of its own, which
# the program fetches when Vau first reads it; the manifest lists them under
# "lazy". Those of the boot list stay in the boot package.
#
# (Adapted from misc/wasm/package.py of TeXmacs.)

import gzip, hashlib, json, os, sys, fnmatch

ROOT = '/Vau'

# what a viewer does not need: the icons of the interface, the tests
EXCLUDE = ['misc/pixmaps', 'misc/images', 'progs/check', '*.DS_Store']

BOOT_GROUPS = ['progs/', 'styles/', 'packages/', 'langs/encoding/',
               'fonts/enc/', 'fonts/virtual/']
BOOT_FILES = ['fonts/font-database.scm', 'fonts/font-characteristics.scm',
              'fonts/font-features.scm', 'fonts/font-substitutions.scm',
              'fonts/pdf-font-issues.scm']

# the other packages, in the order they are loaded: (name, prefixes); a
# package larger than CHUNK is split
PACKAGES = [
  ('langs', ['langs/']),
  ('fonts', ['fonts/']),
  ('misc',  ['']),
]
CHUNK = 4 * 1024 * 1024

# the files which are fetched only when Vau reads them
LAZY_DIRS = ['fonts/truetype/', 'vau-tests/', 'examples/']
LAZY_EXTS = ['.otf', '.ttf', '.ttc', '.tm']

def lazy (rel):
  return (any (rel.startswith (d) for d in LAZY_DIRS) and
          os.path.splitext (rel)[1].lower () in LAZY_EXTS)

def excluded (rel):
  return any (fnmatch.fnmatch (rel, e) or fnmatch.fnmatch (os.path.basename (rel), e)
              or rel.startswith (e + '/') for e in EXCLUDE)

def main ():
  if len (sys.argv) != 4:
    sys.exit ('usage: package.py <resources dir> <output dir> <boot list>')
  root, out, boot_list = sys.argv[1:]
  files = []
  for d, ds, fs in os.walk (root):
    rd = os.path.relpath (d, root)
    rd = '' if rd == '.' else rd + '/'
    ds[:] = sorted (x for x in ds if not excluded (rd + x))
    for f in sorted (fs):
      if not excluded (rd + f): files.append (rd + f)
  boot = set ()
  if os.path.exists (boot_list):
    for line in open (boot_list):
      p = line.strip ()
      if p.startswith (ROOT + '/'): p = p[len (ROOT) + 1:]
      if p and not p.startswith ('#'): boot.add (p)

  # nothing to do when the resources, the boot list and this script are
  # those of the last run
  me = os.path.abspath (__file__)
  stamp = hashlib.sha1 ()
  for extra in (me, boot_list):
    if os.path.exists (extra): stamp.update (open (extra, 'rb').read ())
  for rel in files:
    st = os.stat (os.path.join (root, rel))
    stamp.update (('%s %d %d\n' % (rel, st.st_size, st.st_mtime_ns)).encode ())
  stamp = stamp.hexdigest ()
  manifest_path = os.path.join (out, 'vau-files.json')
  try:
    old = json.load (open (manifest_path))
    names = [p['url'] for p in old['packages']] + [p['gz'] for p in old['packages']] \
            + [f[1] for f in old['lazy']]
    if old.get ('stamp') == stamp and all (os.path.exists (os.path.join (out, n)) for n in names):
      print ('package.py: %s is up to date' % manifest_path)
      return
  except Exception:
    pass

  groups = [('boot', [])] + [(name, []) for name, _ in PACKAGES]
  lazies = []
  for rel in files:
    if rel in boot or rel in BOOT_FILES or any (rel.startswith (g) for g in BOOT_GROUPS):
      groups[0][1].append (rel)
      continue
    if lazy (rel):
      lazies.append (rel)
      continue
    for i, (name, prefixes) in enumerate (PACKAGES):
      if any (rel.startswith (p) for p in prefixes):
        groups[i + 1][1].append (rel)
        break
  # the chunks: a package larger than CHUNK becomes several
  chunks = []
  for name, rels in groups:
    part, size, n = [], 0, 1
    for rel in rels:
      s = os.path.getsize (os.path.join (root, rel))
      if part and size + s > CHUNK and name != 'boot':
        chunks.append ((name + '-' + str (n), part)); n += 1
        part, size = [], 0
      part.append (rel); size += s
    if part: chunks.append ((name if n == 1 else name + '-' + str (n), part))

  pkg = os.path.join (out, 'pkg')
  os.makedirs (pkg, exist_ok = True)
  manifest = { 'root': ROOT, 'stamp': stamp, 'packages': [], 'lazy': [] }
  written = set ()
  def write (name, data):
    written.add (name)
    path = os.path.join (pkg, name)
    if not (os.path.exists (path) and os.path.getsize (path) == len (data)):
      open (path, 'wb').write (data)
  for name, rels in chunks:
    data, entries = bytearray (), []
    for rel in rels:
      b = open (os.path.join (root, rel), 'rb').read ()
      entries.append ([rel, len (data), len (b)])
      data += b
    digest = hashlib.sha1 (data).hexdigest ()[:10]
    base = 'vau-%s-%s.pack' % (name, digest)
    write (base, data)
    gz = base + '.gz'
    if not os.path.exists (os.path.join (pkg, gz)):
      # mtime 0: the same bytes at each run
      open (os.path.join (pkg, gz), 'wb').write (gzip.compress (bytes (data), 9, mtime = 0))
    written.add (gz)
    manifest['packages'].append ({
      'name': name, 'boot': name == 'boot', 'url': 'pkg/' + base,
      'gz': 'pkg/' + gz, 'size': len (data),
      'gzsize': os.path.getsize (os.path.join (pkg, gz)), 'files': entries })
  for rel in lazies:
    b = open (os.path.join (root, rel), 'rb').read ()
    digest = hashlib.sha1 (b).hexdigest ()[:10]
    name = 'vau-file-%s%s' % (digest, os.path.splitext (rel)[1].lower ())
    write (name, b)
    manifest['lazy'].append ([rel, 'pkg/' + name, len (b)])
  # the files of older builds go
  for f in os.listdir (pkg):
    if f.startswith ('vau-') and f not in written: os.remove (os.path.join (pkg, f))
  json.dump (manifest, open (manifest_path, 'w'), separators = (',', ':'))

  total = sum (p['size'] for p in manifest['packages'])
  print ('package.py: %d files in %d packages (%.1f MB, boot %.1f MB, %.1f MB compressed), %d files on demand (%.1f MB)'
         % (sum (len (p['files']) for p in manifest['packages']), len (manifest['packages']),
            total / 1e6, manifest['packages'][0]['size'] / 1e6,
            manifest['packages'][0]['gzsize'] / 1e6,
            len (lazies), sum (f[2] for f in manifest['lazy']) / 1e6))

if __name__ == '__main__':
  main ()
