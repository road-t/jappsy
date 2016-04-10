package com.jappsy.opengl.image;

import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Locale;

import com.jappsy.Jappsy;
import com.jappsy.cipher.MD5;
import com.jappsy.exceptions.EGLInvalidResource;
import com.jappsy.exceptions.EIO;
import com.jappsy.exceptions.EOutOfMemory;
import com.jappsy.gui.JAlignX;
import com.jappsy.gui.JPaint;
import com.jappsy.gui.JPaintColorFilter;
import com.jappsy.gui.JAlignY;
import com.jappsy.io.CacheFile;
import com.jappsy.io.ImageReader;
import com.jappsy.memory.MemoryManager;
import com.jappsy.opengl.GLContext;
import com.jappsy.opengl.font.GLFontGlyphMetrics;
import com.jappsy.opengl.font.GLFontStyle;

public class GLImage {
	public static final int GLImageStyleAuto = -1;
	public static final int GLImageStyleNormal = 0;
    public static final int GLImageStyleSmooth = 1;			// .s.
    public static final int GLImageStyleFlip = 2;			// .f.
    public static final int GLImageStyleRepeat = 3;			// .r.
    public static final int GLImageStylePatch9 = 4;			// .9.
    public static final int GLImageStyleDistanceField = 5;
	
	public long m_memory;						// Image memory usage
	public int m_style;							// Image style
	public int m_width;							// Image width
	public int m_height;						// Image height
	
	// Signed Distance Field Params
	public String m_name;						// Name of Glyph Set
	public int m_size;							// Glyph Size
	public int m_distance;						// Image distance for Signed Distance Field Images
	public int m_stylesCount;
	public GLFontStyle[] m_styles;				// Different Styles (from one up to four)
	
	// GUI Params
	public int[] m_padding = new int[4];		// Image padding
	
	// OpenGL Textures
	public int m_count;
	public GLTexture[] m_list;
	
	private int m_renderCacheLimit = 50;
	private LinkedHashMap<String, GLRender> m_renderCache = new LinkedHashMap<String, GLRender>();
	
	private static final MemoryManager m_mem = new MemoryManager();
	
	public static final GLImage create(CacheFile file) throws EOutOfMemory, EGLInvalidResource, EIO {
		GLImage o = (GLImage)(m_mem.getObject());
		if (o == null) {
			try {
				o = (GLImage)(m_mem.addObject(new GLImage()));
			} catch (OutOfMemoryError e) {
				throw new EOutOfMemory(e);
			}
		}
		
		try {
			o.initFromCache(file);
		} catch (EOutOfMemory e) {
			o.release();
			throw e;
		} catch (EGLInvalidResource e) {
			o.release();
			throw e;
		} catch (EIO e) {
			o.release();
			throw e;
		}
		
		return o;
	}
	
	public void release() throws EOutOfMemory {
		clear();
		m_mem.returnObject(this);
	}
	
	protected final void clear() throws EOutOfMemory {
		m_memory = 0;
		m_style = GLImageStyleAuto;
		m_width = m_height = 0;
		
		m_name = null;
		m_size = m_distance = 0;
		if ((m_stylesCount != 0) && (m_styles != null)) {
			GLFontStyle.releaseArray(m_styles, m_stylesCount);
		}
		m_styles = null;
		m_stylesCount = 0;
		
		m_padding[0] = m_padding[1] = m_padding[2] = m_padding[3] = -1;
		
		if ((m_count != 0) && (m_list != null)) {
			GLTexture.releaseArray(m_list, m_count);
		}
		m_list = null;
		m_count = 0;
		
		Iterator<GLRender> it = m_renderCache.values().iterator();
		EOutOfMemory error = null;
		while (it.hasNext()) {
			try {
				it.next().release();
			} catch (EOutOfMemory e) {
				error = e;
			}
			it.remove();
		}
		m_renderCache.clear();
		
		if (error != null)
			throw error;
	}
	
	private static final int nextPowerOfTwo(int v) {
	    v--;
	    v |= v >> 1;
	    v |= v >> 2;
	    v |= v >> 4;
	    v |= v >> 8;
	    v |= v >> 16;
	    v++;
	    return v;
	}

