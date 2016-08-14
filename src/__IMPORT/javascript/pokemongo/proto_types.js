function UInt64(lo, hi) {
	this.buffer = new ArrayBuffer(8);
	this.view = new Uint8Array(this.buffer);
	this.value = new Uint32Array(this.buffer);

	if ((lo instanceof UInt64) || (lo instanceof Int64) || (lo instanceof SInt64) || (lo instanceof Fixed64) || (lo instanceof SFixed64) || (lo instanceof Double)) {
		this.view.set(lo.view);
	} else if ((lo instanceof UInt32) || (lo instanceof Int32) || (lo instanceof SInt32) || (lo instanceof Fixed32) || (lo instanceof SFixed32) || (lo instanceof Float)) {
		this.view.set(lo.view);
		this.value[1] = 0;
	} else if ((lo instanceof Enum) || (lo instanceof Bool)) {
		this.value[0] = lo.valueOf();
		this.value[1] = 0;
	} else if (lo instanceof Uint8Array) {
		this.value[0] = this.value[1] = 0;
		this.view.set(lo);
	} else if ((lo !== undefined) && (hi !== undefined)) {
		this.value[0] = lo;
		this.value[1] = hi;
	} else if (lo !== undefined) {
		this.value[0] = lo;
		this.value[1] = 0;
	} else {
		this.value[0] = this.value[1] = 0;
	}
};

UInt64.prototype.wireType = function() {
	return Bytes.types.Variant;
};

UInt64.mask = [
	0x00000000,
	0x00000001, 0x00000003, 0x00000007, 0x0000000F,
	0x0000001F, 0x0000003F, 0x0000007F, 0x000000FF,
	0x000001FF, 0x000003FF, 0x000007FF, 0x00000FFF,
	0x00001FFF, 0x00003FFF, 0x00007FFF, 0x0000FFFF,
	0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 0x000FFFFF,
	0x001FFFFF, 0x003FFFFF, 0x007FFFFF, 0x00FFFFFF,
	0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF,
	0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF
];

UInt64.prototype.shr = function(bits) {
	if (bits > 0) {
		if (bits < 32) {
			this.value[0] = (this.value[0] >>> bits) | ((this.value[1] & UInt64.mask[bits]) << (32 - bits));
			this.value[1] = this.value[1] >>> bits;
		} else if (bits == 32) {
			this.value[0] = this.hi;
			this.value[1] = 0;
		} else if (bits < 64) {
			this.value[0] = this.value[1] >>> (bits - 32);
			this.value[1] = 0;
		} else {
			this.value[0] = this.value[1] = 0;
		}
	} else {
		return this.shl(-bits);
	}
	
	return this;
};

UInt64.prototype.shl = function(bits) {
	if (bits > 0) {
		if (bits < 32) {
			this.value[1] = ((this.value[1] & UInt64.mask[32 - bits]) << bits) | (this.value[0] >>> (32 - bits));
			this.value[0] = this.value[0] << bits;
		} else if (bits == 32) {
			this.value[1] = this.value[0];
			this.value[0] = 0;
		} else if (bits < 64) {
			this.value[1] = this.value[0] << (bits - 32);
			this.value[0] = 0;
		} else {
			this.value[0] = this.value[1] = 0;
		}
	} else {
		return this.shr(-bits);
	}
	
	return this;
};

UInt64.prototype.rol = function(bits) {
	while (bits > 64) {
		bits -= 64;
	}
	while (bits < 0) {
		bits += 64;
	}
	if (bits == 0) {
		return this;
	} else {
		if (bits > 32) {
			var tmp = this.value[1];
			this.value[1] = this.value[0];
			this.value[0] = tmp;
			bits -= 32;
		}
		if (bits == 0) {
			return this;
		} else {
			var lo = this.value[1] >>> (32 - bits);
			var hi = this.value[0] >>> (32 - bits);
			this.value[0] = (this.value[0] << bits) | lo;
			this.value[1] = (this.value[1] << bits) | hi;
		}
	}
	return this;
};

