
mergeInto(LibraryManager.library, {
    vaujs_alert: function(x)  {
        console.log(`ALERT ${x}`);
         },
    vaujs_set_pixmap: function(p,s,w,h) {
        //console.log(`SET PIXMAP ${p} ${s} ${w} ${h}`);
        var pixArray= new Uint8ClampedArray(HEAPU8.buffer, p, s).slice();
        let imageData = new ImageData(pixArray, w, h);
        VAUJSPIXMAP=  imageData;
    } 
}); 



