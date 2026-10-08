// A test of the WebAssembly build without a browser, run in the build
// directory:
//
//   node test-node.mjs [document] [output prefix]
//
// It starts the library on the resources of the source tree (their place is
// in vau-build.json, or VAU_RESOURCES), typesets a document (one of the
// resources, as /Vau/..., or a file of this machine), draws its first page and a view
// of it, and writes <prefix>.png and <prefix>.pdf.

import fs from "node:fs";
import path from "node:path";
import { createRequire } from "node:module";
import { fileURLToPath } from "node:url";

const here = path.dirname(fileURLToPath(import.meta.url));
const require = createRequire(import.meta.url);
const libvau = require(path.join(here, "Vau-wasm.js"));

let doc = process.argv[2] || "/Vau/vau-tests/grassmann-sq-example.tm";
const prefix = process.argv[3] || "vau-test";
const quiet = !process.env.VAU_VERBOSE;

function check(cond, what) {
	console.log(`${cond ? "ok  " : "FAIL"} ${what}`);
	if (!cond) process.exitCode = 1;
}

let t0 = Date.now();
const lap = () => { const t = Date.now(), d = t - t0; t0 = t; return `${d} ms`; };

// under node the resources are those of the source tree, mounted as /Vau
// (in the browser they come in packages: vau_packages.js)
const resources = process.env.VAU_RESOURCES ||
	JSON.parse(fs.readFileSync(path.join(here, "vau-build.json"), "utf8")).resources;

const vau = await libvau({
	print: quiet ? () => {} : console.log,
	printErr: quiet ? () => {} : console.error,
	preRun: [module => {
		module.FS.mkdir("/Vau");
		module.FS.mount(module.FS.filesystems.NODEFS, { root: resources }, "/Vau");
	}]
});
vau._wasm_init_vau();
console.log(`     started in ${lap()}`);

if (fs.existsSync(doc)) {
	// a file of this machine: copy it into the file system of the library
	vau.FS.mkdir("/work");
	const inside = "/work/" + path.basename(doc);
	vau.FS.writeFile(inside, fs.readFileSync(doc));
	doc = inside;
}

check(vau.ccall("wasm_open_document", "number", ["string"], ["/no/such/file.tm"]) === 0,
	"a missing document is refused");

const pages = vau.ccall("wasm_open_document", "number", ["string"], [doc]);
check(pages > 0, `${doc}: ${pages} pages, typeset in ${lap()}`);
check(vau._wasm_get_nr_pages() === pages, "the number of pages is kept");

const w = vau._wasm_get_page_width(1, 5), h = vau._wasm_get_page_height(1, 5);
check(w > 0 && h > 0, `page 1 is ${w} x ${h} pixels at zoom 5`);

vau._wasm_get_page_pixmap(1);
let pix = vau.vauPixmap;
check(pix && pix.width === w && pix.height === h &&
	pix.data.length === 4 * w * h, `pixmap of page 1 (${lap()})`);
const ink = pix.data.some((v, i) => i % 4 !== 3 && v < 128);
check(ink, "the page is not blank");

vau._wasm_get_view_pixmap(1, 800, 600, 2.5, 100, 200);
pix = vau.vauPixmap;
check(pix && pix.width === 800 && pix.height === 600, `view of 800 x 600 (${lap()})`);

vau.FS.mkdir("/out");
vau.ccall("wasm_save_page_png", null, ["number", "string"], [1, "/out/page.png"]);
check(vau.FS.analyzePath("/out/page.png").exists, `PNG of page 1 (${lap()})`);
if (vau.FS.analyzePath("/out/page.png").exists)
	fs.writeFileSync(prefix + ".png", vau.FS.readFile("/out/page.png"));

const pdf = vau.ccall("wasm_export_pdf", "number", ["string"], ["/out/doc.pdf"]);
check(pdf === 1, `PDF export (${lap()})`);
if (pdf) {
	const bytes = vau.FS.readFile("/out/doc.pdf");
	check(Buffer.from(bytes.slice(0, 5)).toString() === "%PDF-", `the PDF has ${bytes.length} bytes`);
	fs.writeFileSync(prefix + ".pdf", bytes);
}

const v = vau.ccall("wasm_eval_to_string", "string", ["string"], ["(+ 1 2)"]);
check(v === "3", `Scheme: (+ 1 2) is ${v}`);

console.log(process.exitCode ? "FAILED" : "all passed");