UInt64.prototype.ror = function(bits) {
	while (bits > 64) {
		bits -= 64;
	}
	while (bits < 0) {
		bits += 64;
	}
	if (bits == 0) {
		return this;
	} else {
		if (bits > 32) {
			var tmp = this.hi;
			this.value[1] = this.value[0];
			this.value[0] = tmp;
			bits -= 32;
		}
		if (bits == 0) {
			return this;
		} else {
			var lo = (this.value[1] & UInt64.mask[bits]) << (32 - bits);
			var hi = (this.value[0] & UInt64.mask[bits]) << (32 - bits);
			this.value[0] = (this.value[0] >>> bits) | lo;
			this.value[1] = (this.value[1] >>> bits) | hi;
		}
	}
	return this;
};

UInt64.prototype.or = function(value) {
	if (value instanceof UInt64) {
		this.value[1] |= value.value[1];
		this.value[0] |= value.value[0];
	} else {
		this.value[0] |= value;
	}
};

UInt64.prototype.valueOf = function() {
	if (this.value[1] == 0)
		return this.value[0];
	return this;
};

UInt64.prototype.toString = function() {
	return "0x" + ("00000000" + this.value[1].toString(16)).substr(-8) + ("00000000" + this.value[0].toString(16)).substr(-8);
};

UInt64.prototype.write = function(stream) {
	stream.writeVariant(this.view);
};

UInt64.prototype.read = function(stream) {
	this.value.set(stream.readVariant().value);
};

//===================================================

Int64 = function(lo, hi) {
	this.buffer = new ArrayBuffer(8);
	this.view = new Uint8Array(this.buffer);
	this.value = new Uint32Array(this.buffer);

	if ((lo instanceof UInt64) || (lo instanceof Int64) || (lo instanceof SInt64) || (lo instanceof Fixed64) || (lo instanceof SFixed64) || (lo instanceof Double)) {
		this.view.set(lo.view);
	} else if ((lo instanceof UInt32) || (lo instanceof Int32) || (lo instanceof SInt32) || (lo instanceof Fixed32) || (lo instanceof SFixed32) || (lo instanceof Float)) {
		this.view.set(lo.view);
		this.value[1] = 0;
	} else if ((lo instanceof Enum) || (lo instanceof Bool)) {
		this.value[0] = lo.valueOf();
		this.value[1] = 0;
	} else if ((lo !== undefined) && (hi !== undefined)) {
		this.value[0] = lo;
		this.value[1] = hi;
	} else if (lo !== undefined) {
		this.value[0] = lo;
		this.value[1] = 0;
	} else {
		this.value[0] = this.value[1] = 0;
	}
};

Int64.prototype.wireType = function() {
	return Bytes.types.Variant;
};

Int64.prototype.valueOf = function() {
	if ((this.value[1] == 0) && ((this.value[0] & 0x80000000) == 0))
		return this.value[0];
	else if ((this.value[1] == 0xFFFFFFFF) && ((this.value[0] & 0x80000000) != 0))
		return this.value[0];
	return this;
};

Int64.prototype.toString = function() {
	return "0x" + ("00000000" + this.value[1].toString(16)).substr(-8) + ("00000000" + this.value[0].toString(16)).substr(-8);
};

Int64.prototype.write = function(stream) {
	stream.writeVariant(this.view);
};

Int64.prototype.read = function(stream) {
	this.value.set(stream.readVariant().value);
};

//===================================================

SInt64 = function(lo, hi) {
	this.buffer = new ArrayBuffer(8);
	this.view = new Uint8Array(this.buffer);
	this.value = new Uint32Array(this.buffer);

	if ((lo instanceof UInt64) || (lo instanceof Int64) || (lo instanceof SInt64) || (lo instanceof Fixed64) || (lo instanceof SFixed64) || (lo instanceof Double)) {
		this.view.set(lo.view);
	} else if ((lo instanceof UInt32) || (lo instanceof Int32) || (lo instanceof SInt32) || (lo instanceof Fixed32) || (lo instanceof SFixed32) || (lo instanceof Float)) {
		this.view.set(lo.view);
		this.value[1] = 0;
	} else if ((lo instanceof Enum) || (lo instanceof Bool)) {
		this.value[0] = lo.valueOf();
		this.value[1] = 0;
	} else if ((lo !== undefined) && (hi !== undefined)) {
		this.value[0] = lo;
		this.value[1] = hi;
	} else if (lo !== undefined) {
		this.value[0] = lo;
		this.value[1] = 0;
	} else {
		this.value[0] = this.value[1] = 0;
	}
};

