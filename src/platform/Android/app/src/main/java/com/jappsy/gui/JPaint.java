package com.jappsy.gui;

public class JPaint {
    public float m_scale = 1.0f;
    public boolean m_antialias = false;
    public int m_typeface = JTypeface.DEFAULT;
    public int m_alignX = JAlignX.LEFT;
    public int m_alignY = JAlignY.BASELINE;
    public float m_size = 16.0f;

    public boolean m_invisible = false;
    public int m_color = 0xFF000000;

    public boolean m_strokeInvisible = true;
    public float m_strokeOffset = 0;
    public float m_strokeWidth = 0;
    public int m_strokeColor = 0x00000000;
    
    public boolean m_shadowInvisible = true;
    public int[] m_shadow = {0, 0};
    public int m_shadowWidth = 0;
    public int m_shadowColor = 0x00000000;

    public int m_colorFilter = JPaintColorFilter.PAINT_COLOR_FILTER_NORMAL;
    
    public boolean m_sdfInvisible = true;
    public int[] m_sdfColors = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    
    public JPaint setSDFColors(int[] colors) {
    	int len = 0;
    	if (colors != null) {
    		len = colors.length;
        	if (len > 4) len = 4;
    	}
    	int i = 0;
    	int t = 0;
    	while (i < len) {
    		m_sdfColors[i] = colors[i];
    		if ((m_sdfColors[i] & 0xFF000000) == 0) t++;
    		i++;
    	}
    	while (i < 4) {
    		m_sdfColors[i] = 0x00000000;
    		t++;
    		i++;
    	}
    	m_sdfInvisible = (t == 4);
    	return this;
    }
    
    public JPaint setAntiAlias(boolean antialias) {
        m_antialias = antialias;
        return this;
    }
    
    public JPaint setTypeface(int typeface) {
        m_typeface = typeface;
        return this;
    }
    
    public JPaint setTextSize(float size) {
        m_size = size;
        return this;
    }
    
    public JPaint setAlignX(int align) {
        m_alignX = align;
        return this;
    }
    
    public JPaint setAlignY(int align) {
        m_alignY = align;
        return this;
    }
    
    public JPaint setColor(int color) {
        m_color = color;
        m_invisible = (m_color & 0xFF000000) == 0;
        return this;
    }
    
    public JPaint setStroke(int width, int color) {
    	m_strokeOffset = -width;
        m_strokeWidth = width;
        m_strokeColor = color;
        m_strokeInvisible = ((color & 0xFF000000) == 0) || (width == 0);
        return this;
    }
    
    public JPaint setStroke(float offset, float width, int color) {
    	m_strokeOffset = offset;
        m_strokeWidth = width;
        m_strokeColor = color;
        m_strokeInvisible = ((color & 0xFF000000) == 0) || (width == 0);
        return this;
    }
    
    public JPaint setAlpha(byte alpha) {
        m_color = (m_color & 0x00FFFFFF) | ((int)alpha << 24);
        m_invisible = (m_color & 0xFF000000) == 0;
        return this;
    }
    
    public JPaint setShadow(int width, int dx, int dy, int color) {
        m_shadow[0] = dx;
        m_shadow[1] = dy;
        m_shadowWidth = width;
        m_shadowColor = color;
        m_shadowInvisible = ((color & 0xFF000000) == 0) || ((dx == 0) && (dy == 0) && (width == 0));
        return this;
    }
    
    public JPaint setScale(float scale) {
        m_scale = scale;
        return this;
    }
    
    public JPaint setColorFilter(int colorFilter) {
        m_colorFilter = colorFilter;
        return this;
    }
}
