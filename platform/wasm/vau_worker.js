

"use strict";

// Import the WASM module
// We do additional fetches to these paths to have better error messages in case
// they're missing because the user forgot to compile them.
checkPath("Vau-wasm.wasm");
checkPath("Vau-wasm.js");
importScripts("Vau-wasm.js");
//importScripts("lib/mupdf.js");

function checkPath(path) {
	fetch(path, { method: "HEAD" }).then(response => {
		if (!response.ok)
			postMessage(["ERROR", `Failed to load ${path}: Status ${response.status}. This likely indicates that Vau wasn't compiled to wasm.`]);
	});
}


// A list of RegExp objects to check function names against
let logFilters = [];

function logCall(id, funcName, args) {
	for (const filter of logFilters) {
		if (filter.test(funcName)) {
			console.log(`(${id}) CALL ${funcName}:`, args);
			return;
		}
	}
}

function logReturn(id, funcName, value) {
	for (const filter of logFilters) {
		if (filter.test(funcName)) {
			console.log(`(${id}) RETURN ${funcName}:`, value);
			return;
		}
	}
}

onmessage = async function (event) {
	let [ func, id, args ] = event.data;
	await vau_ready;

	try {
		logCall(id, func, args);
		let result = workerMethods[func](...args);
		logReturn(id, func, result);
		postMessage(["RESULT", id, result]);
	} catch (error) {
		if (error instanceof VauTryLaterError) {
			trylaterQueue.push(event);
		} else {
			postMessage(["ERROR", id, {name: error.name, message: error.message, stack: error.stack}]);
		}
	}
};

let trylaterScheduled = false;
let trylaterQueue = [];
var onFetchCompleted = function (_id) {
	if (!trylaterScheduled) {
		trylaterScheduled = true;

		setTimeout(() => {
			trylaterScheduled = false;
			let currentQueue = trylaterQueue;
			trylaterQueue = [];
			currentQueue.forEach(onmessage);
		}, 0);
	}
};

class VauError extends Error {
	constructor(message) {
		super(message);
		this.name = "MupdfError";
	}
}

class VauTryLaterError extends VauError {
	constructor(message) {
		super(message);
		this.name = "MupdfTryLaterError";
	}
}


const workerMethods = {};


function allocateUTF8(str) {
	var size = libvau.lengthBytesUTF8(str) + 1;
	var pointer = libvau._malloc(size);
	libvau.stringToUTF8(str, pointer, size);
	return pointer;
}

workerMethods.openDocument = function (str) {
//	console.log(`Typesetting ${str} ...`);
	var p= allocateUTF8(str);
	libvau._wasm_open_document(p);
	libvau._free(p)
}

workerMethods.getPagePixmap = function (page) {
    libvau._wasm_get_page_pixmap (page);
    return VAUJSPIXMAP;
};

workerMethods.evalScheme = function (str) {
	var p= allocateUTF8(str);
	libvau._wasm_eval(p);
	libvau._free(p);
};

var vau_ready = libvau({}).then(m => {
	libvau = m;
	libvau._wasm_init_vau();

	if (!globalThis.crossOriginIsolated) {
		console.warn("Vau: The current page is running in a non-isolated context. This means SharedArrayBuffer is not available. See https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/SharedArrayBuffer for details.");
		return { sharedBuffer: null };
	}
	if (globalThis.SharedArrayBuffer == null) {
		console.warn("Vau: You browser does not implement SharedArrayBuffer.");
		return { sharedBuffer: null };
	}
/*    
	if (libvau.wasmMemory == null) {
        console.error(libvau);
		console.error("Vau internal error: emscripten does not export wasmMemory");
		return { sharedBuffer: null };
	}
	if (!(libvau.wasmMemory instanceof WebAssembly.Memory) || !(libvau.wasmMemory.buffer instanceof SharedArrayBuffer)) {
		console.error("Vau internal error: wasmMemory exported by emscripten is not a valid instance of WebAssembly.Memory");
		return { sharedBuffer: null };
	}
	console.log("Vau: WASM module running in cross-origin isolated context")
	return { sharedBuffer: libvau.wasmMemory.buffer }
    */
	return { sharedBuffer: null };
});

vau_ready
	.then(result => postMessage(["READY", result.sharedBuffer, Object.keys(workerMethods)]))
	.catch(error => postMessage(["ERROR", error]));