SInt64.prototype.wireType = function() {
	return Bytes.types.Variant;
};

SInt64.prototype.valueOf = function() {
	if ((this.value[1] == 0) && ((this.value[0] & 0x80000000) == 0))
		return this.value[0];
	else if ((this.value[1] == 0xFFFFFFFF) && ((this.value[0] & 0x80000000) != 0))
		return this.value[0];
	return this;
};

SInt64.prototype.toString = function() {
	return "0x" + ("00000000" + this.value[1].toString(16)).substr(-8) + ("00000000" + this.value[0].toString(16)).substr(-8);
};

SInt64.prototype.write = function(stream) {
	stream.writeVariantZigZag(this.view);
};

SInt64.prototype.read = function(stream) {
	this.value.set(stream.readVariantZigZag().value);
};

//===================================================

Fixed64 = function(lo, hi) {
	this.buffer = new ArrayBuffer(8);
	this.view = new Uint8Array(this.buffer);
	this.value = new Uint32Array(this.buffer);

	if ((lo instanceof UInt64) || (lo instanceof Int64) || (lo instanceof SInt64) || (lo instanceof Fixed64) || (lo instanceof SFixed64) || (lo instanceof Double)) {
		this.view.set(lo.view);
	} else if ((lo instanceof UInt32) || (lo instanceof Int32) || (lo instanceof SInt32) || (lo instanceof Fixed32) || (lo instanceof SFixed32) || (lo instanceof Float)) {
		this.view.set(lo.view);
		this.value[1] = 0;
	} else if ((lo instanceof Enum) || (lo instanceof Bool)) {
		this.value[0] = lo.valueOf();
		this.value[1] = 0;
	} else if ((lo !== undefined) && (hi !== undefined)) {
		this.value[0] = lo;
		this.value[1] = hi;
	} else if (lo !== undefined) {
		this.value[0] = lo;
		this.value[1] = 0;
	} else {
		this.value[0] = this.value[1] = 0;
	}
};

Fixed64.prototype.wireType = function() {
	return Bytes.types.Fixed64;
};

Fixed64.prototype.valueOf = function() {
	if (this.value[1] == 0)
		return this.value[0];
	return this;
};

Fixed64.prototype.toString = function() {
	return "0x" + ("00000000" + this.value[1].toString(16)).substr(-8) + ("00000000" + this.value[0].toString(16)).substr(-8);
};

Fixed64.prototype.write = function(stream) {
	stream.writeBuffer(this.view);
};

Fixed64.prototype.read = function(stream) {
	stream.readBuffer(this.view, 8);
};

//===================================================

SFixed64 = function(lo, hi) {
	this.buffer = new ArrayBuffer(8);
	this.view = new Uint8Array(this.buffer);
	this.value = new Uint32Array(this.buffer);

	if ((lo instanceof UInt64) || (lo instanceof Int64) || (lo instanceof SInt64) || (lo instanceof Fixed64) || (lo instanceof SFixed64) || (lo instanceof Double)) {
		this.view.set(lo.view);
	} else if ((lo instanceof UInt32) || (lo instanceof Int32) || (lo instanceof SInt32) || (lo instanceof Fixed32) || (lo instanceof SFixed32) || (lo instanceof Float)) {
		this.view.set(lo.view);
		this.value[1] = 0;
	} else if ((lo instanceof Enum) || (lo instanceof Bool)) {
		this.value[0] = lo.valueOf();
		this.value[1] = 0;
	} else if ((lo !== undefined) && (hi !== undefined)) {
		this.value[0] = lo;
		this.value[1] = hi;
	} else if (lo !== undefined) {
		this.value[0] = lo;
		this.value[1] = 0;
	} else {
		this.value[0] = this.value[1] = 0;
	}
};

