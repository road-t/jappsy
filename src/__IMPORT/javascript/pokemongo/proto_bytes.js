function BytesKey(wire_type, field_number) {
	this.wire_type = wire_type;
	this.field_number = field_number;
};

function Bytes(size) {
	this.pageSize = 256;
	this.length = size || 0;
	this.pages = Math.floor(Math.max(this.length - 1, 0) / this.pageSize) + 1;
	this.buffer = new ArrayBuffer(this.pages * this.pageSize);
	this.view = new Uint8Array(this.buffer);
	this.position = 0;
	this.variant = new UInt64();
};

Bytes.types = {
	Variant: 0,
	Fixed64: 1,
	Packed: 2,
	Fixed32: 5,
};

Bytes.prototype.resize = function(size) {
	this.length = size || 0;
	var newPages = Math.floor(Math.max(this.length - 1, 0) / this.pageSize) + 1;
	if (this.pages != newPages) {
		var newBuffer = new ArrayBuffer(newPages * this.pageSize);
		var newView = new Uint8Array(newBuffer);
		if (newView.length >= this.view.length) {
			newView.set(this.view, 0);
		} else {
			newView.set(this.view.subarray(0, newView.length), 0);
		}
		this.pages = newPages;
		this.buffer = newBuffer;
		this.view = newView;
	}
};

Bytes.prototype.writeByte = function(value) {
	var p = this.position + 1;
	if (p > this.length)
		this.resize(p);
	this.view[this.position] = value;
	this.position++;
};

Bytes.prototype.writeBuffer = function(buffer) {
	var p = this.position + buffer.length;
	if (p > this.length)
		this.resize(p);
	this.view.set(buffer, this.position);
	this.position = p;
};

Bytes.prototype.writeBytes = function(stream) {
	var p = this.position + stream.length;
	if (p > this.length)
		this.resize(p);
	this.view.set(stream.view, this.position);
	this.position = p;
};

Bytes.prototype.readByte = function() {
	if (this.position < this.length) {
		var p = this.position;
		this.position++;
		return this.view[p];
	}
	return 0;
};

Bytes.prototype.readBuffer = function(buffer, length) {
	var end = this.position + length;
	if (end > this.length) end = this.length;
	if (end > this.position) {
		buffer.set(this.view.subarray(this.position, end), 0);
		this.position = end;
	}
	return buffer;
};

Bytes.prototype.readBytes = function(length) {
	var stream = new Bytes(length);
	var end = this.position + length;
	if (end > this.length) end = this.length;
	if (end > this.position) {
		stream.view.set(this.view.subarray(this.position, end), 0);
		this.position = end;
	}
	return stream;
};

Bytes.prototype.skip = function(amount) {
	this.position += amount;
};

// Input Uint8Array or Integer
Bytes.prototype.writeVariant = function(value) {
	var count = 0;
	
	if (value instanceof Uint8Array) {
		this.variant.value[0] = this.variant.value[1] = 0;
		this.variant.view.set(value);
		do {
			this.variant.shr(7);
			count++;
		} while ((this.variant.value[0] != 0) || (this.variant.value[1] != 0));
		
		var p = this.position + count;
		if (p > this.length)
			this.resize(p);

		this.variant.view.set(value);
		while (true) {
			var bits = this.variant.value[0] & 0x7F;
			this.variant.shr(7);
			if ((this.variant.value[0] != 0) || (this.variant.value[1] != 0)) {
				this.writeByte(bits | 0x80);
			} else {
				this.writeByte(bits);
				break;
			}
		}
	} else {
		var v = value;
		do {
			v = v >>> 7;
			count++;
		} while (v != 0);
		
		var p = this.position + count;
		if (p > this.length)
			this.resize(p);
		
		v = value;
		do {
			var bits = v & 0x7F;
			v = v >>> 7;
			if (v != 0)
				this.writeByte(bits | 0x80);
			else
				this.writeByte(bits);
		} while (v != 0);
	}
};

// Output UInt64
Bytes.prototype.readVariant = function() {
	var v = 0;
	var bits = 0;
	var buffer = [];
	do {
		bits = this.readByte();
		buffer.push(bits & 0x7F);
	} while ((bits & 0x80) != 0);
	if (buffer.length < 5) {
		var i = buffer.length - 1;
		this.variant.value[0] = buffer[i];
		this.variant.value[1] = 0;
		i--;
		while (i >= 0) {
			this.variant.value[0] <<= 7;
			this.variant.value[0] |= buffer[i];
			i--;
		}
	} else {
		var i = buffer.length - 1;
		this.variant.value[0] = buffer[i];
		this.variant.value[1] = 0;
		i--;
		while (i >= 0) {
			this.variant.shl(7);
			this.variant.or(buffer[i]);
			i--;
		}
	}
	return this.variant;
};

