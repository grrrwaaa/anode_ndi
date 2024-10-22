const ndi = require('bindings')('ndi.node');

class Receiver {

    finder = new ndi.Finder()
    recv = null

    constructor(name) {
        this.name = name
    }

    video() {
        if (this.recv) {
            return this.recv.video()
        } else {
            let match = this.finder.find().filter((o, i) => o.name.includes(this.name)).pop()
            if (match) {
                this.recv = new ndi.Receiver(match)
                console.log("found NDI source", this.name)
            }
        }
    }
}

module.exports = {
    Sender: ndi.Sender,
    Finder: ndi.Finder,
    Receiver
}