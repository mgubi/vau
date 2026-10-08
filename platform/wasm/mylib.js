// The functions of JavaScript which the library calls (see vau_lib.cpp)

mergeInto(LibraryManager.library, {
    vaujs_alert: function (x) {
        console.log(`ALERT ${x}`);
    },
    // a copy of the RGBA samples of the picture just drawn, kept in the
    // module as { data, width, height } until it is asked for
    vaujs_set_pixmap: function (p, s, w, h) {
        Module.vauPixmap = {
            data: new Uint8ClampedArray(HEAPU8.buffer, p, s).slice(),
            width: w,
            height: h
        };
    }
});
