package com.jappsy.opengl.image;

import com.jappsy.exceptions.EGLInvalidResource;
import com.jappsy.exceptions.EOutOfMemory;
import com.jappsy.memory.MemoryManager;
import com.jappsy.opengl.font.GLFontGlyphMetrics;
import com.jappsy.opengl.font.GLFontStyle;

public class GLRender {
	public boolean m_visible;				// Visible state
	public float[] m_size = new float[2];
	public int m_count;
	public GLTextureBuffer[] m_buffers;
	
	public float[] m_alignX = new float[5]; // Horizontal Align offsets
	public float[] m_alignY = new float[7];	// Vertical Align offsets
	public float[] m_box = new float[4];	// Bounding box without SDF border
	
	// OpenGL SDF PreCalculated Data
	public float[] m_sdfConst = new float[4];	// SDF const { onePixelDistance, halfPixelDistance, oneHalfPixelDistance, maxPixelDistance }
	public float[] m_sdfParams = new float[4];	// SDF params { edgeDistance, edgeCenter, outerOffset, innerOffset }
	
	private static final MemoryManager m_mem = new MemoryManager();
	
	public static final GLRender create() throws EOutOfMemory {
		GLRender o = (GLRender)(m_mem.getObject());
		if (o == null) {
			try {
				o = (GLRender)(m_mem.addObject(new GLRender()));
			} catch (OutOfMemoryError e) {
				throw new EOutOfMemory(e);
			}
		}
		return o;
	}
	
	public final void release() throws EOutOfMemory {
		clear();
		m_mem.returnObject(this);
	}
	
	protected final void clear() throws EOutOfMemory {
		m_visible = false;
		
		GLTextureBuffer.releaseArray(m_buffers, m_count);

		m_buffers = null;
		m_count = 0;
	}
	
	private final void updateBuffers(final int count) throws EOutOfMemory {
		if (count == 0) {
			m_visible = false;
			return;
		}
		if ((m_buffers != null) && (m_count != count)) {
			GLTextureBuffer.releaseArray(m_buffers, m_count);
			m_buffers = null;
			m_count = 0;
		}
		if (m_buffers == null) {
			m_buffers = GLTextureBuffer.createArray(count);
			m_count = count;
		}
	}
	