	protected final void initFromCache(CacheFile file) throws EOutOfMemory, EIO, EGLInvalidResource {
		clear();
		
		try {
			String name = file.getName().toLowerCase(Locale.ENGLISH);
			if (name.endsWith(".sdff") || name.endsWith(".sdfi")) {
				m_style = GLImageStyleDistanceField;
			} else if (name.endsWith(".9.jimg")) {
				m_style = GLImageStylePatch9;
			} else if (name.endsWith(".s.jimg") || name.endsWith(".s.png") || name.endsWith(".s.jpg") || name.endsWith(".s.jpeg")) {
				m_style = GLImageStyleSmooth;
			} else if (name.endsWith(".f.jimg") || name.endsWith(".f.png") || name.endsWith(".f.jpg") || name.endsWith(".f.jpeg")) {
				m_style = GLImageStyleFlip;
			} else if (name.endsWith(".r.jimg") || name.endsWith(".r.png") || name.endsWith(".r.jpg") || name.endsWith(".r.jpeg")) {
				m_style = GLImageStyleRepeat;
			} else {
				m_style = GLImageStyleNormal;
			}
	
			int whd[] = new int[3];
			int head = ImageReader.getSize(file, whd);
			
	        m_width = whd[0];
	        m_height = whd[1];
	        m_distance = whd[2];
	        
	        int widthSplit;
	        int heightSplit;
	    	int maxTextureSize = GLContext.getMaxTextureSize();
	    	if (m_style == GLImageStylePatch9) {
	    		m_width -= 2;
	    		m_height -= 2;
	    		widthSplit = m_width;
	    		heightSplit = m_height;
	        	if ((widthSplit > maxTextureSize) || (heightSplit > maxTextureSize))
	        		throw new EGLInvalidResource();
	    	} else if (m_style == GLImageStyleDistanceField) {
	    		widthSplit = m_width;
	    		heightSplit = m_height;
	        	if ((widthSplit > maxTextureSize) || (heightSplit > maxTextureSize))
	        		throw new EGLInvalidResource();
	        } else {
	        	widthSplit = 1024;
	        	heightSplit = 1024;
	        	if (widthSplit > maxTextureSize) widthSplit = maxTextureSize;
	            if (heightSplit > maxTextureSize) heightSplit = maxTextureSize;
	        }
	        
	        int widthCount = (m_width + (widthSplit - 1)) / widthSplit;
	        int heightCount = (m_height + (heightSplit - 1)) / heightSplit;
	        int totalCount = (widthCount * heightCount);
	        m_memory = 0;
	
	        if ((m_style == GLImageStylePatch9) && ((m_width < 1) || (m_height < 1)))
	        	throw new EGLInvalidResource();
	        if ((m_style == GLImageStyleRepeat) && (totalCount > 1))
	        	throw new EGLInvalidResource();
	        
	       	m_list = GLTexture.createArray(totalCount);
	    	m_count = totalCount;
	
	        if (m_style == GLImageStylePatch9) {
	    		GLTexture texture = m_list[0];
	        	texture.m_count = 9;
	        	texture.m_cropX = texture.m_cropY = 1;
	        	texture.m_cropWidth = m_width;
	        	texture.m_cropHeight = m_height;
	        	
	        	if (GLContext.getNPOTSupport()) {
	        		texture.m_fullWidth = m_width;
	            	texture.m_fullHeight = m_height;
	            } else {
	            	texture.m_fullWidth = nextPowerOfTwo(m_width);
	            	texture.m_fullHeight = nextPowerOfTwo(m_height);
	            }
	
	       		texture.m_sprites = GLSprite.createArray(9);
	
	       		int[] pixelData = ImageReader.getPixels(file, null);
	        		
	    		int dataWidth = m_width + 2;
	    		int dataHeight = m_height + 2;
	    		
	    		int ptrTop = 1;
	    		int ptrBottom = 1 + (dataHeight - 1) * dataWidth;
	    		int ptrLeft = dataWidth;
	    		int ptrRight = dataWidth + (dataWidth - 1);
	    		
	    		int w = dataWidth - 1;
	    		int h = dataWidth - 1;
	    		
	            int x = 1;
	            // search top start
	            while (x < w) {
	                if ((pixelData[ptrTop] & 0xFF000000) != 0) break;
	                x++; ptrTop++;
	            }
	            if (x < w) {
	            	texture.m_sprites[0].m_texture[0] = texture.m_sprites[3].m_texture[0] = texture.m_sprites[6].m_texture[0] = 1;
	            	texture.m_sprites[0].m_texture[2] = texture.m_sprites[3].m_texture[2] = texture.m_sprites[6].m_texture[2] = x;
	            	texture.m_sprites[1].m_texture[0] = texture.m_sprites[4].m_texture[0] = texture.m_sprites[7].m_texture[0] = x;
	                
	                // search top end
	                while (x < w) {
	                    if ((pixelData[ptrTop] & 0xFF000000) == 0) break;
	                    x++; ptrTop++;
	                }
	                if (x < w) {
	                	texture.m_sprites[1].m_texture[2] = texture.m_sprites[4].m_texture[2] = texture.m_sprites[7].m_texture[2] = x;
	                	texture.m_sprites[2].m_texture[0] = texture.m_sprites[5].m_texture[0] = texture.m_sprites[8].m_texture[0] = x;
	                	texture.m_sprites[2].m_texture[2] = texture.m_sprites[5].m_texture[2] = texture.m_sprites[8].m_texture[2] = w;
	                } else {
	                	texture.m_sprites[1].m_texture[2] = texture.m_sprites[4].m_texture[2] = texture.m_sprites[7].m_texture[2] = w;
	                }
	            } else {
	            	texture.m_sprites[1].m_texture[0] = texture.m_sprites[4].m_texture[0] = texture.m_sprites[7].m_texture[0] = 1;
	            	texture.m_sprites[1].m_texture[2] = texture.m_sprites[4].m_texture[2] = texture.m_sprites[7].m_texture[2] = w;
	            }
	            
	            int y = 1;
	            // search left start
	            while (y < h) {
	                if ((pixelData[ptrLeft] & 0xFF000000) != 0) break;
	                y++; ptrLeft += dataWidth;
	            }
	            if (y < h) {
	            	texture.m_sprites[0].m_texture[1] = texture.m_sprites[1].m_texture[1] = texture.m_sprites[2].m_texture[1] = 1;
	            	texture.m_sprites[0].m_texture[3] = texture.m_sprites[1].m_texture[3] = texture.m_sprites[2].m_texture[3] = y;
	            	texture.m_sprites[3].m_texture[1] = texture.m_sprites[4].m_texture[1] = texture.m_sprites[5].m_texture[1] = y;
	                
	                //search left end
	                while (y < h) {
	                    if ((pixelData[ptrLeft] & 0xFF000000) == 0) break;
	                    y++; ptrLeft += dataWidth;
	                }
	                if (y < h) {
	                	texture.m_sprites[3].m_texture[3] = texture.m_sprites[4].m_texture[3] = texture.m_sprites[5].m_texture[3] = y;
	                	texture.m_sprites[6].m_texture[1] = texture.m_sprites[7].m_texture[1] = texture.m_sprites[8].m_texture[1] = y;
	                	texture.m_sprites[6].m_texture[3] = texture.m_sprites[7].m_texture[3] = texture.m_sprites[8].m_texture[3] = h;
	                } else {
	                	texture.m_sprites[3].m_texture[3] = texture.m_sprites[4].m_texture[3] = texture.m_sprites[5].m_texture[3] = h;
	                }
	            } else {
	            	texture.m_sprites[3].m_texture[1] = texture.m_sprites[4].m_texture[1] = texture.m_sprites[5].m_texture[1] = 1;
	            	texture.m_sprites[3].m_texture[3] = texture.m_sprites[4].m_texture[3] = texture.m_sprites[5].m_texture[3] = h;
	            }
	            
	            x = 1;
	            // search bottom start
	            while (x < w) {
	                if ((pixelData[ptrBottom] & 0xFF000000) != 0) break;
	                x++; ptrBottom++;
	            }
	            if (x < w) {
	                m_padding[0] = x - 1;
	                
	                // search bottom end
	                while (x < dataWidth) {
	                    if ((pixelData[ptrBottom] & 0xFF000000) == 0) break;
	                    x++; ptrBottom++;
	                }
	                if (x < dataWidth) {
	                    m_padding[2] = (dataWidth - x - 1);
	                } else {
	                    m_padding[2] = -1;
	                }
	            } else {
	                m_padding[0] = -1;
	                m_padding[2] = -1;
	            }
	            
	            y = 1;
	            // search right start
	            while (y < h) {
	                if ((pixelData[ptrRight] & 0xFF000000) != 0) break;
	                y++; ptrRight += dataWidth;
	            }
	            if (y < h) {
	                m_padding[1] = y - 1;
	                
	                // search right end
	                while (y < dataHeight) {
	                    if ((pixelData[ptrRight] & 0xFF000000) == 0) break;
	                    y++; ptrRight += dataWidth;
	                }
	                if (y < dataHeight) {
	                    m_padding[3] = (dataHeight - y - 1);
	                } else {
	                    m_padding[3] = -1;
	                }
	            } else {
	                m_padding[1] = -1;
	                m_padding[3] = -1;
	            }
	            
	            pixelData = null;
	            
	            for (int i = 0; i < 9; i++) {
	            	GLSprite sprite = texture.m_sprites[i];
	                if ((sprite.m_texture[0] == 0) ||
	                    (sprite.m_texture[1] == 0) ||
	                    (sprite.m_texture[2] == 0) ||
	                    (sprite.m_texture[3] == 0)
	                ) {
	                	sprite.m_texture[0] = sprite.m_texture[1] = sprite.m_texture[2] = sprite.m_texture[3] = 0;
	                } else {
	                	sprite.m_texture[0]--;
	                	sprite.m_texture[1]--;
	                	sprite.m_texture[2]--;
	                	sprite.m_texture[3]--;
	                }
	                
	                sprite.m_vertex[0] = sprite.m_texture[0];
	                sprite.m_vertex[1] = sprite.m_texture[1];
	                sprite.m_vertex[2] = sprite.m_texture[2];
	                sprite.m_vertex[3] = sprite.m_texture[3];
	            }
	            
	            m_memory += (long)(texture.m_fullWidth) * texture.m_fullHeight * 4;
	        } else {
	        	int textureIndex = 0;
	            for (int y = 0; y < heightCount; y++) {
	                int cropHeight = m_height - (y * heightSplit);
	                if (cropHeight > heightSplit) cropHeight = heightSplit;
	                for (int x = 0; x < widthCount; x++) {
	                    int cropWidth = m_width - (x * widthSplit);
	                    if (cropWidth > widthSplit) cropWidth = widthSplit;
	                    
	                    GLTexture texture = m_list[textureIndex];
	                    texture.m_count = 1;
	                    texture.m_cropX = x * widthSplit;
	                    texture.m_cropY = y * widthSplit;
	                    texture.m_cropWidth = cropWidth;
	                    texture.m_cropHeight = cropHeight;
	                    
	                	if (GLContext.getNPOTSupport()) {
	                		texture.m_fullWidth = cropWidth;
	                    	texture.m_fullHeight = cropHeight;
	                    } else {
	                    	texture.m_fullWidth = nextPowerOfTwo(cropWidth);
	                    	texture.m_fullHeight = nextPowerOfTwo(cropHeight);
	                    }
	                	
	               		texture.m_sprites = GLSprite.createArray(1);
	               		
	               		GLSprite sprite = texture.m_sprites[0];
	               		
	               		sprite.m_vertex[0] = texture.m_cropX;
	               		sprite.m_vertex[1] = texture.m_cropY;
	               		sprite.m_vertex[2] = sprite.m_vertex[0] + cropWidth;
	               		sprite.m_vertex[3] = sprite.m_vertex[1] + cropHeight;
	               		
	               		sprite.m_texture[0] = sprite.m_texture[1] = 0;
	               		sprite.m_texture[2] = cropWidth;
	               		sprite.m_texture[3] = cropHeight;
	               		
	               		m_memory += (long)texture.m_fullWidth * texture.m_fullHeight * 4;
	               		
	               		textureIndex++;
	                }
	            }
	        }
	        
	        for (int i = 0; i < m_count; i++) {
	        	GLTexture texture = m_list[i];
	
	        	int[] pixelData = ImageReader.getPixels(file, texture.m_fullWidth, texture.m_fullHeight, texture.m_cropX, texture.m_cropY, texture.m_cropX + texture.m_cropWidth, texture.m_cropY + texture.m_cropHeight);
	        	
	        	if (m_style != GLImage.GLImageStyleDistanceField) {
	        		for (int j = 0; j < texture.m_count; j++) {
	        			GLSprite sprite = texture.m_sprites[j];
	        			if ((sprite.m_texture[0] != 0) ||
	        				(sprite.m_texture[1] != 0) ||
							(sprite.m_texture[2] != 0) ||
							(sprite.m_texture[3] != 0)
						) {
	        				int w = sprite.m_texture[2] - sprite.m_texture[0];
	        				int h = sprite.m_texture[3] - sprite.m_texture[1];
	        				int ptrIndex = sprite.m_texture[1] * texture.m_fullWidth + sprite.m_texture[0];
	                		int lineSkip = texture.m_fullWidth - w;
	                		for (int yy = h; yy > 0; yy--) {
	                			for (int xx = w; xx > 0; xx--) {
	                				if ((pixelData[ptrIndex] & 0xFF000000) != 0xFF000000) {
	                					sprite.m_alpha = true;
	                					break;
	                				}
	                				ptrIndex++;
	                			}
	                			if (sprite.m_alpha) break;
	                			ptrIndex += lineSkip;
	                		}
	        			}
	        		}
	        	}
	        	
	        	texture.createSprites();
	        	
	        	IntBuffer buffer;
	        	try {
	        		buffer = IntBuffer.wrap(pixelData);
	        	} catch (OutOfMemoryError e) {
	        		throw new EOutOfMemory(e);
	        	}
	        	texture.m_handle[0] = GLContext.createTexture(buffer, m_style, texture.m_fullWidth, texture.m_fullHeight);
	        	pixelData = null;
	        	buffer = null;
	        }
	        
	        if (head == ImageReader.sdfiHead) {
	        	m_name = "Signed Distance Field Image";
				m_size = m_height - m_distance; // Real Height

				m_styles = GLFontStyle.createArray(1);
				GLFontStyle style = m_styles[0];
				style.m_height = m_size;
				style.m_ascent = m_size;
				style.m_descent = 0;
				style.m_glyphIndexTable = " ";
				
				style.m_glyphMetrics = GLFontGlyphMetrics.createArray(1);
				style.m_count = 1;
				GLFontGlyphMetrics glyphMetrics = style.m_glyphMetrics[0];
				glyphMetrics.m_glyph = 32;
				glyphMetrics.m_x = glyphMetrics.m_y = 0;
				glyphMetrics.m_width = m_width;
				glyphMetrics.m_height = m_height;
				glyphMetrics.m_outX = -(float)m_distance / 2.0f;
				glyphMetrics.m_outY = glyphMetrics.m_outX - (float)m_size;
				glyphMetrics.m_nextX = m_width - m_distance;
				glyphMetrics.m_nextY = 0;
				glyphMetrics.createPairs(0);
				style.m_defaultGlyph = glyphMetrics;
	        } else if (head == ImageReader.sdffHead) {
	        	ByteBuffer buffer = ImageReader.getFontInfo(file, whd);

	        	m_name = "";
				boolean skip = false;
				int nameLen = 0x00FFFF & buffer.getChar();
				for (int i = 0; i < nameLen; i++) {
					char ch = buffer.getChar();
					if ((!skip) && (ch != 0))
						m_name += String.valueOf(ch);
					else
						skip = true;
				}
					
				m_size = 0x00FF & buffer.get();
				m_stylesCount = 0x00FF & buffer.get();
				m_styles = GLFontStyle.createArray(m_stylesCount);
				
				for (int i = 0; i < m_stylesCount; i++) {
					GLFontStyle style = m_styles[i];

					style.m_height = buffer.getFloat();
					style.m_ascent = buffer.getFloat();
					style.m_descent = buffer.getFloat();

					int glyphIndexCount = 0x00FFFF & buffer.getShort();
					style.m_glyphMetrics = GLFontGlyphMetrics.createArray(glyphIndexCount);
					style.m_count = glyphIndexCount;
					
					String glyphIndexTable = new String("");
					for (int j = 0; j < glyphIndexCount; j++) {
						glyphIndexTable += String.valueOf(buffer.getChar());
					}
					style.m_glyphIndexTable = glyphIndexTable;
					
					GLFontGlyphMetrics defaultGlyph = null;
					for (int j = 0; j < glyphIndexCount; j++) {
						GLFontGlyphMetrics glyphMetrics = style.m_glyphMetrics[j];
						glyphMetrics.m_glyph = glyphIndexTable.charAt(j);
						glyphMetrics.m_x = buffer.getFloat();
						glyphMetrics.m_y = buffer.getFloat();
						glyphMetrics.m_width = buffer.getFloat();
						glyphMetrics.m_height = buffer.getFloat();
						glyphMetrics.m_outX = buffer.getFloat();
						glyphMetrics.m_outY = buffer.getFloat();
						glyphMetrics.m_nextX = buffer.getFloat();
						glyphMetrics.m_nextY = buffer.getFloat();
						glyphMetrics.createPairs(0x00FF & buffer.get());
						for (int k = 0; k < glyphMetrics.m_kerningPairsCount; k++) {
							int wSecond = 0x00FFFF & buffer.getShort();
							float kernAmount = buffer.getFloat();
							glyphMetrics.m_kerningPairs.put(wSecond, Float.valueOf(kernAmount));
						}
						if (defaultGlyph == null) defaultGlyph = glyphMetrics;
						else if (glyphMetrics.m_glyph == (char)32) defaultGlyph = glyphMetrics;
					}
					style.m_defaultGlyph = defaultGlyph;
				}
	        }
		} catch (EOutOfMemory e) {
			clear();
			throw e;
		} catch (EIO e) {
			clear();
			throw e;
		} catch (EGLInvalidResource e) {
			clear();
			throw e;
		}
	}