SFixed64.prototype.wireType = function() {
	return Bytes.types.Fixed64;
};

SFixed64.prototype.valueOf = function() {
	if ((this.value[1] == 0) && ((this.value[0] & 0x80000000) == 0))
		return this.value[0];
	else if ((this.value[1] == 0xFFFFFFFF) && ((this.value[0] & 0x80000000) != 0))
		return this.value[0];
	return this;
};

SFixed64.prototype.toString = function() {
	return "0x" + ("00000000" + this.value[1].toString(16)).substr(-8) + ("00000000" + this.value[0].toString(16)).substr(-8);
};

SFixed64.prototype.write = function(stream) {
	stream.writeBuffer(this.view);
};

SFixed64.prototype.read = function(stream) {
	stream.readBuffer(this.view, 8);
};

//===================================================

Double = function(v) {
	this.buffer = new ArrayBuffer(8);
	this.view = new Uint8Array(this.buffer);
	this.value = new Float64Array(this.buffer);

	if ((v instanceof UInt64) || (v instanceof Fixed64)) {
		this.value[0] = v.value[1];
		this.value[0] *= 0x100000000;
		this.value[0] += v.value[0];
	} else if ((v instanceof Int64) || (v instanceof SInt64) || (v instanceof SFixed64)) {
		if ((v.value[1] & 0x80000000) != 0) {
			var vh = v.value[1] ^ 0xFFFFFFFF;
			var vl = v.value[0] ^ 0xFFFFFFFF;
			if (vh < 0)	vh += 0x100000000;
			if (vl < 0) vl += 0x100000000;
			this.value[0] = vh;
			this.value[0] *= 0x100000000;
			this.value[0] += vl;
			this.value[0] += 1;
			this.value[0] = -this.value[0];
		} else {
			this.value[0] = v.value[1];
			this.value[0] *= 0x100000000;
			this.value[0] += v.value[0];
		}
	} else if ((v instanceof UInt32) || (v instanceof Fixed32) || (v instanceof Double) || (v instanceof Float)) {
		this.value[0] = v.value[0];
	} else if ((v instanceof Int32) || (v instanceof SInt32) || (v instanceof SFixed32)) {
		if ((v.value[0] & 0x80000000) != 0) {
			var vl = v.value[0] ^ 0xFFFFFFFF;
			if (vl < 0) vl += 0x100000000;
			this.value[0] = vl;
			this.value[0] += 1;
			this.value[0] = -this.value[0];
		} else {
			this.value[0] = v.value[0];
		}
	} else if ((v instanceof Enum) || (v instanceof Bool)) {
		this.value[0] = v.valueOf();
	} else if (v !== undefined) {
		this.value[0] = v;
	} else {
		this.value[0] = 0;
	}
};

Double.prototype.wireType = function() {
	return Bytes.types.Fixed64;
};

Double.prototype.valueOf = function() {
	return this.value[0];
};

Double.prototype.toString = function() {
	return this.value[0].toString();
};

Double.prototype.write = function(stream) {
	stream.writeBuffer(this.view);
};

Double.prototype.read = function(stream) {
	stream.readBuffer(this.view, 8);
};

//===================================================

UInt32 = function(v) {
	this.buffer = new ArrayBuffer(4);
	this.view = new Uint8Array(this.buffer);
	this.value = new Uint32Array(this.buffer);

	if ((v instanceof UInt64) || (v instanceof Int64) || (v instanceof SInt64) || (v instanceof Fixed64) || (v instanceof SFixed64) ||
		(v instanceof UInt32) || (v instanceof Int32) || (v instanceof SInt32) || (v instanceof Fixed32) || (v instanceof SFixed32)) {
		this.value[0] = v.value[0];
	} else if ((v instanceof Double) || (v instanceof Float) ||	(v instanceof Enum) || (v instanceof Bool)) {
		this.value[0] = v.valueOf();
	} else if (v !== undefined) {
		this.value[0] = v;
	} else {
		this.value[0] = 0;
	}
};

UInt32.prototype.wireType = function() {
	return Bytes.types.Variant;
};

UInt32.prototype.valueOf = function() {
	return this.value[0];
};