	// Generate GLRender structure for stretched image
	public final void update(GLImage image, final int width, final int height, final float scale) throws EOutOfMemory, EGLInvalidResource {
		if (image.m_style == GLImage.GLImageStyleDistanceField)
			throw new EGLInvalidResource();
		
		m_size[0] = width;
		m_size[1] = height;
		m_alignX[0] = m_alignX[1] = 0;
		m_alignX[2] = m_size[0] / 2.0f;
		m_alignX[3] = m_alignX[4] = m_size[0];
		m_alignY[0] = m_alignY[1] = m_alignY[2] = 0;
		m_alignY[3] = m_size[1] / 2.0f;
		m_alignY[4] = m_alignY[5] = m_alignY[6] = m_size[1];
		m_box[0] = m_box[1] = 0;
		m_box[2] = m_size[0];
		m_box[3] = m_size[1];

		// Calculate count of texture buffers
		int count = 0;
		for (int i = 0; i < image.m_count; i++) {
			if (image.m_list[i].m_handle[0] != 0) {
				if (image.m_list[i].m_normalCount > 0)
					count++;
				if (image.m_list[i].m_alphaCount > 0)
					count++;
			}
		}
		
		updateBuffers(count);
		
		int bufferIndex = 0;
		for (int i = 0; i < image.m_count; i++) {
			if (image.m_list[i].m_handle[0] != 0) {
				GLTexture texture = image.m_list[i];
				int normalCount = texture.m_normalCount;
				if (normalCount > 0) {
					m_buffers[bufferIndex].m_handle[0] = texture.m_handle[0];
					m_buffers[bufferIndex].m_alpha = false;
					if (m_buffers[bufferIndex].m_count != normalCount)
						m_buffers[bufferIndex].updateBuffers(normalCount);
					bufferIndex++;
				}
				int alphaCount = texture.m_alphaCount;
				if (alphaCount > 0) {
					m_buffers[bufferIndex].m_handle[0] = texture.m_handle[0];
					m_buffers[bufferIndex].m_alpha = true;
					if (m_buffers[bufferIndex].m_count != alphaCount)
						m_buffers[bufferIndex].updateBuffers(alphaCount);
					bufferIndex++;
				}
			}
		}
		
		if (image.m_style == GLImage.GLImageStylePatch9) {
			GLTexture texture = image.m_list[0];
			
			float[] vertexOfsX = new float[4];
		    float[] vertexOfsY = new float[4];
		    
		    vertexOfsX[0] = 0;
            vertexOfsX[1] = (float) (vertexOfsX[0] + Math.floor((float)(texture.m_sprites[3].m_vertex[2] - texture.m_sprites[3].m_vertex[0]) * scale));
            vertexOfsX[3] = m_size[0];
            vertexOfsX[2] = (float) (vertexOfsX[3] - Math.floor((float)(texture.m_sprites[5].m_vertex[2] - texture.m_sprites[5].m_vertex[0]) * scale));
            
            vertexOfsY[0] = 0;
            vertexOfsY[1] = (float) (vertexOfsY[0] + Math.floor((float)(texture.m_sprites[1].m_vertex[3] - texture.m_sprites[1].m_vertex[1]) * scale));
            vertexOfsY[3] = m_size[1];
            vertexOfsY[2] = (float) (vertexOfsY[3] - Math.floor((float)(texture.m_sprites[7].m_vertex[3] - texture.m_sprites[7].m_vertex[1]) * scale));
            
            if ((vertexOfsX[2] <= vertexOfsX[1]) || (vertexOfsY[2] <= vertexOfsY[1])) {
            	m_visible = false;
            	return;
            }
            
            int normalIdx = 0;
            int alphaIdx = 0;
			int spriteIdx = 0;
            for (int iy = 0; iy < 3; iy++) {
                for (int ix = 0; ix < 3; ix++) {
                    GLSprite sprite = texture.m_sprites[spriteIdx];
                    bufferIndex = (m_buffers[0].m_alpha != sprite.m_alpha) ? 1 : 0;
                    GLTextureBuffer buffer = m_buffers[bufferIndex];
                    
                    int index = sprite.m_alpha ? alphaIdx : normalIdx;

                    /*
                    buffer.m_vertexBuffer[index+0] = buffer.m_vertexBuffer[index+4] = vertexOfsX[ix];
                    buffer.m_vertexBuffer[index+1] = buffer.m_vertexBuffer[index+3] = vertexOfsY[iy];
                    buffer.m_vertexBuffer[index+2] = buffer.m_vertexBuffer[index+6] = vertexOfsX[ix + 1];
                    buffer.m_vertexBuffer[index+5] = buffer.m_vertexBuffer[index+7] = vertexOfsY[iy + 1];

                    buffer.m_textureBuffer[index+0] = buffer.m_textureBuffer[index+4] = (float)(sprite.m_texture[0]) / texture.m_fullWidth;
                    buffer.m_textureBuffer[index+1] = buffer.m_textureBuffer[index+3] = (float)(sprite.m_texture[1]) / texture.m_fullHeight;
                    buffer.m_textureBuffer[index+2] = buffer.m_textureBuffer[index+6] = (float)(sprite.m_texture[2]) / texture.m_fullWidth;
                    buffer.m_textureBuffer[index+5] = buffer.m_textureBuffer[index+7] = (float)(sprite.m_texture[3]) / texture.m_fullHeight;
                    */
                    buffer.seek(index);
                    buffer.writeVertex(vertexOfsX[ix], vertexOfsY[iy], vertexOfsX[ix + 1], vertexOfsY[iy + 1]);
                    buffer.writeTexture(
                    		(float)(sprite.m_texture[0]) / texture.m_fullWidth,
                    		(float)(sprite.m_texture[1]) / texture.m_fullHeight,
                    		(float)(sprite.m_texture[2]) / texture.m_fullWidth,
                    		(float)(sprite.m_texture[3]) / texture.m_fullHeight);
                    
                    if (sprite.m_alpha)
                    	alphaIdx = index + 8;
                    else
                    	normalIdx = index + 8; 

                    spriteIdx++;
                }
            }
            
            for (int i = 0; i < m_count; i++) {
            	m_buffers[i].update();
            }
		} else {
			if ((image.m_count == 1) && (image.m_list[0].m_count == 1)) {
				GLTexture texture = image.m_list[0];
				GLSprite sprite = texture.m_sprites[0];
				GLTextureBuffer buffer = m_buffers[0];
				
				/*
				buffer.m_vertexBuffer[0] = buffer.m_vertexBuffer[4] = m_view[0];
				buffer.m_vertexBuffer[1] = buffer.m_vertexBuffer[3] = m_view[1];
				buffer.m_vertexBuffer[2] = buffer.m_vertexBuffer[6] = m_view[2];
				buffer.m_vertexBuffer[5] = buffer.m_vertexBuffer[7] = m_view[3];

                if (image.m_style == GLImage.GLImageStyleRepeat) {
                	buffer.m_textureBuffer[0] = buffer.m_textureBuffer[4] = 0;
                	buffer.m_textureBuffer[1] = buffer.m_textureBuffer[3] = 0;
                	buffer.m_textureBuffer[2] = buffer.m_textureBuffer[6] = ((float)(m_view[2] - m_view[0]) / image.m_scale) / texture.m_fullWidth;
                	buffer.m_textureBuffer[5] = buffer.m_textureBuffer[7] = ((float)(m_view[3] - m_view[1]) / image.m_scale) / texture.m_fullHeight;
                } else if (image.m_style == GLImage.GLImageStyleFlip) {
                	buffer.m_textureBuffer[0] = buffer.m_textureBuffer[4] = (float)(sprite.m_texture[0]) / texture.m_fullWidth;
                	buffer.m_textureBuffer[1] = buffer.m_textureBuffer[3] = (float)(sprite.m_texture[3]) / texture.m_fullHeight;
                	buffer.m_textureBuffer[2] = buffer.m_textureBuffer[6] = (float)(sprite.m_texture[2]) / texture.m_fullWidth;
                	buffer.m_textureBuffer[5] = buffer.m_textureBuffer[7] = (float)(sprite.m_texture[1]) / texture.m_fullHeight;
                } else {
                	buffer.m_textureBuffer[0] = buffer.m_textureBuffer[4] = (float)(sprite.m_texture[0]) / texture.m_fullWidth;
                	buffer.m_textureBuffer[1] = buffer.m_textureBuffer[3] = (float)(sprite.m_texture[1]) / texture.m_fullHeight;
                	buffer.m_textureBuffer[2] = buffer.m_textureBuffer[6] = (float)(sprite.m_texture[2]) / texture.m_fullWidth;
                	buffer.m_textureBuffer[5] = buffer.m_textureBuffer[7] = (float)(sprite.m_texture[3]) / texture.m_fullHeight;
                }*/
				
				buffer.seek(0);
				buffer.writeVertex(0, 0, m_size[0], m_size[1]);
				if (image.m_style == GLImage.GLImageStyleRepeat) {
					buffer.writeTexture(
							0,
							0,
							(m_size[0] / scale) / texture.m_fullWidth,
							(m_size[1] / scale) / texture.m_fullHeight);
				} else if (image.m_style == GLImage.GLImageStyleFlip) {
					buffer.writeTexture(
							(float)(sprite.m_texture[0]) / texture.m_fullWidth,
							(float)(sprite.m_texture[3]) / texture.m_fullHeight,
							(float)(sprite.m_texture[2]) / texture.m_fullWidth,
							(float)(sprite.m_texture[1]) / texture.m_fullHeight);
				} else {
					buffer.writeTexture(
							(float)(sprite.m_texture[0]) / texture.m_fullWidth,
							(float)(sprite.m_texture[1]) / texture.m_fullHeight,
							(float)(sprite.m_texture[2]) / texture.m_fullWidth,
							(float)(sprite.m_texture[3]) / texture.m_fullHeight);
				}
				buffer.update();
			} else {
				bufferIndex = 0;
				
				float wScale = m_size[0] / image.m_width;
				float hScale = m_size[1] / image.m_height;
				
				for (int i = 0; i < image.m_count; i++) {
					if (image.m_list[i].m_handle[0] != 0) {
						GLTexture texture = image.m_list[i];
						int normalCount = texture.m_normalCount;
						if (normalCount > 0) {
							GLTextureBuffer buffer = m_buffers[bufferIndex];
							int index = 0;
							for (int j = 0; j < normalCount; j++) {
								GLSprite sprite = texture.m_normalSprites[j];
								
								/*
								buffer.m_vertexBuffer[index+0] = buffer.m_vertexBuffer[index+4] = (float)m_view[0] + (float)(sprite.m_vertex[0]) * wScale;
								buffer.m_vertexBuffer[index+1] = buffer.m_vertexBuffer[index+3] = (float)m_view[1] + (float)(sprite.m_vertex[1]) * hScale;
								buffer.m_vertexBuffer[index+2] = buffer.m_vertexBuffer[index+6] = (float)m_view[0] + (float)(sprite.m_vertex[2]) * wScale;
								buffer.m_vertexBuffer[index+5] = buffer.m_vertexBuffer[index+7] = (float)m_view[1] + (float)(sprite.m_vertex[3]) * hScale;
								
			                	buffer.m_textureBuffer[index+0] = buffer.m_textureBuffer[index+4] = (float)(sprite.m_texture[0]) / texture.m_fullWidth;
			                	buffer.m_textureBuffer[index+1] = buffer.m_textureBuffer[index+3] = (float)(sprite.m_texture[1]) / texture.m_fullHeight;
			                	buffer.m_textureBuffer[index+2] = buffer.m_textureBuffer[index+6] = (float)(sprite.m_texture[2]) / texture.m_fullWidth;
			                	buffer.m_textureBuffer[index+5] = buffer.m_textureBuffer[index+7] = (float)(sprite.m_texture[3]) / texture.m_fullHeight;
								*/
								
								buffer.seek(index);
								buffer.writeVertex(
										(float)(sprite.m_vertex[0]) * wScale,
										(float)(sprite.m_vertex[1]) * hScale,
										(float)(sprite.m_vertex[2]) * wScale,
										(float)(sprite.m_vertex[3]) * hScale);
								buffer.writeTexture(
										(float)(sprite.m_texture[0]) / texture.m_fullWidth,
										(float)(sprite.m_texture[1]) / texture.m_fullHeight,
										(float)(sprite.m_texture[2]) / texture.m_fullWidth,
										(float)(sprite.m_texture[3]) / texture.m_fullHeight);
								
								index += 8;
							}
							buffer.update();
							bufferIndex++;
						}
						int alphaCount = texture.m_alphaCount;
						if (alphaCount > 0) {
							GLTextureBuffer buffer = m_buffers[bufferIndex];
							int index = 0;
							for (int j = 0; j < alphaCount; j++) {
								GLSprite sprite = texture.m_alphaSprites[j];
								
								/*
								buffer.m_vertexBuffer[index+0] = buffer.m_vertexBuffer[index+4] = (float)m_view[0] + (float)(sprite.m_vertex[0]) * wScale;
								buffer.m_vertexBuffer[index+1] = buffer.m_vertexBuffer[index+3] = (float)m_view[1] + (float)(sprite.m_vertex[1]) * hScale;
								buffer.m_vertexBuffer[index+2] = buffer.m_vertexBuffer[index+6] = (float)m_view[0] + (float)(sprite.m_vertex[2]) * wScale;
								buffer.m_vertexBuffer[index+5] = buffer.m_vertexBuffer[index+7] = (float)m_view[1] + (float)(sprite.m_vertex[3]) * hScale;
								
			                	buffer.m_textureBuffer[index+0] = buffer.m_textureBuffer[index+4] = (float)(sprite.m_texture[0]) / texture.m_fullWidth;
			                	buffer.m_textureBuffer[index+1] = buffer.m_textureBuffer[index+3] = (float)(sprite.m_texture[1]) / texture.m_fullHeight;
			                	buffer.m_textureBuffer[index+2] = buffer.m_textureBuffer[index+6] = (float)(sprite.m_texture[2]) / texture.m_fullWidth;
			                	buffer.m_textureBuffer[index+5] = buffer.m_textureBuffer[index+7] = (float)(sprite.m_texture[3]) / texture.m_fullHeight;
			                	*/
								
								buffer.seek(index);
								buffer.writeVertex(
										(float)(sprite.m_vertex[0]) * wScale,
										(float)(sprite.m_vertex[1]) * hScale,
										(float)(sprite.m_vertex[2]) * wScale,
										(float)(sprite.m_vertex[3]) * hScale);
								buffer.writeTexture(
										(float)(sprite.m_texture[0]) / texture.m_fullWidth,
										(float)(sprite.m_texture[1]) / texture.m_fullHeight,
										(float)(sprite.m_texture[2]) / texture.m_fullWidth,
										(float)(sprite.m_texture[3]) / texture.m_fullHeight);

								index += 8;
							}
							buffer.update();
							bufferIndex++;
						}
					}
				}
			}
        }
	}
	
