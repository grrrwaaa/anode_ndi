let ndi = require("./index.js")

let name = "example"

// create a sender with a given name
let sender = new ndi.Sender(name)
console.log(sender)

let w = 16
let h = 10
let data = new Uint8Array(w*h*4)

setInterval(function() {
        for (i=0; i<data.length; i++) data[i] = Math.random()*255
        sender.send(data, w, h)
}, 300)


//let finder = new ndi.Finder()
//console.log(finder.find())

let receiver = new ndi.Receiver(name)

setInterval(function() {
    
    let frame = receiver.video()
    if (frame) console.log("got frame", frame.bytelength)

}, 30) 