UInt32.prototype.toString = function() {
	return this.value[0].toString(arguments[0] || 10);
};

UInt32.prototype.write = function(stream) {
	stream.writeVariant(this.view);
};

UInt32.prototype.read = function(stream) {
	var v = stream.readVariant();
	this.value[0] = v.value[0];
};

//===================================================

Int32 = function(v) {
	this.buffer = new ArrayBuffer(4);
	this.view = new Uint8Array(this.buffer);
	this.value = new Int32Array(this.buffer);

	if ((v instanceof UInt64) || (v instanceof Int64) || (v instanceof SInt64) || (v instanceof Fixed64) || (v instanceof SFixed64) ||
		(v instanceof UInt32) || (v instanceof Int32) || (v instanceof SInt32) || (v instanceof Fixed32) || (v instanceof SFixed32)) {
		this.value[0] = v.value[0];
	} else if ((v instanceof Double) || (v instanceof Float) ||	(v instanceof Enum) || (v instanceof Bool)) {
		this.value[0] = v.valueOf();
	} else if (v !== undefined) {
		this.value[0] = v;
	} else {
		this.value[0] = 0;
	}
};

Int32.prototype.wireType = function() {
	return Bytes.types.Variant;
};

Int32.prototype.valueOf = function() {
	return this.value[0];
};

Int32.prototype.toString = function() {
	return this.value[0].toString(arguments[0] || 10);
};

Int32.prototype.write = function(stream) {
	stream.writeVariant(this.view);
};

Int32.prototype.read = function(stream) {
	var v = stream.readVariant();
	this.value[0] = v.value[0];
};

//===================================================

SInt32 = function(v) {
	this.buffer = new ArrayBuffer(4);
	this.view = new Uint8Array(this.buffer);
	this.value = new Int32Array(this.buffer);

	if ((v instanceof UInt64) || (v instanceof Int64) || (v instanceof SInt64) || (v instanceof Fixed64) || (v instanceof SFixed64) ||
		(v instanceof UInt32) || (v instanceof Int32) || (v instanceof SInt32) || (v instanceof Fixed32) || (v instanceof SFixed32)) {
		this.value[0] = v.value[0];
	} else if ((v instanceof Double) || (v instanceof Float) ||	(v instanceof Enum) || (v instanceof Bool)) {
		this.value[0] = v.valueOf();
	} else if (v !== undefined) {
		this.value[0] = v;
	} else {
		this.value[0] = 0;
	}
};

SInt32.prototype.wireType = function() {
	return Bytes.types.Variant;
};

SInt32.prototype.valueOf = function() {
	return this.value[0];
};

SInt32.prototype.toString = function() {
	return this.value[0].toString(arguments[0] || 10);
};

SInt32.prototype.write = function(stream) {
	stream.writeVariantZigZag(this.view);
};

SInt32.prototype.read = function(stream) {
	var v = stream.readVariantZigZag();
	this.value[0] = v.value[0];
};

//===================================================

Fixed32 = function(v) {
	this.buffer = new ArrayBuffer(4);
	this.view = new Uint8Array(this.buffer);
	this.value = new Uint32Array(this.buffer);

	if ((v instanceof UInt64) || (v instanceof Int64) || (v instanceof SInt64) || (v instanceof Fixed64) || (v instanceof SFixed64) ||
		(v instanceof UInt32) || (v instanceof Int32) || (v instanceof SInt32) || (v instanceof Fixed32) || (v instanceof SFixed32)) {
		this.value[0] = v.value[0];
	} else if ((v instanceof Double) || (v instanceof Float) ||	(v instanceof Enum) || (v instanceof Bool)) {
		this.value[0] = v.valueOf();
	} else if (v !== undefined) {
		this.value[0] = v;
	} else {
		this.value[0] = 0;
	}
};

Fixed32.prototype.wireType = function() {
	return Bytes.types.Fixed32;
};

Fixed32.prototype.valueOf = function() {
	return this.value[0];
};

Fixed32.prototype.toString = function() {
	return this.value[0].toString(arguments[0] || 10);
};