	// Generate GLRender structure for cropped image
	public final void update(GLImage image, final int x1, final int y1, final int x2, final int y2) throws EGLInvalidResource, EOutOfMemory {
		if ((image.m_style != GLImage.GLImageStyleNormal) && (image.m_style != GLImage.GLImageStyleSmooth))
			throw new EGLInvalidResource();

		m_size[0] = (x2 - x1);
		m_size[1] = (y2 - y1);
		m_alignX[0] = m_alignX[1] = 0;
		m_alignX[2] = m_size[0] / 2.0f;
		m_alignX[3] = m_alignX[4] = m_size[0];
		m_alignY[0] = m_alignY[1] = m_alignY[2] = 0;
		m_alignY[3] = m_size[1] / 2.0f;
		m_alignY[4] = m_alignY[5] = m_alignY[6] = m_size[1];
		m_box[0] = m_box[1] = 0;
		m_box[2] = m_size[0];
		m_box[3] = m_size[1];

		// Calculate count of texture buffers
		int count = 0;
		for (int i = 0; i < image.m_count; i++) {
			if (image.m_list[i].m_handle[0] != 0) {
				GLTexture texture = image.m_list[i];
				int normalCount = 0;
				for (int j = 0; j < texture.m_normalCount; j++)
					normalCount += texture.m_normalSprites[j].updateVisibility(x1, y1, x2, y2);
				if (normalCount > 0) {
					texture.m_visibleNormalCount = normalCount;
					count++;
				}
				int alphaCount = 0;
				for (int j = 0; j < texture.m_alphaCount; j++)
					alphaCount += texture.m_alphaSprites[j].updateVisibility(x1, y1, x2, y2);
				if (alphaCount > 0) {
					texture.m_visibleAlphaCount = alphaCount;
					count++;
				}
			}
		}
		
		updateBuffers(count);
		
		int bufferIndex = 0;
		for (int i = 0; i < image.m_count; i++) {
			if (image.m_list[i].m_handle[0] != 0) {
				GLTexture texture = image.m_list[i];
				int normalCount = texture.m_visibleNormalCount;
				if (normalCount > 0) {
					m_buffers[bufferIndex].m_handle[0] = texture.m_handle[0];
					m_buffers[bufferIndex].m_alpha = false;
					if (m_buffers[bufferIndex].m_count != normalCount)
						m_buffers[bufferIndex].updateBuffers(normalCount);
					bufferIndex++;
				}
				int alphaCount = texture.m_visibleAlphaCount;
				if (alphaCount > 0) {
					m_buffers[bufferIndex].m_handle[0] = texture.m_handle[0];
					m_buffers[bufferIndex].m_alpha = true;
					if (m_buffers[bufferIndex].m_count != alphaCount)
						m_buffers[bufferIndex].updateBuffers(alphaCount);
					bufferIndex++;
				}
			}
		}
		
		bufferIndex = 0;
		int[] v;
		int[] t;
		try {
			v = new int[4];
			t = new int[4];
		} catch (OutOfMemoryError e) {
			throw new EOutOfMemory(e);
		}
		for (int i = 0; i < image.m_count; i++) {
			if (image.m_list[i].m_handle[0] != 0) {
				GLTexture texture = image.m_list[i];
				if (texture.m_visibleNormalCount > 0) {
					GLTextureBuffer buffer = m_buffers[bufferIndex];
					int index = 0;
					for (int j = 0; j < texture.m_normalCount; j++) {
						GLSprite sprite = texture.m_normalSprites[j];
						if (sprite.m_visible) {
							sprite.updateVisibleRect(v, t, x1, y1, x2, y2);
							
							/*
							buffer.m_vertexBuffer[index+0] = buffer.m_vertexBuffer[index+4] = (float)m_view[0] + v[0];
							buffer.m_vertexBuffer[index+1] = buffer.m_vertexBuffer[index+3] = (float)m_view[1] + v[1];
							buffer.m_vertexBuffer[index+2] = buffer.m_vertexBuffer[index+6] = (float)m_view[0] + v[2];
							buffer.m_vertexBuffer[index+5] = buffer.m_vertexBuffer[index+7] = (float)m_view[1] + v[3];
							
		                	buffer.m_textureBuffer[index+0] = buffer.m_textureBuffer[index+4] = (float)(t[0]) / texture.m_fullWidth;
		                	buffer.m_textureBuffer[index+1] = buffer.m_textureBuffer[index+3] = (float)(t[1]) / texture.m_fullHeight;
		                	buffer.m_textureBuffer[index+2] = buffer.m_textureBuffer[index+6] = (float)(t[2]) / texture.m_fullWidth;
		                	buffer.m_textureBuffer[index+5] = buffer.m_textureBuffer[index+7] = (float)(t[3]) / texture.m_fullHeight;
		                	*/
							
							buffer.seek(index);
							buffer.writeVertex(v[0], v[1], v[2], v[3]);
							buffer.writeTexture(
									(float)(t[0]) / texture.m_fullWidth,
									(float)(t[1]) / texture.m_fullHeight,
									(float)(t[2]) / texture.m_fullWidth,
									(float)(t[3]) / texture.m_fullHeight);

							index += 8;
						}
					}
					buffer.update();
					bufferIndex++;
				}
				if (texture.m_visibleAlphaCount > 0) {
					GLTextureBuffer buffer = m_buffers[bufferIndex];
					int index = 0;
					for (int j = 0; j < texture.m_alphaCount; j++) {
						GLSprite sprite = texture.m_alphaSprites[j];
						if (sprite.m_visible) { 
							sprite.updateVisibleRect(v, t, x1, y1, x2, y2);
							
							/*
							buffer.m_vertexBuffer[index+0] = buffer.m_vertexBuffer[index+4] = (float)m_view[0] + v[0];
							buffer.m_vertexBuffer[index+1] = buffer.m_vertexBuffer[index+3] = (float)m_view[1] + v[1];
							buffer.m_vertexBuffer[index+2] = buffer.m_vertexBuffer[index+6] = (float)m_view[0] + v[2];
							buffer.m_vertexBuffer[index+5] = buffer.m_vertexBuffer[index+7] = (float)m_view[1] + v[3];
							
		                	buffer.m_textureBuffer[index+0] = buffer.m_textureBuffer[index+4] = (float)(t[0]) / texture.m_fullWidth;
		                	buffer.m_textureBuffer[index+1] = buffer.m_textureBuffer[index+3] = (float)(t[1]) / texture.m_fullHeight;
		                	buffer.m_textureBuffer[index+2] = buffer.m_textureBuffer[index+6] = (float)(t[2]) / texture.m_fullWidth;
		                	buffer.m_textureBuffer[index+5] = buffer.m_textureBuffer[index+7] = (float)(t[3]) / texture.m_fullHeight;
		                	*/
							
							buffer.seek(index);
							buffer.writeVertex(v[0], v[1], v[2], v[3]);
							buffer.writeTexture(
									(float)(t[0]) / texture.m_fullWidth,
									(float)(t[1]) / texture.m_fullHeight,
									(float)(t[2]) / texture.m_fullWidth,
									(float)(t[3]) / texture.m_fullHeight);

							index += 8;
						}
					}
					buffer.update();
					bufferIndex++;
				}
			}
		}
	}
	
