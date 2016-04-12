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

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.IntBuffer;

import com.jappsy.exceptions.EOutOfMemory;
import com.jappsy.memory.Memory;
import com.jappsy.memory.MemoryManager;

public class GLTextureBuffer {				// Preprocessed texture data for rendering
	public int[] m_handle = new int[1];		// OpenGL Handle
	public boolean m_alpha;					// Alpha channel
	public int m_count;						// Count of Quads
	public int[] m_vertex;					// Out vertex buffer
	public int[] m_texture;					// Texture vertex buffer
	public IntBuffer m_vertexBuffer;
	public IntBuffer m_textureBuffer;
	
	private static final MemoryManager m_mem = new MemoryManager();
	
	public static final GLTextureBuffer create() throws EOutOfMemory {
		GLTextureBuffer o = (GLTextureBuffer)(m_mem.getObject());
		if (o == null) {
			try {
				o = (GLTextureBuffer)(m_mem.addObject(new GLTextureBuffer()));
			} catch (OutOfMemoryError e) {
				throw new EOutOfMemory(e);
			}
		}
		return o;
	}
	
	protected final void clear() throws EOutOfMemory {
		m_handle[0] = 0;
		m_alpha = false;
		
		releaseBuffers();
	}
	
	public final void release() throws EOutOfMemory {
		clear();
		m_mem.returnObject(this);
	}
	
	public final void updateBuffers(int count) throws EOutOfMemory {
		if (m_count != count) {
			try {
				m_vertex = Memory.mmVirtualAllocInt(8 * count);
				m_texture = Memory.mmVirtualAllocInt(8 * count);
				m_vertexBuffer = ByteBuffer.allocateDirect(8 * 4 * count).order(ByteOrder.nativeOrder()).asIntBuffer();
				m_textureBuffer = ByteBuffer.allocateDirect(8 * 4 * count).order(ByteOrder.nativeOrder()).asIntBuffer();
			} catch (OutOfMemoryError e) {
				releaseBuffers();
				throw new EOutOfMemory(e);
			}
			m_count = count;
		}
	}
	
	private final void releaseBuffers() throws EOutOfMemory {
		m_vertexBuffer = m_textureBuffer = null;
		if (m_vertex != null) Memory.mmVirtualFreeInt(m_vertex);
		if (m_texture != null) Memory.mmVirtualFreeInt(m_texture);
		m_vertex = m_texture = null;
		m_count = 0;
	}

	public static final GLTextureBuffer[] createArray(int count) throws EOutOfMemory {
		GLTextureBuffer[] array;
		try {
			array = new GLTextureBuffer[count];
			try {
				for (int i = 0; i < count; i++) {
					array[i] = GLTextureBuffer.create();
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
	
	public static final void releaseArray(GLTextureBuffer[] array, int count) throws EOutOfMemory {
		if ((array != null) && (count != 0)) {
			for (int i = 0; i < count; i++) {
				if (array[i] != null) {
					array[i].release();
					array[i] = null;
				}
			}
		}
	}
	
	public final void update() {
		m_vertexBuffer.position(0);
		m_textureBuffer.position(0);
		m_vertexBuffer.put(m_vertex, 0, m_count * 8);
		m_textureBuffer.put(m_texture, 0, m_count * 8);
		m_vertexBuffer.position(0);
		m_textureBuffer.position(0);
	}
	
	private int m_position = 0;
	
	public final void seek(int pos) {
		m_position = pos;
	}
	
	public final void writeVertex(float vx1, float vy1, float vx2, float vy2) {
		m_vertex[m_position + 0] = m_vertex[m_position + 4] = Float.floatToRawIntBits(vx1);
		m_vertex[m_position + 1] = m_vertex[m_position + 3] = Float.floatToRawIntBits(vy1);
		m_vertex[m_position + 2] = m_vertex[m_position + 6] = Float.floatToRawIntBits(vx2);
		m_vertex[m_position + 5] = m_vertex[m_position + 7] = Float.floatToRawIntBits(vy2);
	}

	public final void writeTexture(float tx1, float ty1, float tx2, float ty2) {
		m_texture[m_position + 0] = m_texture[m_position + 4] = Float.floatToRawIntBits(tx1);
		m_texture[m_position + 1] = m_texture[m_position + 3] = Float.floatToRawIntBits(ty1);
		m_texture[m_position + 2] = m_texture[m_position + 6] = Float.floatToRawIntBits(tx2);
		m_texture[m_position + 5] = m_texture[m_position + 7] = Float.floatToRawIntBits(ty2);
	}
	
	public final void next() {
		m_position += 8;
	}
	
}

