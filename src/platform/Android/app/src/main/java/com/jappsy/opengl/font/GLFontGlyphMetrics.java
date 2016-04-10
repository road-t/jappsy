package com.jappsy.opengl.font;

import com.jappsy.exceptions.EOutOfMemory;
import com.jappsy.memory.MemoryManager;

import android.util.SparseArray;

public class GLFontGlyphMetrics {

	public char m_glyph;
	public float m_x;
	public float m_y;
	public float m_width;
	public float m_height;
	public float m_outX;
	public float m_outY;
	public float m_nextX;
	public float m_nextY;
	
	public int m_kerningPairsCount;
	public SparseArray<Float> m_kerningPairs;

	private static final MemoryManager m_mem = new MemoryManager();
	
	public static final GLFontGlyphMetrics create() throws EOutOfMemory {
		GLFontGlyphMetrics o = (GLFontGlyphMetrics)(m_mem.getObject());
		if (o == null) {
			try {
				o = (GLFontGlyphMetrics)(m_mem.addObject(new GLFontGlyphMetrics()));
			} catch (OutOfMemoryError e) {
				throw new EOutOfMemory(e);
			}
		}
		return o;
	}
	
	protected final void clear() {
		m_glyph = 0;
		m_x = m_y = m_width = m_height = m_outX = m_outY = m_nextX = m_nextY = 0;
		m_kerningPairsCount = 0;
		
		if (m_kerningPairs != null) {
			m_kerningPairs.clear();
			m_kerningPairs = null;
		}
	}
	
	public final void release() {
		clear();
		m_mem.returnObject(this);
	}
	
	public static final GLFontGlyphMetrics[] createArray(int count) throws EOutOfMemory {
		GLFontGlyphMetrics[] array;
		try {
			array = new GLFontGlyphMetrics[count];
			try {
				for (int i = 0; i < count; i++) {
					array[i] = GLFontGlyphMetrics.create();
				}
			} catch (EOutOfMemory e) {
				releaseArray(array, count);
				throw new EOutOfMemory(e);
			}
		} catch (OutOfMemoryError e) {
			throw new EOutOfMemory(e);
		}
		return array;
	}
	
	public static final void releaseArray(GLFontGlyphMetrics[] array, int count) {
		if ((array != null) && (count != 0)) {
			for (int i = 0; i < count; i++) {
				if (array[i] != null) {
					array[i].release();
					array[i] = null;
				}
			}
		}
	}
	
	public final void createPairs(int count) throws EOutOfMemory {
		m_kerningPairsCount = count;
		if (count == 0) {
			if (m_kerningPairs != null) {
				m_kerningPairs.clear();
				m_kerningPairs = null;
			}
		} else {
			try {
				m_kerningPairs = new SparseArray<Float>();
			} catch (OutOfMemoryError e) {
				throw new EOutOfMemory(e);
			}
		}
	}

}