Fixed32.prototype.write = function(stream) {
	stream.writeBuffer(this.view);
};

Fixed32.prototype.read = function(stream) {
	stream.readBuffer(this.view, 4);
};

//===================================================

SFixed32 = function(v) {
	this.buffer = new ArrayBuffer(4);
	this.view = new Uint8Array(this.buffer);
	this.value = new Int32Array(this.buffer);

	if ((v instanceof UInt64) || (v instanceof Int64) || (v instanceof SInt64) || (v instanceof Fixed64) || (v instanceof SFixed64) ||
		(v instanceof UInt32) || (v instanceof Int32) || (v instanceof SInt32) || (v instanceof Fixed32) || (v instanceof SFixed32)) {
		this.value[0] = v.value[0];
	} else if ((v instanceof Double) || (v instanceof Float) ||	(v instanceof Enum) || (v instanceof Bool)) {
		this.value[0] = v.valueOf();
	} else if (v !== undefined) {
		this.value[0] = v;
	} else {
		this.value[0] = 0;
	}
};

SFixed32.prototype.wireType = function() {
	return Bytes.types.Fixed32;
};

SFixed32.prototype.valueOf = function() {
	return this.value[0];
};

SFixed32.prototype.toString = function() {
	return this.value[0].toString(arguments[0] || 10);
};

SFixed32.prototype.write = function(stream) {
	stream.writeBuffer(this.view);
};

SFixed32.prototype.read = function(stream) {
	stream.readBuffer(this.view, 4);
};

//===================================================

Float = function(v) {
	this.buffer = new ArrayBuffer(4);
	this.view = new Uint8Array(this.buffer);
	this.value = new Float32Array(this.buffer);

	if ((v instanceof UInt64) || (v instanceof Fixed64)) {
		this.value[0] = v.value[1];
		this.value[0] *= 0x100000000;
		this.value[0] += v.value[0];
	} else if ((v instanceof Int64) || (v instanceof SInt64) || (v instanceof SFixed64)) {
		if ((v.value[1] & 0x80000000) != 0) {
			var vh = v.value[1] ^ 0xFFFFFFFF;
			var vl = v.value[0] ^ 0xFFFFFFFF;
			if (vh < 0)	vh += 0x100000000;
			if (vl < 0) vl += 0x100000000;
			this.value[0] = vh;
			this.value[0] *= 0x100000000;
			this.value[0] += vl;
			this.value[0] += 1;
			this.value[0] = -this.value[0];
		} else {
			this.value[0] = v.value[1];
			this.value[0] *= 0x100000000;
			this.value[0] += v.value[0];
		}
	} else if ((v instanceof UInt32) || (v instanceof Fixed32) || (v instanceof Double) || (v instanceof Float)) {
		this.value[0] = v.value[0];
	} else if ((v instanceof Int32) || (v instanceof SInt32) || (v instanceof SFixed32)) {
		if ((v.value[0] & 0x80000000) != 0) {
			var vl = v.value[0] ^ 0xFFFFFFFF;
			if (vl < 0) vl += 0x100000000;
			this.value[0] = vl;
			this.value[0] += 1;
			this.value[0] = -this.value[0];
		} else {
			this.value[0] = v.value[0];
		}
	} else if ((v instanceof Enum) || (v instanceof Bool)) {
		this.value[0] = v.valueOf();
	} else if (v !== undefined) {
		this.value[0] = v;
	} else {
		this.value[0] = 0;
	}
};

Float.prototype.wireType = function() {
	return Bytes.types.Fixed32;
};

Float.prototype.valueOf = function() {
	return this.value[0];
};

Float.prototype.toString = function() {
	return this.value[0].toString();
};

Float.prototype.write = function(stream) {
	stream.writeBuffer(this.view);
};

Float.prototype.read = function(stream) {
	stream.readBuffer(this.view, 4);
};

//===================================================

Bool = function() {
	this.value = (arguments[0] || false) ? 1 : 0;
	return this;
};

Bool.prototype.wireType = function() {
	return Bytes.types.Variant;
};

Bool.prototype.valueOf = function() {
	return this.value;
};