	protected final void initFromHandle(int[] handle, int style, int cropWidth, int cropHeight, int fullWidth, int fullHeight) throws EOutOfMemory {
		clear();
		
		try {
			m_style = style;
			m_width = cropWidth;
			m_height = cropHeight;
			m_distance = 0;
			
			m_list = GLTexture.createArray(1);
			m_count = 1;
			
			GLTexture texture = m_list[0];
	    	texture.m_count = 1;
	    	texture.m_cropX = texture.m_cropY = 0;
	    	texture.m_cropWidth = m_width;
	    	texture.m_cropHeight = m_height;
	   		texture.m_fullWidth = fullWidth;
	       	texture.m_fullHeight = fullHeight;
	
	   		texture.m_sprites = GLSprite.createArray(1);
	   		
	   		GLSprite sprite = texture.m_sprites[0];
	   		sprite.m_alpha = true;
	   		sprite.m_vertex[0] = texture.m_cropX;
	   		sprite.m_vertex[1] = texture.m_cropY;
	   		sprite.m_vertex[2] = sprite.m_vertex[0] + cropWidth;
	   		sprite.m_vertex[3] = sprite.m_vertex[1] + cropHeight;
	   		sprite.m_texture[0] = sprite.m_texture[1] = 0;
	   		sprite.m_texture[2] = cropWidth;
	   		sprite.m_texture[3] = cropHeight;
	   		
	   		m_memory += (long)texture.m_fullWidth * texture.m_fullHeight * 4;
	   		
	   		texture.createSprites();
	    	
	    	texture.m_handle[0] = handle[0];
		} catch (EOutOfMemory e) {
			clear();
			throw e;
		}
	}
	