	private final void updateSDFConst(final float distance, final float scale) {
		m_sdfConst[0] = 1.0f / distance;
		m_sdfConst[1] = m_sdfConst[0] * 0.5f;
		m_sdfConst[2] = m_sdfConst[0] * 1.5f;
		m_sdfConst[3] = m_sdfConst[0] * ((distance / 2.0f) - 1.0f);

		m_sdfParams[0] = m_sdfConst[1] / (float)Math.pow(scale, 1.5f);
		if (m_sdfParams[0] < 0.005f) m_sdfParams[0] = 0.005f;
		if (m_sdfParams[0] > 0.2f) m_sdfParams[0] = 0.2f;
		
		if (scale > 1.0f) {
			m_sdfParams[1] = 0.5f - m_sdfConst[0];
		} else {
			m_sdfParams[1] = 0.5f - m_sdfConst[1] - (m_sdfConst[1] / scale);
		}
		if (m_sdfParams[1] < m_sdfConst[2]) m_sdfParams[1] = m_sdfConst[2];
	}

	public final void updateSDFParams(final float scale, final float edgeOffset, final float edgeSize) {
		if (scale > 1.0f) {
			m_sdfParams[2] = m_sdfParams[1] + edgeOffset * m_sdfConst[0] / scale;
		} else {
			m_sdfParams[2] = m_sdfParams[1] + edgeOffset * m_sdfConst[0] / (float)Math.pow(scale, 0.3f);
		}
		if (m_sdfParams[2] < m_sdfConst[1]) m_sdfParams[2] = m_sdfConst[1];
		if (m_sdfParams[2] > (1.0f - m_sdfConst[1])) m_sdfParams[2] = 1.0f - m_sdfConst[1];
		
		if (edgeSize <= 0.0f) {
			m_sdfParams[3] = m_sdfParams[2];
		} else {
			m_sdfParams[3] = m_sdfParams[2] + edgeSize * m_sdfConst[0] / scale;
			if (m_sdfParams[3] > 1.0f) m_sdfParams[3] = 1.0f;
		}
	}
	
