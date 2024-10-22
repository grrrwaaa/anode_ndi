const ndi = require('bindings')('ndi.node');

class Receiver {

    finder = new ndi.Finder()   

    constructor(name) {
        this.name = name
    }

    video() {
        if (this.receiver) {
            return this.receiver.video()
        } else {
            let match = this.finder.find().filter((o, i) => o.name.includes(this.name)).pop()
            if (match) {
                this.receiver = new ndi.Receiver(match)
                console.log("found NDI source", this.name, this.receiver)
            }
        }
    }
}

module.exports = {
    Sender: ndi.Sender,
    Finder: ndi.Finder,
    Receiver
}