	private final GLRender createRender(final String hash) throws EOutOfMemory {
		GLRender render;
		
		if (m_renderCache.size() >= m_renderCacheLimit) {
			Iterator<GLRender> it = m_renderCache.values().iterator();
			render = it.next();
			it.remove();
		} else {
			render = GLRender.create();
		}
		
		m_renderCache.put(hash, render);
		return render;
	}
	
	public final GLRender render(GLContext ctx, final int x, final int y, JPaint paint) throws EOutOfMemory, EGLInvalidResource {
		String hash = Float.toString(paint.m_scale);
		GLRender render = m_renderCache.get(hash);
		if (render == null) {
			render = createRender(hash);
			render.update(this, m_width, m_height, paint.m_scale);
		}
		ctx.addOffset(x, y);
		ctx.drawTexture(render, paint.m_colorFilter == JPaintColorFilter.PAINT_COLOR_FILTER_NEGATIVE, paint.m_antialias);
		ctx.subOffset(x, y);
		return render;
	}
	
	public final GLRender render(GLContext ctx, final int x, final int y, final int width, final int height, JPaint paint) throws EOutOfMemory, EGLInvalidResource {
		String hash = width + "," + height + "," + paint.m_scale;
		GLRender render = m_renderCache.get(hash);
		if (render == null) {
			render = createRender(hash);
			render.update(this, width, height, paint.m_scale);
		}
		ctx.addOffset(x, y);
		ctx.drawTexture(render, paint.m_colorFilter == JPaintColorFilter.PAINT_COLOR_FILTER_NEGATIVE, paint.m_antialias);
		ctx.subOffset(x, y);
		return render;
	}
	
