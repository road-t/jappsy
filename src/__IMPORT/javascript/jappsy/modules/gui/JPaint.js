Jappsy.JPaint = function(paint) {
	if (paint === undefined) {
		this.m_scale = 1.0;
		this.m_antialias = false;
		this.m_typeface = Jappsy.JTypeface.DEFAULT;
		this.m_alignX = Jappsy.JAlignX.LEFT;
		this.m_alignY = Jappsy.JAlignY.BASELINE;
		this.m_size = 16.0;
		
		this.m_invisible = false;
		this.m_color = 0xFF000000;
		this.m_color4iv = new Vec4([0,0,0,1]);
		
		this.m_strokeInvisible = true;
		this.m_strokeOffset = 0;
		this.m_strokeWidth = 0;
		this.m_strokeColor = 0x00000000;
		
		this.m_shadowInvisible = true;
		this.m_shadow = [0, 0];
		this.m_shadowWidth = 0;
		this.m_shadowColor = 0x00000000;
		
		this.m_sdfInvisible = true;
		this.m_sdfColors = [ 0x00000000, 0x00000000, 0x00000000, 0x00000000 ];
	} else {
		this.m_scale = paint.m_scale;
		this.m_antialias = paint.m_antialias;
		this.m_typeface = paint.m_typeface;
		this.m_alignX = paint.m_alignX;
		this.m_alignY = paint.m_alignY;
		this.m_size = paint.m_size;
		
		this.m_invisible = paint.m_invisible;
		this.m_color = paint.m_color;
		this.m_color4iv = new Vec4(paint.m_color4iv);
		
		this.m_strokeInvisible = paint.m_strokeInvisible;
		this.m_strokeOffset = paint.m_strokeOffset;
		this.m_strokeWidth = paint.m_strokeWidth;
		this.m_strokeColor = paint.m_strokeColor;
		
		this.m_shadowInvisible = paint.m_shadowInvisible;
		this.m_shadow = paint.m_shadow.slice(0);
		this.m_shadowWidth = paint.m_shadowWidth;
		this.m_shadowColor = paint.m_shadowColor;
		
		this.m_sdfInvisible = paint.m_sdfInvisible;
		this.m_sdfColors = paint.m_sdfColors.slice(0);
	}
};

Jappsy.JPaint.prototype.setSDFColors = function (colors) {
	var count = colors.length;
	if (count > 4) count = 4;
	for (var i = 0; i < count; i++)
		this.m_sdfColors[i] = colors[i];
	for (var i = count; i < 4; i++)
		this.m_sdfColors[i] = 0x00000000;
	this.m_sdfInvisible = (count == 0);
	return this;
};

Jappsy.JPaint.prototype.setAntiAlias = function (antialias) {
	this.m_antialias = antialias;
	return this;
};

Jappsy.JPaint.prototype.setTypeface = function (typeface) {
	this.m_typeface = typeface;
	return this;
};

Jappsy.JPaint.prototype.setTextSize = function (size) {
	this.m_size = size;
	return this;
};

Jappsy.JPaint.prototype.setAlignX = function (align) {
	this.m_alignX = align;
	return this;
};

Jappsy.JPaint.prototype.setAlignY = function (align) {
	this.m_alignY = align;
	return this;
};

Jappsy.JPaint.prototype.setColor = function (color) {
	this.m_color = color;
	this.m_invisible = (color & 0xFF000000) == 0;
	this.m_color4iv[0] = ((color >>> 16) & 0xFF) / 255.0;		// R
	this.m_color4iv[1] = ((color >>> 8) & 0xFF) / 255.0;		// G
	this.m_color4iv[2] = (color & 0xFF) / 255.0;				// B
	this.m_color4iv[3] = ((color >>> 24) & 0xFF) / 255.0;		// A
	return this;
};

Jappsy.JPaint.prototype.setLight = function (light, alpha) {
	this.m_color4iv[0] = this.m_color4iv[1] = this.m_color4iv[2] = light;
	this.m_color4iv[3] = alpha;
	this.m_color =
		Math.floor(Jappsy.clamp(this.m_color4iv[2] * 255.0, 0, 255)) |
		(Math.floor(Jappsy.clamp(this.m_color4iv[1] * 255.0, 0, 255)) << 8) |
		(Math.floor(Jappsy.clamp(this.m_color4iv[0] * 255.0, 0, 255)) << 16) |
		(Math.floor(Jappsy.clamp(this.m_color4iv[3] * 255.0, 0, 255)) << 24);
	this.m_invisible = (this.m_color & 0xFF000000) == 0;
	return this;
};

Jappsy.JPaint.prototype.setStroke = function (width, color) {
	this.m_strokeOffset = -width;
	this.m_strokeWidth = width;
	this.m_strokeColor = color;
	this.m_strokeInvisible = ((color & 0xFF000000) == 0) || (width == 0);
	return this;
};

Jappsy.JPaint.prototype.setAlpha = function (alpha) {
	this.m_color = (this.m_color & 0x00FFFFFF) | ((alpha & 0xFF) << 24);
	this.m_invisible = (this.m_color & 0xFF000000) == 0;
	return this;
};

Jappsy.JPaint.prototype.setShadow = function (width, dx, dy, color) {
	this.m_shadow = [ dx, dy ];
	this.m_shadowWidth = width;
	this.m_shadowColor = color;
	this.m_shadowInvisible = ((color & 0xFF000000) == 0) || ((dx == 0) && (dy == 0) && (width == 0));
	return this;
};

Jappsy.JPaint.prototype.setScale = function (scale) {
	this.m_scale = scale;
	return this;
};
