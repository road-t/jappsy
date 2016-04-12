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
import com.jappsy.opengl.GLContext;

public class GLTexture {					// OpenGL Texture
	public int[] m_handle = new int[1];		// OpenGL Handle
	public int m_fullWidth;					// Texture Real Width
	public int m_fullHeight;				// Texture Real Height
	public int m_cropX;						// Crop Start X
	public int m_cropY;						// Crop Start Y
	public int m_cropWidth;					// Texture Cropped Width
	public int m_cropHeight;				// Texture Cropped Height
	
	public int m_count;						// Count of sprites
	public int m_normalCount;				// Count of sprites without alpha channel
	public int m_alphaCount;				// Count of sprites with alpha channel
	public GLSprite[] m_sprites;			// Sprites list
	public GLSprite[] m_normalSprites;
	public GLSprite[] m_alphaSprites;
	
	// GLRender update
	public int m_visibleNormalCount;
	public int m_visibleAlphaCount;
	
	private static final MemoryManager m_mem = new MemoryManager();
	
	public static final GLTexture create() throws EOutOfMemory {
		GLTexture o = (GLTexture)(m_mem.getObject());
		if (o == null) {
			try {
				o = (GLTexture)(m_mem.addObject(new GLTexture()));
			} catch (OutOfMemoryError e) {
				throw new EOutOfMemory(e);
			}
		}
		return o;
	}
	
	protected final void clear() {
		GLContext.destroyTexture(m_handle);
		m_fullWidth = m_fullHeight = m_cropX = m_cropY = m_cropWidth = m_cropHeight = 0;
		
		GLSprite.releaseArray(m_sprites, m_count);
		GLSprite.clearArray(m_normalSprites, m_normalCount);
		GLSprite.clearArray(m_alphaSprites, m_alphaCount);

		m_count = m_normalCount = m_alphaCount = 0;
		m_sprites = m_normalSprites = m_alphaSprites = null;
		
		m_visibleNormalCount = m_visibleAlphaCount = 0;
	}
	
	public final void release() {
		clear();
		m_mem.returnObject(this);
	}
	
	public static final GLTexture[] createArray(int count) throws EOutOfMemory {
		GLTexture[] array;
		try {
			array = new GLTexture[count];
			try {
				for (int i = 0; i < count; i++) {
					array[i] = GLTexture.create();
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
	
	public static final void releaseArray(GLTexture[] array, int count) {
		if ((array != null) && (count != 0)) {
			for (int i = 0; i < count; i++) {
				if (array[i] != null) {
					array[i].release();
					array[i] = null;
				}
			}
		}
	}
	
	// GLImage init
	public final void createSprites() throws EOutOfMemory {
		int normalCount = 0;
    	int alphaCount = 0;
    	
    	for (int j = 0; j < m_count; j++) {
    		if (m_sprites[j].m_alpha)
    			alphaCount++;
    		else
    			normalCount++;
    	}
    	
    	m_normalSprites = GLSprite.createArray(normalCount);
    	m_normalCount = normalCount;

    	m_alphaSprites = GLSprite.createArray(alphaCount);
    	m_alphaCount = alphaCount;

    	int normalIndex = 0;
    	int alphaIndex = 0;
    	for (int j = 0; j < m_count; j++) {
    		if (m_sprites[j].m_alpha)
    			m_alphaSprites[alphaIndex++] = m_sprites[j];
    		else
    			m_normalSprites[normalIndex++] = m_sprites[j];
    	}
	}

}

