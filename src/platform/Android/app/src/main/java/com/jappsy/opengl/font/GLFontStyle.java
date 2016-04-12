/*
 * Copyright (C) 2016 The Jappsy Open Source Project (http://jappsy.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.jappsy.opengl.font;

import com.jappsy.exceptions.EOutOfMemory;
import com.jappsy.memory.MemoryManager;

public class GLFontStyle {

	public float m_height;
	public float m_ascent;
	public float m_descent;

	public String m_glyphIndexTable;
	public int m_count;
	public GLFontGlyphMetrics[] m_glyphMetrics;
	public GLFontGlyphMetrics m_defaultGlyph;

	private static final MemoryManager m_mem = new MemoryManager();
	
	public static final GLFontStyle create() throws EOutOfMemory {
		GLFontStyle o = (GLFontStyle)(m_mem.getObject());
		if (o == null) {
			try {
				o = (GLFontStyle)(m_mem.addObject(new GLFontStyle()));
			} catch (OutOfMemoryError e) {
				throw new EOutOfMemory(e);
			}
		}
		return o;
	}
	
	protected final void clear() {
		m_height = m_ascent = m_descent = 0;
		m_glyphIndexTable = null;
		
		m_defaultGlyph = null;
		
		if ((m_count != 0) && (m_glyphMetrics != null)) {
			GLFontGlyphMetrics.releaseArray(m_glyphMetrics, m_count);
		}
		m_glyphMetrics = null;
		m_count = 0;
	}
	
	public final void release() {
		clear();
		m_mem.returnObject(this);
	}
	
	public static final GLFontStyle[] createArray(int count) throws EOutOfMemory {
		GLFontStyle[] array;
		try {
			array = new GLFontStyle[count];
			try {
				for (int i = 0; i < count; i++) {
					array[i] = GLFontStyle.create();
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
	
	public static final void releaseArray(GLFontStyle[] array, int count) {
		if ((array != null) && (count != 0)) {
			for (int i = 0; i < count; i++) {
				if (array[i] != null) {
					array[i].release();
					array[i] = null;
				}
			}
		}
	}
	
	public final GLFontGlyphMetrics findGlyph(final char glyph) {
		int index = m_glyphIndexTable.indexOf(glyph);
		if (index >= 0) {
			return m_glyphMetrics[index];
		}
		return m_defaultGlyph;
	}
	
	/*
	private final void generateGlyph(GLFontGlyphMetrics glyph, GLFontGlyphMetrics prevGlyph, float x, float y, float scale, float[] next, float[] vertexSquare, float[] textureSquare) {
		if ((prevGlyph != null) && (prevGlyph.kerningPairsCount != 0)) {
			Float kernAmount = prevGlyph.kerningPairs.get(glyph.glyph);
			if (kernAmount != null) {
				x += kernAmount.floatValue() * scale;
			}
		}
		
		if (next != null) {
			next[0] = (float)glyph.nextX * scale;
			next[1] = (float)glyph.nextY * scale;
		}
		
		float vx1 = x + glyph.outX * scale;
		float vy1 = y + glyph.outY * scale;
		float vx2 = vx1 + glyph.width * scale;
		float vy2 = vy1 + glyph.height * scale;
		float tx1 = glyph.x / (float)m_info.fullSize[0];
		float ty1 = glyph.y / (float)m_info.fullSize[1];
		float tx2 = tx1 + glyph.width / (float)m_info.fullSize[0];
		float ty2 = ty1 + glyph.height / (float)m_info.fullSize[1];
		
		vertexSquare[0] = vertexSquare[4] = vx1;
		vertexSquare[1] = vertexSquare[3] = vy1;
		vertexSquare[2] = vertexSquare[6] = vx2;
		vertexSquare[5] = vertexSquare[7] = vy2;
        
        textureSquare[0] = textureSquare[4] = tx1;
        textureSquare[1] = textureSquare[3] = ty1;
        textureSquare[2] = textureSquare[6] = tx2;
        textureSquare[5] = textureSquare[7] = ty2;
	}
	*/

}
