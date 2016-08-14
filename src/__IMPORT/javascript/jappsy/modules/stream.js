Jappsy.Stream = function(data) {
	if (typeof data == 'string'){
		data = Jappsy.binstring2buf(data);
	} else {
		var type = toString.call(data).slice(8, -1);
		if (type == 'ArrayBuffer') data = new Uint8Array(data);
	}

	this.position = 0;
	this.data = data;
};

Jappsy.Stream.equalBytes = function(a, b) {
	if (a.length != b.length) return false;
	for (var l = a.length; l--;) if (a[l] != b[l]) return false;
	return true;
};

Jappsy.Stream.readUInt32 = function(buffer, offset) {
	return
		(buffer[offset] << 24) +
		(buffer[offset + 1] << 16) +
		(buffer[offset + 2] << 8) +
		(buffer[offset + 3] << 0);
};

Jappsy.Stream.readUInt16 = function(buffer, offset) {
	return (buffer[offset + 1] << 8) + (buffer[offset] << 0);
};

Jappsy.Stream.readUInt8 = function(buffer, offset) {
	return buffer[offset] << 0;
};

Jappsy.Stream.prototype.readBytes = function(length) {
	var end = this.position + length;
	if (end > this.data.length)
		throw new Error('Unexpected end of file');
	//var data = slice.call(this.data, this.position, end);
	var data = this.data.subarray(this.position, end);
	this.position = end;
	return data;
};

Jappsy.Stream.prototype.readInt = function() {
	var end = this.position + 4;
	if (end > this.data.length)
		throw new Error('Unexpected end of file');
	var value =
		(this.data[this.position+3] << 24) +
		(this.data[this.position+2] << 16) +
		(this.data[this.position+1] << 8) +
		(this.data[this.position] << 0);
	this.position = end;
	return value;
};

Jappsy.Stream.prototype.readUnsignedByte = function() {
	var end = this.position + 1;
	if (end > this.data.length)
		throw new Error('Unexpected end of file');
	var value = this.data[this.position];
	this.position = end;
	return value;
};

Jappsy.Stream.prototype.skip = function(length) {
	var end = this.position + length;
	if (end > this.data.length)
		return -1;
	this.position = end;
	return length;
};

//===============================

Jappsy.Float32Stream = function(size) {
	this.position = 0;
	this.data = new Float32Array(size);
};

Jappsy.Float32Stream.prototype.write = function(value1, value2, value3) {
	if ((value1 !== undefined) && ((this.position + 1) <= this.data.length)) {
		this.data[this.position] = value1;
		this.position++;
	}
	if ((value2 !== undefined) && ((this.position + 1) <= this.data.length)) {
		this.data[this.position] = value2;
		this.position++;
	}
	if ((value3 !== undefined) && ((this.position + 1) <= this.data.length)) {
		this.data[this.position] = value3;
		this.position++;
	}
};

//===============================

Jappsy.Uint16Stream = function(size) {
	this.position = 0;
	this.data = new Uint16Array(size);
};

Jappsy.Uint16Stream.prototype.write = function(value1, value2, value3) {
	if ((value1 !== undefined) && ((this.position + 1) <= this.data.length)) {
		this.data[this.position] = value1;
		this.position++;
	}
	if ((value2 !== undefined) && ((this.position + 1) <= this.data.length)) {
		this.data[this.position] = value2;
		this.position++;
	}
	if ((value3 !== undefined) && ((this.position + 1) <= this.data.length)) {
		this.data[this.position] = value3;
		this.position++;
	}
};