	// Generate GLRender structure for Signed Distance Field Image
	public final void update(GLImage image, final float scale) throws EGLInvalidResource, EOutOfMemory {
		if (image.m_style != GLImage.GLImageStyleDistanceField)
			throw new EGLInvalidResource();

		GLTexture texture = image.m_list[0];
		
		updateBuffers(1);

		GLTextureBuffer buffer = m_buffers[0];
		buffer.updateBuffers(1);
		buffer.m_handle[0] = texture.m_handle[0];
		
		GLFontStyle style = image.m_styles[0];
		GLFontGlyphMetrics glyph = style.m_defaultGlyph;
		
		final float vx1 = glyph.m_outX * scale;
		final float vy1 = (style.m_ascent + glyph.m_outY) * scale;
		final float vx2 = vx1 + glyph.m_width * scale;
		final float vy2 = vy1 + glyph.m_height * scale;
		final float tx1 = glyph.m_x / (float)texture.m_fullWidth;
		final float ty1 = glyph.m_y / (float)texture.m_fullHeight;
		final float tx2 = tx1 + glyph.m_width / (float)texture.m_fullWidth;
		final float ty2 = ty1 + glyph.m_height / (float)texture.m_fullHeight;
		
		buffer.seek(0);
		buffer.writeVertex(vx1, vy1, vx2, vy2);
		buffer.writeTexture(tx1, ty1, tx2, ty2);
		buffer.update();
		
		m_size[0] = (glyph.m_width - (float)image.m_distance) * scale;
		m_size[1] = (glyph.m_height - (float)image.m_distance) * scale;
		m_alignX[0] = m_alignX[1] = 0;
		m_alignX[2] = m_size[0] / 2.0f;
		m_alignX[3] = m_alignX[4] = m_size[0];
		m_alignY[0] = m_alignY[1] = m_alignY[2] = 0;
		m_alignY[3] = m_size[1] / 2.0f;
		m_alignY[4] = m_alignY[5] = m_alignY[6] = m_size[1];
		m_box[0] = m_box[1] = 0;
		m_box[2] = m_size[0];
		m_box[3] = m_size[1];
		
		updateSDFConst(image.m_distance, scale);
	}
	