	public final GLRender render(GLContext ctx, final int x, final int y, final int x1, final int y1, final int x2, final int y2, JPaint paint) throws EOutOfMemory, EGLInvalidResource {
		String hash = x1 + "," + y1 + "," + x2 + "," + y2;
		GLRender render = m_renderCache.get(hash);
		if (render == null) {
			render = createRender(hash);
			render.update(this, x1, y1, x2, y2);
		}
		ctx.addOffset(x, y);
		ctx.drawTexture(render, paint.m_colorFilter == JPaintColorFilter.PAINT_COLOR_FILTER_NEGATIVE, paint.m_antialias);
		ctx.subOffset(x, y);
		return render;
	}
	
	private static int[] sdfColors = new int[5];
	
	public final GLRender render(GLContext ctx, final int x, final int y, final float size, JPaint paint) throws EOutOfMemory, EGLInvalidResource {
		final float scale = size / (float)m_size;
		String hash = Float.toString(scale);
		GLRender render = m_renderCache.get(hash);
		if (render == null) {
			render = createRender(hash);
			render.update(this, scale);
		}
		render.updateSDFParams(scale, paint.m_strokeOffset * paint.m_scale, paint.m_strokeWidth * paint.m_scale);
		float ox = x - render.m_alignX[paint.m_alignX - JAlignX.LEFT];
		float oy = y - render.m_alignY[paint.m_alignY - JAlignY.TOP];
		if (paint.m_colorFilter == JPaintColorFilter.PAINT_COLOR_FILTER_NEGATIVE) {
			sdfColors[0] = paint.m_sdfColors[0] ^ 0x00FFFFFF;
			sdfColors[1] = paint.m_sdfColors[1] ^ 0x00FFFFFF;
			sdfColors[2] = paint.m_sdfColors[2] ^ 0x00FFFFFF;
			sdfColors[3] = paint.m_sdfColors[3] ^ 0x00FFFFFF;
			sdfColors[4] = paint.m_strokeColor ^ 0x00FFFFFF;
		} else {
			sdfColors[0] = paint.m_sdfColors[0];
			sdfColors[1] = paint.m_sdfColors[1];
			sdfColors[2] = paint.m_sdfColors[2];
			sdfColors[3] = paint.m_sdfColors[3];
			sdfColors[4] = paint.m_strokeColor;
		}
		ctx.addOffset(ox, oy);
		if (paint.m_strokeInvisible) {
			if (!paint.m_sdfInvisible) {
				ctx.drawSDFTexture4Fill(render, sdfColors);
			}
		} else if (paint.m_sdfInvisible) {
			ctx.drawSDFTexture4Stroke(render, sdfColors[4]);
		} else {
			ctx.drawSDFTexture4StrokeFill(render, sdfColors);
		}
		ctx.subOffset(ox, oy);
		return render;
	}
	
