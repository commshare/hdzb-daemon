const HEADER_SIZE = 3 * 4;

function Packet() {
  this.header = new Buffer(HEADER_SIZE);
  this.body;

  this.headerOffset = 0;
  this.bodyOffset = 0;
}

Packet.prototype.reset = function() {
  this.headerOffset = this.bodyOffset = 0;
}

Packet.prototype.getHeader = function() {
  return this.header;
}

Packet.prototype.getBody = function() {
  return this.body;
}

Packet.prototype.parse = function(data) {
  var header = this.header;
  // console.log(header);
  var offset = 0;

  if (this.headerOffset !== HEADER_SIZE) { //header
    // console.log('parse header')
    var spaceSize = HEADER_SIZE - this.headerOffset;
    var writeSize = data.length > spaceSize ? spaceSize : data.length;
    data.copy(header, this.headerOffset, offset, offset + writeSize);
    this.headerOffset += writeSize;
    offset += writeSize;

    // console.log({
    //   headerOffset: this.headerOffset,
    //   offset: offset
    // })

    if (this.headerOffset === HEADER_SIZE) {
      header.width = header.readInt32LE();
      header.height = header.readInt32LE(4);
      header.size = header.readInt32LE(8);

      // this.body = new Buffer(header.size);

      if (!this.body || this.body.length < header.size) {
        console.log({
          w: header.width,
          h: header.height,
          s: header.size
        });
        this.body = new Buffer(header.size);
      }

    }
  }

  if (this.headerOffset === HEADER_SIZE) { //body
    if (this.bodyOffset !== header.size) {
      // console.log('parse body')
      var spaceSize = header.size - this.bodyOffset;
      var writeSize = (data.length - offset) > spaceSize ? spaceSize : (data.length -
        offset);
      // console.log({
      //   writeSize: writeSize
      // })
      data.copy(this.body, this.bodyOffset, offset, offset + writeSize);

      offset += writeSize;
      this.bodyOffset += writeSize;
      // console.log({
      //   bodyOffset: this.bodyOffset,
      //   offset: offset
      // });
    }
  }

  return offset;
}

Packet.prototype.printHeader = function() {
  console.log('w: ' + this.header.width + ' h: ' + this.header.height +
    ' size: ' + this.header.size);
}

Packet.prototype.isFull = function() {
  return this.headerOffset === HEADER_SIZE && this.bodyOffset ===
    this.header.size;
}

module.exports = Packet;