Bool.prototype.toString = function() {
	return this.value ? "TRUE" : "FALSE";
};

Bool.prototype.write = function(stream) {
	stream.writeVariant(this.value);
};

Bool.prototype.read = function(stream) {
	var v = stream.readVariant();
	this.value = ((v.value[0] != 0) || (v.value[1] != 0)) ? 1 : 0;
};

//===================================================

Enum = function() {
	this.value = arguments[0] || 0;
	return this;
};

Enum.prototype.wireType = function() {
	return Bytes.types.Variant;
};

Enum.assign = function(enums) {
	if (this.keys === undefined)
		this.keys = {};

	for (var key in enums) {
		this[key] = enums[key];
		this.keys[enums[key].toString()] = key.toString();
	}
};

Enum.create = function(varname, enums) {
	eval(varname + " = function() { return Enum.apply(this, arguments); };");		
	var cls = eval(varname);
	Object.extend(cls, Enum);
	cls.assign(enums);
	return cls;
};

Enum.prototype.valueOf = function() {
	return this.value;
};

Enum.prototype.toString = function() {
	if (this.__proto__.constructor.keys !== undefined) {
		return this.__proto__.constructor.keys[this.value] || "";
	}
	return "";
};

Enum.prototype.write = function(stream) {
	stream.writeVariant(this.value);
};

Enum.prototype.read = function(stream) {
	this.value = stream.readVariant().value[0];
};

//===================================================

PackedString = function() {
	this.value = arguments[0] || "";
	return this;
};

PackedString.prototype.wireType = function() {
	return Bytes.types.Packed;
};

PackedString.prototype.valueOf = function() {
	return this.value;
};

PackedString.prototype.toString = function() {
	return this.value;
};

PackedString.prototype.write = function(stream) {
	if (this.value.length > 0) {
		var buffer = String.string2buf(this.value);
		stream.writeVariant(buffer.length);
		stream.writeBuffer(buffer);
	} else {
		stream.writeByte(0);
	}
};

PackedString.prototype.read = function(stream) {
	var v = stream.readVariant();
	if ((v.value[1] != 0) || (v.value[0] > 1024*1024*10)) {
		throw 'Stream error: String to long!';
	}
	if (v.value[0] == 0) {
		this.value = "";
	} else {
		var buffer = new Uint8Array(v.value[0]);
		stream.readBuffer(buffer, v.value[0]);
		this.value = String.buf2string(buffer);
	}
};

//===================================================

Repeated = (function(){
	function Repeated(cls) {
		var obj = Object.create(Array.prototype);
		obj = Array.apply(obj, []);

		obj.__element__ = cls;
		var args = Array.prototype.slice.call(arguments);
		for (var i = 1; i < args.length; i++) {
			if (args[i] instanceof cls) {
				obj.push(args[i]);
			} else if (Array.isArray(args[i])) {
				for (var j = 0; j < args[i].length; j++) {
					if (args[i][j] instanceof cls) {
						obj.push(args[i][j]);
					} else {
						obj.push(new cls(args[i][j]));
					}
				}
			} else {
				obj.push(new cls(args[i]));
			}
		}
		
		for (var method in Repeated.prototype)
			if (Repeated.prototype.hasOwnProperty(method))
				obj[method] = Repeated.prototype[method];

		return obj;
	};
	
	Repeated.prototype.wireType = function() {
		return Bytes.types.Packed;
	};

	Repeated.prototype.write = function(stream) {
		if (this.length > 0) {
			var buffer = new Bytes();
			for (var i = 0; i < this.length; i++) {
				this[i].write(buffer);
			}
			stream.writeVariant(buffer.length);
			stream.writeBuffer(buffer.view.subarray(0, buffer.length));
		} else {
			stream.writeByte(0);
		}
	};
	
	Repeated.prototype.read = function(stream) {
		var v = stream.readVariant();
		if ((v.value[1] != 0) || (v.value[0] > 1024*1024*10)) {
			throw 'Stream error: Repeated to long!';
		}
		if (v.value[0] == 0) {
			this.length = 0;
		} else {
			var buffer = stream.readBytes(v.value[0]);
			while (buffer.position < buffer.length) {
				var v = new this.__element__();
				v.read(buffer);
				this.push(v);
			}
		}
	};
	
	return Repeated;
}).call({});

