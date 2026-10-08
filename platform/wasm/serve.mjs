// A static server for the page of Vau, for tests on this machine:
//
//   node serve.mjs [port] [directory]
//
// (the files of a WebAssembly build have to come from a server, with the
// right type for the .wasm file)

import http from "node:http";
import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

const port = Number(process.argv[2] || 8080);
const root = path.resolve(process.argv[3] || path.dirname(fileURLToPath(import.meta.url)));
const types = {
	".html": "text/html; charset=utf-8", ".js": "text/javascript",
	".mjs": "text/javascript", ".wasm": "application/wasm",
	".data": "application/octet-stream", ".json": "application/json",
	".png": "image/png", ".tm": "text/plain; charset=utf-8"
};

http.createServer((req, res) => {
	let rel = decodeURIComponent(new URL(req.url, "http://localhost").pathname);
	if (rel === "/") rel = "/Vau.html";
	const file = path.join(root, rel);
	if (!file.startsWith(root + path.sep)) { res.writeHead(403).end(); return; }
	fs.stat(file, (err, st) => {
		if (err || !st.isFile()) { res.writeHead(404).end("not found"); return; }
		res.writeHead(200, {
			"Content-Type": types[path.extname(file)] || "application/octet-stream",
			"Content-Length": st.size,
			"Cache-Control": "no-cache"
		});
		fs.createReadStream(file).pipe(res);
	});
}).listen(port, "127.0.0.1", () => {
	console.log(`Vau: http://localhost:${port}/ (serving ${root})`);
});