	public final GLRender render(GLContext ctx, final String text, final int x, final int y, final int style, JPaint paint) throws EOutOfMemory, EGLInvalidResource {
		final float scale = paint.m_size * paint.m_scale / (float)m_size;
		String hash = MD5.unicode(text) + "," + Float.toString(scale) + "," + style;
		GLRender render = m_renderCache.get(hash);
		if (render == null) {
			render = createRender(hash);
			render.update(this, text, scale, style);
		}
		render.updateSDFParams(scale, paint.m_strokeOffset * paint.m_scale, paint.m_strokeWidth * paint.m_scale);
		float ox = x - render.m_alignX[paint.m_alignX - JAlignX.LEFT];
		float oy = y - render.m_alignY[paint.m_alignY - JAlignY.TOP];
		if (paint.m_colorFilter == JPaintColorFilter.PAINT_COLOR_FILTER_NEGATIVE) {
			sdfColors[0] = paint.m_color ^ 0x00FFFFFF;
			sdfColors[1] = paint.m_strokeColor ^ 0x00FFFFFF;
		} else {
			sdfColors[0] = paint.m_color;
			sdfColors[1] = paint.m_strokeColor;
		}
		ctx.addOffset(ox, oy);
		if (paint.m_strokeInvisible) {
			if (!paint.m_invisible) {
				ctx.drawSDFTexture1Fill(render, style, sdfColors[0]);
			}
		} else if (paint.m_invisible) {
			ctx.drawSDFTexture1Stroke(render, style, sdfColors[1]);
		} else {
			ctx.drawSDFTexture1StrokeFill(render, style, sdfColors);
		}
		ctx.subOffset(ox, oy);
		return render;
	}
	
}