// Input Uint8Array or Integer
Bytes.prototype.writeVariantZigZag = function(value) {
	var count = 0;
	
	if (value instanceof Uint8Array) {
		this.variant.value[0] = this.variant.value[1] = 0;
		this.variant.view.set(value);
		this.variant.rol(1);
		if ((this.variant.value[0] & 1) != 0) {
			this.variant.value[0] ^= 0xFFFFFFFE;
			this.variant.value[1] ^= 0xFFFFFFFF;
		}
		do {
			this.variant.shr(7);
			count++;
		} while ((this.variant.value[0] != 0) || (this.variant.value[1] != 0));
		
		var p = this.position + count;
		if (p > this.length)
			this.resize(p);

		this.variant.view.set(value);
		this.variant.rol(1);
		if ((this.variant.value[0] & 1) != 0) {
			this.variant.value[0] ^= 0xFFFFFFFE;
			this.variant.value[1] ^= 0xFFFFFFFF;
		}
		while (true) {
			var bits = this.variant.value[0] & 0x7F;
			this.variant.shr(7);
			if ((this.variant.value[0] != 0) || (this.variant.value[1] != 0)) {
				this.writeByte(bits | 0x80);
			} else {
				this.writeByte(bits);
				break;
			}
		}
	} else {
		var v = (value << 1) | (value >>> 31);
		if ((v & 1) != 0)
			v ^= 0xFFFFFFFE;
		if (v < 0)
			v += 0x100000000;
		
		do {
			v = v >>> 7;
			count++;
		} while (v != 0);
		
		var p = this.position + count;
		if (p > this.length)
			this.resize(p);
		
		v = (value << 1) | (value >>> 31);
		if ((v & 1) != 0)
			v ^= 0xFFFFFFFE;
		if (v < 0)
			v += 0x100000000;
		do {
			var bits = v & 0x7F;
			v = v >>> 7;
			if (v != 0)
				this.writeByte(bits | 0x80);
			else
				this.writeByte(bits);
		} while (v != 0);
	}
};

// Output UInt64
Bytes.prototype.readVariantZigZag = function(value) {
	var v = this.readVariant();
	if ((v.value[0] & 1) != 0) {
		v.value[0] ^= 0xFFFFFFFE;
		v.value[1] ^= 0xFFFFFFFF;
	}
	v.ror(1);
	return v;
};

Bytes.prototype.writeKey = function(key) {
	if (key instanceof BytesKey) {
		var v = new UInt64(key.field_number);
		v.shl(3);
		v.or(key.wire_type);
		this.writeVariant(v);
	}
};

Bytes.prototype.readKey = function() {
	var v = this.readVariant();
	return new BytesKey(v.value[0] & 0x7, v.shr(3).value[0]);
};

Bytes.prototype.skipKeyValue = function(key) {
	if (key.wire_type == Bytes.types.Variant) {
		var bits = 0;
		do {
			bits = this.readByte();
		} while ((bits & 0x80) != 0);
	} else if (key.wire_type == Bytes.types.Fixed64) {
		this.skip(8);
	} else if (key.wire_type == Bytes.types.Packed) {
		var v = this.readVariant();
		this.skip(v.value[0]);
	} else if (key.wire_type == Bytes.types.Fixed32) {
		this.skip(4);
	} else {
		throw 'Stream error: Unsupported wire type';
	}
};

Bytes.prototype.wireType = function() {
	return Bytes.types.Packed;
};

Bytes.prototype.write = function(stream) {
	if (this.length > 0) {
		stream.writeVariant(this.length);
		stream.writeBuffer(this.view.subarray(0, this.length));
	} else {
		stream.writeByte(0);
	}
};

Bytes.prototype.read = function(stream) {
	var v = stream.readVariant();
	if ((v.value[1] != 0) || (v.value[0] > 1024*1024*10)) {
		throw 'Stream error: Bytes to long!';
	}
	if (v.value[0] == 0) {
		this.resize(0);
	} else {
		this.resize(v.value[0]);
		stream.readBuffer(this.view, v.value[0]);
	}
	this.position = 0;
};