	// Generate GLRender structure for Signed Distance Field Font
	public final void update(GLImage image, final String text, final float scale, int style) throws EGLInvalidResource, EOutOfMemory {
		if (image.m_style != GLImage.GLImageStyleDistanceField)
			throw new EGLInvalidResource();
		
		if (style >= image.m_stylesCount)
			style = 0;
		
		GLTexture texture = image.m_list[0];
		
		updateBuffers(1);
		
		int textLen = text.length();
		GLTextureBuffer buffer = m_buffers[0];
		buffer.updateBuffers(textLen);
		buffer.m_handle[0] = texture.m_handle[0];
		buffer.seek(0);
		
		GLFontStyle fontStyle = image.m_styles[style];
		
		float x = 0;
		float y = (float)Math.ceil(fontStyle.m_ascent * scale);
		float halfDistance = (float)image.m_distance / 2.0f;
		GLFontGlyphMetrics prevGlyph = fontStyle.m_defaultGlyph;
		
		for (int i = 0; i < textLen; i++) {
			GLFontGlyphMetrics glyph = fontStyle.findGlyph(text.charAt(i));
			
			float kx = 0;
			if ((prevGlyph != null) && (prevGlyph.m_kerningPairsCount != 0)) {
				Float kernAmount = prevGlyph.m_kerningPairs.get(glyph.m_glyph);
				if (kernAmount != null)
					kx = kernAmount.floatValue() * scale;
			}
			
			float vx1 = x + kx + glyph.m_outX * scale;
			float vy1 = y + glyph.m_outY * scale;
			float vx2 = vx1 + glyph.m_width * scale;
			float vy2 = vy1 + glyph.m_height * scale;
			float tx1 = glyph.m_x / (float)texture.m_fullWidth;
			float ty1 = glyph.m_y / (float)texture.m_fullHeight;
			float tx2 = tx1 + glyph.m_width / (float)texture.m_fullWidth;
			float ty2 = ty1 + glyph.m_height / (float)texture.m_fullHeight;
			
			if (i == 0) {
				m_box[0] = vx1 + halfDistance;
				m_box[1] = vy1 + halfDistance;
				m_box[2] = vx2 - halfDistance;
				m_box[3] = vy2 - halfDistance;
			} else {
				if (m_box[0] > (vx1 + halfDistance))
					m_box[0] = vx1 + halfDistance;
				if (m_box[1] > (vy1 + halfDistance))
					m_box[1] = vy1 + halfDistance;
				if (m_box[2] < (vx2 - halfDistance))
					m_box[2] = vx2 - halfDistance;
				if (m_box[3] < (vy2 - halfDistance))
					m_box[3] = vy2 - halfDistance;
			}
			
			buffer.writeVertex(vx1, vy1, vx2, vy2);
			buffer.writeTexture(tx1, ty1, tx2, ty2);
			buffer.next();
			
			x += (float)glyph.m_nextX * scale;
			y += (float)glyph.m_nextY * scale;

			prevGlyph = glyph;
		}
		buffer.update();
		
		m_size[0] = x;
		m_size[1] = (int)Math.ceil(fontStyle.m_height * scale);
		m_alignX[0] = 0;
		m_alignX[1] = m_box[0];
		m_alignX[2] = m_size[0] / 2.0f;
		m_alignX[3] = m_box[2];
		m_alignX[4] = m_size[0];
		m_alignY[0] = 0;
		m_alignY[1] = m_box[1];
		m_alignY[2] = (fontStyle.m_ascent - (float)image.m_size) * scale;
		m_alignY[3] = m_alignY[1] + ((float)image.m_size * scale / 2.0f);
		m_alignY[4] = fontStyle.m_ascent * scale;
		m_alignY[5] = m_box[3];
		m_alignY[6] = m_size[1];
		
		updateSDFConst(image.m_distance, scale);
	}
}
