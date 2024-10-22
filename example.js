let ndi = require("./index.js")

let name = "example"

// create a sender with a given name
let sender = new ndi.Sender(name)
console.log(sender)

// minimum size is 2x2
let w = 2
let h = 2
let data = new Uint8Array(w*h*4)

// sending frames at a very low rate for debugging purposes:
setInterval(function() {
    // randomized frame:
    for (i=0; i<data.length; i+=4) {
        data[i+0] = Math.random()*255
        data[i+1] = Math.random()*255
        data[i+2] = Math.random()*255
        data[i+3] = 255
    }
    sender.send(data, w, h)
}, 500)

// show current sources
// Note, it can take some time for the network to reveal all sources, so normally you wouldn't want to call find() right after creating a Finder
//let finder = new ndi.Finder()
//console.log(finder.find())

// names are matched to sources using String.includes() 
// it may take some time before the receiver finds a matching source on the network, but it won't slow down the script
// calling .video() before the source is found will just return null
let receiver = new ndi.Receiver(name)
console.log(receiver)

setInterval(function() {
    // it won't wait. If there's no new video frame, it returns null
    let frame = receiver.video()
    // frame has xres, yres, bytelength, and data (Uint8Array)
    if (frame) console.log("got frame", frame.bytelength, frame)

}, 30) 
