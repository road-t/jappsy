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

package com.jappsy.opengl.image;

import com.jappsy.exceptions.EOutOfMemory;
import com.jappsy.memory.MemoryManager;

public class GLSprite { 					// Sprite - Part/Full OpenGL Texture
	public boolean m_alpha;					// Alpha channel
	public int[] m_vertex = new int[4];		// Rectangle Part of Image
	public int[] m_texture = new int[4];	// Rectangle Part of Texture

	// GLRender update
	public boolean m_visible;				// Visibility check
	
	private static final MemoryManager m_mem = new MemoryManager();
	
	public static final GLSprite create() throws EOutOfMemory {
		GLSprite o = (GLSprite)(m_mem.getObject());
		if (o == null) {
			try {
				o = (GLSprite)(m_mem.addObject(new GLSprite()));
			} catch (OutOfMemoryError e) {
				throw new EOutOfMemory(e);
			}
		}
		return o;
	}
	
	protected final void clear() {
		m_alpha = false;
		m_vertex[0] = m_vertex[1] = m_vertex[2] = m_vertex[3] = 0;
		m_texture[0] = m_texture[1] = m_texture[2] = m_texture[3] = 0;
		
		m_visible = false;
	}
	
	public final void release() {
		clear();
		m_mem.returnObject(this);
	}
	
	public static final GLSprite[] createArray(int count) throws EOutOfMemory {
		GLSprite[] array;
		try {
			array = new GLSprite[count];
			try {
				for (int i = 0; i < count; i++) {
					array[i] = GLSprite.create();
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
	
	public static final void releaseArray(GLSprite[] array, int count) {
		if ((array != null) && (count != 0)) {
			for (int i = 0; i < count; i++) {
				if (array[i] != null) {
					array[i].release();
					array[i] = null;
				}
			}
		}
	}
	
	public static final void clearArray(GLSprite[] array, int count) {
		if ((array != null) && (count != 0)) {
			for (int i = 0; i < count; i++) {
				array[i] = null;
			}
		}
	}
	
	// GLRender update
	public final int updateVisibility(int x1, int y1, int x2, int y2) {
		if ((m_vertex[0] == 0) ||
			(m_vertex[1] == 0) ||
			(m_vertex[2] == 0) ||
			(m_vertex[3] == 0) ||
			(m_vertex[0] >= x2) ||
			(m_vertex[1] >= y2) ||
			(m_vertex[2] < x1) ||
			(m_vertex[3] < y1)
		) {
			m_visible = false;
			return 0;
		} else {
			m_visible = true;
			return 1;
		}
	}
	
	//GLRender update
	public final void updateVisibleRect(int[] v, int[] t, int x1, int y1, int x2, int y2) {
		v[0] = m_vertex[0];
		v[1] = m_vertex[1];
		v[2] = m_vertex[2];
		v[3] = m_vertex[3];
		t[0] = m_texture[0];
		t[1] = m_texture[1];
		t[2] = m_texture[2];
		t[3] = m_texture[3];
		
		if (v[0] < x1) {
			t[0] += (x1 - v[0]);
			v[0] = x1;
		}
		if (v[1] < y1) {
			t[1] += (y1 - v[1]);
			v[1] = y1;
		}
		if (v[2] > x2) {
			t[2] -= (v[2] - x2);
			v[2] = x2;
		}
		if (v[3] > y2) {
			t[3] -= (v[3] - y2);
			v[3] = y2;
		}
		v[0] -= x1;
		v[1] -= y1;
		v[2] -= x1;
		v[3] -= y1;
	}
}
