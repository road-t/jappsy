Jappsy.JTypeface = function(family, style) {
	return (family & 0xC) | (style & 0x3);
};

Jappsy.JTypeface.DEFAULT = 8;
Jappsy.JTypeface.DEFAULT_BOLD = 9;
Jappsy.JTypeface.MONOSPACE = 4;
Jappsy.JTypeface.SANS_SERIF = 8;
Jappsy.JTypeface.SERIF = 0;

Jappsy.JTypeface.NORMAL = 0;
Jappsy.JTypeface.BOLD = 1;
Jappsy.JTypeface.ITALIC = 2;
Jappsy.JTypeface.BOLD_ITALIC = 3;
