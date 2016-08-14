Jappsy.assign(Jappsy, {
	shrinkBuf: function (buf, size) {
		if (buf.length === size) return buf;
		if (buf.subarray) return buf.subarray(0, size);
		buf.length = size;
		return buf;
	},
	
	arraySet: function (dest, src, src_offs, len, dest_offs) {
		if (src.subarray && dest.subarray) {
			dest.set(src.subarray(src_offs, src_offs + len), dest_offs);
			return;
		}
		// Fallback to ordinary array
		for (var i = 0; i < len; i++) {
			dest[dest_offs + i] = src[src_offs + i];
		}
	},
	
	// Join array of chunks to single array.
	flattenChunks: function (chunks) {
		var i, l, len, pos, chunk, result;

		// calculate data length
		len = 0;
		for (i = 0, l = chunks.length; i < l; i++)
			len += chunks[i].length;

		// join chunks
		result = new Uint8Array(len);
		pos = 0;
		for (i = 0, l = chunks.length; i < l; i++) {
			chunk = chunks[i];
			result.set(chunk, pos);
			pos += chunk.length;
		}

		return result;
	},
	
	Buf8: Uint8Array,
	Buf16: Uint16Array,
	Buf32: Int32Array
});
