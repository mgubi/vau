// The worker which runs Vau: the page (Vau.html) sends [method, id, args]
// and gets ["RESULT", id, value] or ["ERROR", id, error] back; once the
// library has started it sends ["READY", names of the methods].

"use strict";

importScripts("Vau-wasm.js");

let vau = null;        // the module, once instantiated
const methods = {};

// call a function of the library with strings among its arguments
function call(name, ret, types, args) {
	return vau.ccall(name, ret, types, args);
}

// the pixmap left by the library (mylib.js), to be transferred to the page
function takePixmap() {
	const pix = vau.vauPixmap;
	vau.vauPixmap = null;
	return pix;
}

// load and typeset a document of the file system of the library, return
// { pages }; pages is 0 when the document could not be opened
methods.openDocument = function (path) {
	return { pages: call("wasm_open_document", "number", ["string"], [path]) };
};

// write a document sent by the page (an ArrayBuffer) and open it
methods.openBuffer = function (buffer, name) {
	const dir = "/work";
	if (!vau.FS.analyzePath(dir).exists) vau.FS.mkdir(dir);
	const path = dir + "/" + name.replace(/[^A-Za-z0-9._-]/g, "_");
	vau.FS.writeFile(path, new Uint8Array(buffer));
	return methods.openDocument(path);
};

methods.pageCount = function () {
	return vau._wasm_get_nr_pages();
};

// the size in pixels of a page at a zoom factor
methods.pageSize = function (page, zoom) {
	return {
		width: vau._wasm_get_page_width(page, zoom),
		height: vau._wasm_get_page_height(page, zoom)
	};
};

// a whole page, at the resolution the document was typeset for
methods.getPagePixmap = function (page) {
	vau._wasm_get_page_pixmap(page);
	return takePixmap();
};

// the part of a page seen in a view of width x height pixels
methods.getViewPixmap = function (page, width, height, zoom, scrollX, scrollY) {
	vau._wasm_get_view_pixmap(page, width, height, zoom, scrollX | 0, scrollY | 0);
	return takePixmap();
};

// the document as a PDF file (an ArrayBuffer), or null
methods.exportPdf = function () {
	const path = "/tmp/vau-export.pdf";
	if (!vau.FS.analyzePath("/tmp").exists) vau.FS.mkdir("/tmp");
	if (!call("wasm_export_pdf", "number", ["string"], [path])) return null;
	const data = vau.FS.readFile(path);
	vau.FS.unlink(path);
	return data.buffer;
};

// evaluate a Scheme expression, return its value as written by Scheme
methods.evalScheme = function (code) {
	return call("wasm_eval_to_string", "string", ["string"], [code]);
};

// the files read so far (for the boot list, platform/wasm/boot-files.txt)
// and the state of the loading of the packages
methods.filesRead = function () {
	return vau.vauPackages ? vau.vauPackages.filesRead() : [];
};
methods.packageStats = function () {
	return vau.vauPackages ? Object.assign({}, vau.vauPackages.stats) : null;
};

function transferables(value) {
	if (value instanceof ArrayBuffer) return [value];
	if (value && value.data && value.data.buffer instanceof ArrayBuffer)
		return [value.data.buffer];
	return [];
}

onmessage = async function (event) {
	const [func, id, args] = event.data;
	try {
		await ready;
		const result = methods[func](...args);
		postMessage(["RESULT", id, result], transferables(result));
	} catch (error) {
		postMessage(["ERROR", id,
			{ name: error.name, message: String(error.message || error), stack: error.stack }]);
	}
};

// The resources of the library come in packages (vau_packages.js, in the
// program): the boot package before it starts, the others in the
// background, the fonts and the examples when they are first read.

const ready = libvau({
	print: text => postMessage(["LOG", text]),
	printErr: text => postMessage(["LOG", text]),
	setStatus: text => { if (text) postMessage(["STATUS", text]); },
	onAbort: what => postMessage(["FAILED", String(what)])
}).then(module => {
	vau = module;
	vau._wasm_init_vau();
	postMessage(["READY", Object.keys(methods)]);
}).catch(error => {
	postMessage(["FAILED", String(error && error.message || error)]);
	throw error;
});