//===================================================

Message = function() {
	var cls = this.__proto__.constructor;
	var args = Array.prototype.slice.call(arguments);
	for (var i = 0; i < args.length; i++) {
		for (var name in args[i]) {
			if (cls[name] !== undefined) {
				if (cls.__params__[name] !== undefined) {
					this[name] = new cls[name](cls.__params__[name], args[i][name]);
				} else {
					this[name] = new cls[name](args[i][name]);
				}
			}
		}
	}
	
	return this;
};

Message.prototype.wireType = function() {
	return Bytes.types.Packed;
};

Message.assign = function(proto) {
	if (this.__keys__ === undefined) {
		this.__indexes__ = {};
		this.__keys__ = {};
		this.__params__ = {};
	}

	var parser = /^(.*)#(.*)$/;

	for (var key in proto) {
		var find = parser.exec(key);
		if (find && find[1] && find[2]) {
			var name = find[1];
			var index = find[2];
			var obj = proto[key];
			
			if (Array.isArray(obj)) {
				if (obj.__element__ !== undefined) {
					this.__params__[name] = obj.__element__;
					this[name] = Repeated;
				} else {
					throw 'Message: Prototype error!';
				}
			} else {
				if (this.__params__[name] !== undefined) {
					delete this.__params__[name];
				}
				this[name] = obj.__proto__.constructor;
			}
			this.__indexes__[name] = index;
			this.__keys__[index] = name;
			this.prototype[name] = null;
		} else {
			throw 'Message: Prototype error!';
		}
	}
};

Message.create = function(varname, proto) {
	eval(varname + " = function() { return Message.apply(this, arguments); };");		
	var cls = eval(varname);
	Object.extend(cls, Message);
	cls.assign(proto);
	return cls;
};

Message.prototype.valueOf = function() {
	return this;
};

Message.prototype.toString = function() {
	var cls = this.__proto__.constructor;
	var s = "";
	for (var index in cls.__keys__) {
		var name = cls.__keys__[index];
		if (s != "") s += ", ";
		if (this[name] !== null) {
			s += name + ": " + this[name].toString();
		} else {
			s += name + ": null";
		}
	}
	
	return "{" + s + "}";
};

Message.prototype.write = function(stream, base) {
	var cls = this.__proto__.constructor;
	var buffer = new Bytes();
	for (var index in cls.__keys__) {
		var name = cls.__keys__[index];
		if (this[name] !== null) {
			var v = this[name];
			buffer.writeKey(new BytesKey(v.wireType(), parseInt(index)));
			v.write(buffer, false);
		}
	}

	if (buffer.length > 0) {
		if (base === undefined) base = true;
		if (!base)
			stream.writeVariant(buffer.length);
		stream.writeBuffer(buffer.view.subarray(0, buffer.length));
	} else {
		stream.writeByte(0);
	}
};

Message.prototype.read = function(stream, len) {
	if (len === undefined) len = -1;
	if (len < 0) {
		var v = stream.readVariant();
		if ((v.value[1] != 0) || (v.value[0] > 1024*1024*10)) {
			throw 'Stream error: Message to long!';
		}
		len = v.value[0];
	}
	var cls = this.__proto__.constructor;
	for (var index in cls.__keys__) {
		var name = cls.__keys__[index];
		this[name] = null;
	}
	if (len > 0) {
		var buffer = stream.readBytes(len);
		while (buffer.position < buffer.length) {
			var key = buffer.readKey();
			var index = key.field_number.toString();
			if (cls.__keys__[index] !== undefined) {
				var name = cls.__keys__[index];
				if (cls.__params__[name] !== undefined) {
					this[name] = new cls[name](cls.__params__[name]);
				} else {
					this[name] = new cls[name]();
				}

				if (key.wire_type != this[name].wireType()) {
					throw 'Stream error: Wrong wire type!';
				}

				this[name].read(buffer);
			} else {
				buffer.skipKeyValue(key);
			}
		}
	}
};
