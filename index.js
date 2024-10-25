const ndi = require('bindings')('ndi.node');

class Receiver {

    finder = new ndi.Finder()
    recv = null

    constructor(name) {
        this.name = name
    }

    search() {
        let match = this.finder.find().filter((o, i) => o.name.includes(this.name)).pop()
        if (match) {
            this.recv = new ndi.Receiver(match)
            console.log("found NDI source", this.name)
        }
    }

    video() {
        if (this.recv) {
            return this.recv.video(...arguments)
        } else {
            this.search()
        }
    }

    video_into() {
        if (this.recv) {
            return this.recv.video_into(...arguments)
        } else {
            this.search()
        }
    }
}

module.exports = {
    Sender: ndi.Sender,
    Finder: ndi.Finder,
    Receiver
}