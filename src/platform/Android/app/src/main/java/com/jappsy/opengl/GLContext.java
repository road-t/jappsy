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

package com.jappsy.opengl;

import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.util.Stack;

import com.jappsy.BuildConfig;
import com.jappsy.core.Global;
import com.jappsy.core.Log;
import com.jappsy.exceptions.EOutOfMemory;
import com.jappsy.gui.JPaint;
import com.jappsy.gui.JPaintColorFilter;
import com.jappsy.opengl.image.GLImage;
import com.jappsy.opengl.image.GLRender;
import com.jappsy.opengl.image.GLTextureBuffer;
import com.jappsy.opengl.types.GLContextEntry;

import android.opengl.GLES20;

public class GLContext {

	private final static int mBytesPerFloat = 4;
	
	private int m_progCircleFill;
    private int m_progCircleStroke;
    private int m_progSquareFill;
    private int m_progSquareStroke;
    private int m_progTexture;
    private int m_progSquareVerticalGradient;
    private int m_progSDFTexture1Fill;
    private int m_progSDFTexture1Stroke;
    private int m_progSDFTexture1StrokeFill;
    private int m_progSDFTexture4Fill;
    private int m_progSDFTexture4Stroke;
    private int m_progSDFTexture4StrokeFill;
    
    public int[] m_size = new int[2];
    private float[] m_aspect = new float[2];
    private int[] m_client = new int[8];
	private float[] m_projection = new float[16];
    
    public GLContext() {
    }
    
    private static int[] maxTextureSize = {0};
	private static boolean isNPOTSupported = false;
	
	public static final int getMaxTextureSize() {
		return maxTextureSize[0];
	}
	
	public static final boolean getNPOTSupport() {
		return isNPOTSupported;
	}

	public static final int createTexture(Buffer pixelData, final int style, final int width, final int height) throws EOutOfMemory {
	    int[] texture = {0};
	    GLES20.glGenTextures(1, texture, 0);
	    if (GLES20.glGetError() == GLES20.GL_OUT_OF_MEMORY)
	    	throw new EOutOfMemory();
	    
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texture[0]);
        if (style == GLImage.GLImageStyleDistanceField) {
        	GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
        	GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        } else if ((style != GLImage.GLImageStyleSmooth) && (style != GLImage.GLImageStylePatch9) && (style != GLImage.GLImageStyleRepeat)) {
            GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_NEAREST);
            GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_NEAREST);
        } else {
        	GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
        	GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        }
        if (style != GLImage.GLImageStyleRepeat) {
        	GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
        	GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);
        } else {
        	GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT);
        	GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT);
        }
        
    	GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_RGBA, width, height, 0, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, pixelData);
        if (GLES20.glGetError() == GLES20.GL_OUT_OF_MEMORY) {
        	GLES20.glDeleteTextures(1, texture, 0);
            throw new EOutOfMemory();
        }

	    return texture[0];
	}
	
	public static final void destroyTexture(int[] texture) {
		if (texture[0] != 0) {
			GLES20.glDeleteTextures(1, texture, 0);
			texture[0] = 0;
		}
	}
	
	public static final void createFramebuffer(final int width, final int height, int[] handle, int[] frameBuffer, int[] stencilBuffer, final boolean smooth) throws EOutOfMemory {
		stencilBuffer[0] = frameBuffer[0] = handle[0] = 0;
	    
	    GLES20.glGenTextures(1, handle, 0);
	    if (GLES20.glGetError() == GLES20.GL_OUT_OF_MEMORY)
	    	throw new EOutOfMemory();
		    
    	GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, handle[0]);
    	ByteBuffer pixelData;
	    try {
	    	pixelData = ByteBuffer.allocateDirect(width * height * 4).order(ByteOrder.nativeOrder());
            for (int i = 0; i < width * height * 4; i+=4) {
            	byte v = (byte)Math.floor(Math.random() * 256.0);
            	pixelData.put(i, v);
            	pixelData.put(i + 1, v);
            	pixelData.put(i + 2, v);
            	v = (byte)Math.floor(Math.random() * 256.0);
            	pixelData.put(i + 3, v);
            }
	    } catch (OutOfMemoryError e) {
	    	GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
        	GLES20.glDeleteTextures(1, handle, 0);
            handle[0] = 0;
            throw new EOutOfMemory();
	    }
	    
        if (smooth) {
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        } else {
        	GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_NEAREST);
        	GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_NEAREST);
        }
        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_RGBA, width, height, 0, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, pixelData.position(0));
        if (GLES20.glGetError() == GLES20.GL_OUT_OF_MEMORY) {
        	GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
        	GLES20.glDeleteTextures(1, handle, 0);
            handle[0] = 0;
            throw new EOutOfMemory();
        }
        pixelData = null;
	           
		int[] restoreFrameBuffer = {0};
        GLES20.glGetIntegerv(GLES20.GL_FRAMEBUFFER_BINDING, restoreFrameBuffer, 0);

        try {
            GLES20.glGenFramebuffers(1, frameBuffer, 0);
            if (GLES20.glGetError() == GLES20.GL_OUT_OF_MEMORY)
		    	throw new EOutOfMemory();
            
            GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, frameBuffer[0]);
            GLES20.glFramebufferTexture2D(GLES20.GL_FRAMEBUFFER, GLES20.GL_COLOR_ATTACHMENT0, GLES20.GL_TEXTURE_2D, handle[0], 0);
        
            GLES20.glGenRenderbuffers(1, stencilBuffer, 0);
            if (GLES20.glGetError() == GLES20.GL_OUT_OF_MEMORY) {
            	GLES20.glDeleteFramebuffers(1, frameBuffer, 0);
            	frameBuffer[0] = 0;
		    	throw new EOutOfMemory();
            }
            
            GLES20.glBindRenderbuffer(GLES20.GL_RENDERBUFFER, stencilBuffer[0]);
            GLES20.glRenderbufferStorage(GLES20.GL_RENDERBUFFER, GLES20.GL_STENCIL_INDEX8, width, height);
            if (GLES20.glGetError() == GLES20.GL_OUT_OF_MEMORY) {
            	GLES20.glDeleteRenderbuffers(1, stencilBuffer, 0);
            	stencilBuffer[0] = 0;
            	GLES20.glDeleteFramebuffers(1, frameBuffer, 0);
            	frameBuffer[0] = 0;
		    	throw new EOutOfMemory();
            }
            
            GLES20.glFramebufferRenderbuffer(GLES20.GL_FRAMEBUFFER, GLES20.GL_STENCIL_ATTACHMENT, GLES20.GL_RENDERBUFFER, stencilBuffer[0]);
        } catch (EOutOfMemory e) {
        	GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
        	GLES20.glDeleteTextures(1, handle, 0);
            handle[0] = 0;
        	throw e;
        } finally {
            GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, restoreFrameBuffer[0]);
        }
	}
	
	public static final void destroyFramebuffer(int[] frameBuffer, int[] stencilBuffer) {
		if (stencilBuffer[0] != 0)
			GLES20.glDeleteRenderbuffers(1, stencilBuffer, 0);
		if (frameBuffer[0] != 0)
			GLES20.glDeleteFramebuffers(1, frameBuffer, 0);
		stencilBuffer[0] = frameBuffer[0] = 0;
	}
	
	public GLContext(final int w, final int h) {
    	GLES20.glGetIntegerv(GLES20.GL_MAX_TEXTURE_SIZE, maxTextureSize, 0);

    	final String extensionString = GLES20.glGetString(GLES20.GL_EXTENSIONS);
    	final String[] extensions = extensionString.split("[ \t]+");
    	for (String oneExtension : extensions) {
    		Log.debug(oneExtension);
    		if (oneExtension.equals("GL_OES_texture_npot")) {
    			isNPOTSupported = true;
    		}
    	}
        
        m_size[0] = w;
        m_size[1] = h;
        
        m_aspect[0] = m_aspect[1] = 1.0f;
        
        m_client[0] = m_client[1] = m_client[4] = m_client[5] = 0;
        m_client[2] = m_client[6] = w;
        m_client[3] = m_client[7] = h;

        setProjection(0.0f, 0.0f);
        
        m_progCircleFill = createShader("vsh_main", "fsh_circle_fill_aa");
        m_progCircleStroke = createShader("vsh_main", "fsh_circle_stroke_aa");
        m_progSquareFill = createShader("vsh_main", "fsh_square_fill");
        m_progSquareStroke = createShader("vsh_main", "fsh_square_stroke");
        m_progTexture = createShader("vsh_texture", "fsh_square_texture");
        m_progSquareVerticalGradient = createShader("vsh_main", "fsh_square_vertical_gradient");
        m_progSDFTexture1Fill = createShader("vsh_texture", "fsh_sdf_texture1_fill");
        m_progSDFTexture1Stroke = createShader("vsh_texture", "fsh_sdf_texture1_stroke");
        m_progSDFTexture1StrokeFill = createShader("vsh_texture", "fsh_sdf_texture1_stroke_fill");
        m_progSDFTexture4Fill = createShader("vsh_texture", "fsh_sdf_texture4_fill");
        m_progSDFTexture4Stroke = createShader("vsh_texture", "fsh_sdf_texture4_stroke");
        m_progSDFTexture4StrokeFill = createShader("vsh_texture", "fsh_sdf_texture4_stroke_fill");
	}
    
    public void Destroy() {
    	m_progSDFTexture4StrokeFill = destroyShader(m_progSDFTexture4StrokeFill);
    	m_progSDFTexture4Stroke = destroyShader(m_progSDFTexture4Stroke);
    	m_progSDFTexture4Fill = destroyShader(m_progSDFTexture4Fill);
    	m_progSDFTexture1StrokeFill = destroyShader(m_progSDFTexture1StrokeFill);
    	m_progSDFTexture1Stroke = destroyShader(m_progSDFTexture1Stroke);
    	m_progSDFTexture1Fill = destroyShader(m_progSDFTexture1Fill);
    	m_progSquareVerticalGradient = destroyShader(m_progSquareVerticalGradient);
    	m_progTexture = destroyShader(m_progTexture);
    	m_progSquareStroke = destroyShader(m_progSquareStroke);
    	m_progSquareFill = destroyShader(m_progSquareFill);
    	m_progCircleStroke = destroyShader(m_progCircleStroke);
    	m_progCircleFill = destroyShader(m_progCircleFill);
    }
    
    // Drawing
    
    private final void setProjection(final float offsetX, final float offsetY) {
    	 for (int i = 0; i < 16; i++) {
         	m_projection[i] = 0;
         }
         m_projection[0] = 2.0f / (float)(m_size[0]);
         m_projection[3] = -1.0f + offsetX * m_projection[0];
         m_projection[5] = -2.0f / (float)(m_size[1]);
         m_projection[7] = 1.0f + offsetY * m_projection[5];
         m_projection[10] = 1.0f;
         m_projection[15] = 1.0f;
         /*
 	     {  2/w,     0,     0,   -1}		rX = sX * 2 / w - 1 <=> (0..w) -> (-1..+1)
 	     {    0,  -2/h,     0,    1}		rY = sY * -2 / h + 1 <=> (0..w) -> (+1..-1)	// flip vertical
 	     {    0,     0,     1,    0}
 	     {    0,     0,     0,    1}
         */
    }
    
    public final void addOffset(final float offsetX, final float offsetY) {
    	m_projection[3] += offsetX * m_projection[0];
    	m_projection[7] += offsetY * m_projection[5];
    }
    
    public final void subOffset(final float offsetX, final float offsetY) {
    	m_projection[3] -= offsetX * m_projection[0];
    	m_projection[7] -= offsetY * m_projection[5];
    }
    
    private static FloatBuffer m_square = ByteBuffer.allocateDirect(8 * mBytesPerFloat).order(ByteOrder.nativeOrder()).asFloatBuffer();
    private static FloatBuffer m_line = ByteBuffer.allocateDirect(4 * mBytesPerFloat).order(ByteOrder.nativeOrder()).asFloatBuffer();;
    //private static FloatBuffer m_point = ByteBuffer.allocateDirect(2 * mBytesPerFloat).order(ByteOrder.nativeOrder()).asFloatBuffer();
    private static float[] m_color = new float[4*5];

    private static final FloatBuffer makeRect(final int x1, final int y1, final int x2, final int y2) {
    	m_square.put(0, x1);
        m_square.put(1, y1);
        m_square.put(2, x2);
        m_square.put(3, y1);
        m_square.put(4, x1);
        m_square.put(5, y2);
        m_square.put(6, x2);
        m_square.put(7, y2);
        m_square.position(0);
        return m_square;
    }

    private static final FloatBuffer makeLine(final int x1, final int y1, final int x2, final int y2) {
        m_line.put(0, x1);
        m_line.put(1, y1);
        m_line.put(2, x2);
        m_line.put(3, y2);
        m_line.position(0);
        return m_line;
    }

    /*
    private static final FloatBuffer makePoint(final int x1, final int y1) {
        m_point.put(0, x1);
        m_point.put(1, y1);
        m_point.position(0);
        return m_point;
    }
    */
    
    public static final float[] makeColor(final int color) {
        m_color[0] = ((color >> 16) & 0xFF) / 255.0f; // R
        m_color[1] = ((color >> 8) & 0xFF) / 255.0f; // G
        m_color[2] = (color & 0xFF) / 255.0f; // B
        m_color[3] = ((color >> 24) & 0xFF) / 255.0f; // A
        return m_color;
    }
    
    public static final float[] makeColor(final int[] color, final int count) {
    	int ofs = 0;
    	for (int i = 0; i < count; i++) {
    		m_color[ofs + 0] = ((color[i] >> 16) & 0xFF) / 255.0f; // R
    		m_color[ofs + 1] = ((color[i] >> 8) & 0xFF) / 255.0f; // G
    		m_color[ofs + 2] = (color[i] & 0xFF) / 255.0f; // B
    		m_color[ofs + 3] = ((color[i] >> 24) & 0xFF) / 255.0f; // A
            ofs += 4;
    	}
    	return m_color;
    }

    // Shader Loader
    
    private final int createShader(final String vertexShaderFile, final String fragmentShaderFile) {
        int program = 0;
		final int[] status = {0};
        
        int vertexShader = GLES20.glCreateShader(GLES20.GL_VERTEX_SHADER);
        if (vertexShader == 0) {
        	return 0;
        }
        
        String vertexShaderSource = Global.getRawString(vertexShaderFile);
        if (vertexShaderSource == null) {
        	GLES20.glDeleteShader(vertexShader);
        	return 0;
        }
        	
        GLES20.glShaderSource(vertexShader, vertexShaderSource);
        GLES20.glCompileShader(vertexShader);
        
		GLES20.glGetShaderiv(vertexShader, GLES20.GL_COMPILE_STATUS, status, 0);
		if(status[0] == GLES20.GL_FALSE) {
	    	if (BuildConfig.DEBUG) { 
    			Log.debug("OpenGL Vertex Shader Log:\r\n" + GLES20.glGetShaderInfoLog(vertexShader));
	    	}
			GLES20.glDeleteShader(vertexShader);
	    	return 0;
		}
        
        int fragmentShader = GLES20.glCreateShader(GLES20.GL_FRAGMENT_SHADER);
        if (fragmentShader == 0) {
        	GLES20.glDeleteShader(vertexShader);
        	return 0;
        }
        
        String fragmentShaderSource = Global.getRawString(fragmentShaderFile);
        if (fragmentShaderSource == null) {
        	GLES20.glDeleteShader(fragmentShader);
        	GLES20.glDeleteShader(vertexShader);
        	return 0;
        }
        
        GLES20.glShaderSource(fragmentShader, fragmentShaderSource);
        GLES20.glCompileShader(fragmentShader);
        
		GLES20.glGetShaderiv(fragmentShader, GLES20.GL_COMPILE_STATUS, status, 0);
		if(status[0] == GLES20.GL_FALSE) {
	    	if (BuildConfig.DEBUG) { 
    			Log.debug("OpenGL Fragment Shader Log:\r\n" + GLES20.glGetShaderInfoLog(fragmentShader));
    		}
			GLES20.glDeleteShader(fragmentShader);
			GLES20.glDeleteShader(vertexShader);
	    	return 0;
        }

    	program = GLES20.glCreateProgram();
        GLES20.glAttachShader(program, vertexShader);
        GLES20.glAttachShader(program, fragmentShader);
        GLES20.glLinkProgram(program);
        
        GLES20.glGetProgramiv(program, GLES20.GL_LINK_STATUS, status, 0);
		if(status[0] == GLES20.GL_FALSE) {
			if (BuildConfig.DEBUG) { 
    			Log.debug("OpenGL ES Programm Log:\r\n" + GLES20.glGetProgramInfoLog(program));
    		}
			GLES20.glDeleteShader(fragmentShader);
			GLES20.glDeleteShader(vertexShader);
			GLES20.glDeleteProgram(program);
			return 0;
        }
        
        return program;
    }

    private final int destroyShader(final int program) {
        GLES20.glDeleteProgram(program);
        return 0;
    }

    public final void drawColor(final int color) {
    	float[] c = makeColor(color);
        GLES20.glClearColor(c[0], c[1], c[2], c[3]);
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
    }

    // Drawing
    
    public static final void fill(final int color) {
	    float[] c = makeColor(color);
	    GLES20.glClearColor(c[0], c[1], c[2], c[3]);
	    GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
    }

    public static final void fillAlpha(final byte alpha) {
		GLES20.glColorMask(false, false, false, true);
		GLES20.glClearColor(0.0f, 0.0f, 0.0f, (float)(alpha) / 255.0f);
		GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
		GLES20.glColorMask(true, true, true, true);
    }
    
    private final void drawSDFTexture1Fill(final int texture, final int count, final Buffer vertexSquare, final Buffer textureSquare, final float[] params, final int index, final int color) {
    	GLES20.glEnable(GLES20.GL_BLEND);
    	GLES20.glUseProgram(m_progSDFTexture1Fill);
    	
        int uProjection = GLES20.glGetUniformLocation(m_progSDFTexture1Fill, "u_projection");
        GLES20.glUniformMatrix4fv(uProjection, 1, false, m_projection, 0);
    	
        int uParams = GLES20.glGetUniformLocation(m_progSDFTexture1Fill, "u_params");
        GLES20.glUniform4fv(uParams, 1, params, 0);

        int aPosition = GLES20.glGetAttribLocation(m_progSDFTexture1Fill, "a_position");
        GLES20.glVertexAttribPointer(aPosition, 2, GLES20.GL_FLOAT, false, 0, vertexSquare);
        int aTexture = GLES20.glGetAttribLocation(m_progSDFTexture1Fill, "a_texture");
        GLES20.glVertexAttribPointer(aTexture, 2, GLES20.GL_FLOAT, false, 0, textureSquare);
        
        GLES20.glEnableVertexAttribArray(aPosition);
        GLES20.glEnableVertexAttribArray(aTexture);
        
        GLES20.glEnable(GLES20.GL_TEXTURE_2D);
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texture);
        
        int uSampler = GLES20.glGetUniformLocation(m_progSDFTexture1Fill, "u_sampler");
        GLES20.glUniform1f(uSampler, 0);

        int uIndex = GLES20.glGetUniformLocation(m_progSDFTexture1Fill, "u_index");
        GLES20.glUniform1i(uIndex, index);

        float[] c = makeColor(color);
        int uColors = GLES20.glGetUniformLocation(m_progSDFTexture1Fill, "u_color");
        GLES20.glUniform4fv(uColors, 1, c, 0);

        for (int i = 0; i < count; i++) {
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, i * 4, 4);
        }
        GLES20.glDisable(GLES20.GL_TEXTURE_2D);
    }

    public final void drawSDFTexture1Fill(final GLRender render, final int index, final int color) {
		GLES20.glEnable(GLES20.GL_BLEND);
    	for (int i = 0; i < render.m_count; i++) {
    		GLTextureBuffer buffer = render.m_buffers[i];
    		drawSDFTexture1Fill(buffer.m_handle[0], buffer.m_count, buffer.m_vertexBuffer, buffer.m_textureBuffer, render.m_sdfParams, index, color);
    	}
    }
    
    private final void drawSDFTexture1Stroke(final int texture, final int count, final Buffer vertexSquare, final Buffer textureSquare, final float[] params, final int index, final int color) {
    	GLES20.glEnable(GLES20.GL_BLEND);
    	GLES20.glUseProgram(m_progSDFTexture1Stroke);
    	
        int uProjection = GLES20.glGetUniformLocation(m_progSDFTexture1Stroke, "u_projection");
        GLES20.glUniformMatrix4fv(uProjection, 1, false, m_projection, 0);
    	
        int uParams = GLES20.glGetUniformLocation(m_progSDFTexture1Stroke, "u_params");
        GLES20.glUniform4fv(uParams, 1, params, 0);

        int aPosition = GLES20.glGetAttribLocation(m_progSDFTexture1Stroke, "a_position");
        GLES20.glVertexAttribPointer(aPosition, 2, GLES20.GL_FLOAT, false, 0, vertexSquare);
        int aTexture = GLES20.glGetAttribLocation(m_progSDFTexture1Stroke, "a_texture");
        GLES20.glVertexAttribPointer(aTexture, 2, GLES20.GL_FLOAT, false, 0, textureSquare);
        
        GLES20.glEnableVertexAttribArray(aPosition);
        GLES20.glEnableVertexAttribArray(aTexture);
        
        GLES20.glEnable(GLES20.GL_TEXTURE_2D);
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texture);
        
        int uSampler = GLES20.glGetUniformLocation(m_progSDFTexture1Stroke, "u_sampler");
        GLES20.glUniform1f(uSampler, 0);

        int uIndex = GLES20.glGetUniformLocation(m_progSDFTexture1Stroke, "u_index");
        GLES20.glUniform1i(uIndex, index);

        float[] c = makeColor(color);
        int uColors = GLES20.glGetUniformLocation(m_progSDFTexture1Stroke, "u_color");
        GLES20.glUniform4fv(uColors, 1, c, 0);

        for (int i = 0; i < count; i++) {
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, i * 4, 4);
        }
        GLES20.glDisable(GLES20.GL_TEXTURE_2D);
    }

    public final void drawSDFTexture1Stroke(final GLRender render, final int index, final int color) {
		GLES20.glEnable(GLES20.GL_BLEND);
    	for (int i = 0; i < render.m_count; i++) {
    		GLTextureBuffer buffer = render.m_buffers[i];
    		drawSDFTexture1Stroke(buffer.m_handle[0], buffer.m_count, buffer.m_vertexBuffer, buffer.m_textureBuffer, render.m_sdfParams, index, color);
    	}
    }
    
    private final void drawSDFTexture1StrokeFill(final int texture, final int count, final Buffer vertexSquare, final Buffer textureSquare, final float[] params, final int index, final int[] colors) {
    	GLES20.glEnable(GLES20.GL_BLEND);
    	GLES20.glUseProgram(m_progSDFTexture1StrokeFill);
    	
        int uProjection = GLES20.glGetUniformLocation(m_progSDFTexture1StrokeFill, "u_projection");
        GLES20.glUniformMatrix4fv(uProjection, 1, false, m_projection, 0);
    	
        int uParams = GLES20.glGetUniformLocation(m_progSDFTexture1StrokeFill, "u_params");
        GLES20.glUniform4fv(uParams, 1, params, 0);

        int aPosition = GLES20.glGetAttribLocation(m_progSDFTexture1StrokeFill, "a_position");
        GLES20.glVertexAttribPointer(aPosition, 2, GLES20.GL_FLOAT, false, 0, vertexSquare);
        int aTexture = GLES20.glGetAttribLocation(m_progSDFTexture1StrokeFill, "a_texture");
        GLES20.glVertexAttribPointer(aTexture, 2, GLES20.GL_FLOAT, false, 0, textureSquare);
        
        GLES20.glEnableVertexAttribArray(aPosition);
        GLES20.glEnableVertexAttribArray(aTexture);
        
        GLES20.glEnable(GLES20.GL_TEXTURE_2D);
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texture);
        
        int uSampler = GLES20.glGetUniformLocation(m_progSDFTexture1StrokeFill, "u_sampler");
        GLES20.glUniform1f(uSampler, 0);

        int uIndex = GLES20.glGetUniformLocation(m_progSDFTexture1StrokeFill, "u_index");
        GLES20.glUniform1i(uIndex, index);

        float[] c = makeColor(colors, 2);
        int uColors = GLES20.glGetUniformLocation(m_progSDFTexture1StrokeFill, "u_colors");
        GLES20.glUniform4fv(uColors, 2, c, 0);

        for (int i = 0; i < count; i++) {
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, i * 4, 4);
        }
        GLES20.glDisable(GLES20.GL_TEXTURE_2D);
    }

    public final void drawSDFTexture1StrokeFill(final GLRender render, final int index, final int[] colors) {
		GLES20.glEnable(GLES20.GL_BLEND);
    	for (int i = 0; i < render.m_count; i++) {
    		GLTextureBuffer buffer = render.m_buffers[i];
    		drawSDFTexture1StrokeFill(buffer.m_handle[0], buffer.m_count, buffer.m_vertexBuffer, buffer.m_textureBuffer, render.m_sdfParams, index, colors);
    	}
    }
    
    private final void drawSDFTexture4Fill(final int texture, final int count, final Buffer vertexSquare, final Buffer textureSquare, final float[] params, final int[] colors) {
    	GLES20.glEnable(GLES20.GL_BLEND);
    	GLES20.glUseProgram(m_progSDFTexture4Fill);
    	
        int uProjection = GLES20.glGetUniformLocation(m_progSDFTexture4Fill, "u_projection");
        GLES20.glUniformMatrix4fv(uProjection, 1, false, m_projection, 0);
    	
        int uParams = GLES20.glGetUniformLocation(m_progSDFTexture4Fill, "u_params");
        GLES20.glUniform4fv(uParams, 1, params, 0);

        int aPosition = GLES20.glGetAttribLocation(m_progSDFTexture4Fill, "a_position");
        GLES20.glVertexAttribPointer(aPosition, 2, GLES20.GL_FLOAT, false, 0, vertexSquare);
        int aTexture = GLES20.glGetAttribLocation(m_progSDFTexture4Fill, "a_texture");
        GLES20.glVertexAttribPointer(aTexture, 2, GLES20.GL_FLOAT, false, 0, textureSquare);
        
        GLES20.glEnableVertexAttribArray(aPosition);
        GLES20.glEnableVertexAttribArray(aTexture);
        
        GLES20.glEnable(GLES20.GL_TEXTURE_2D);
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texture);
        
        int uSampler = GLES20.glGetUniformLocation(m_progSDFTexture4Fill, "u_sampler");
        GLES20.glUniform1f(uSampler, 0);

        float[] c = makeColor(colors, 4);
        int uColors = GLES20.glGetUniformLocation(m_progSDFTexture4Fill, "u_colors");
        GLES20.glUniform4fv(uColors, 4, c, 0);

        for (int i = 0; i < count; i++) {
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, i * 4, 4);
        }
        GLES20.glDisable(GLES20.GL_TEXTURE_2D);
    }

    public final void drawSDFTexture4Fill(final GLRender render, final int[] colors) {
		GLES20.glEnable(GLES20.GL_BLEND);
    	for (int i = 0; i < render.m_count; i++) {
    		GLTextureBuffer buffer = render.m_buffers[i];
    		drawSDFTexture4Fill(buffer.m_handle[0], buffer.m_count, buffer.m_vertexBuffer, buffer.m_textureBuffer, render.m_sdfParams, colors);
    	}
    }

    private final void drawSDFTexture4Stroke(final int texture, final int count, final Buffer vertexSquare, final Buffer textureSquare, final float[] params, final int color) {
    	GLES20.glEnable(GLES20.GL_BLEND);
    	GLES20.glUseProgram(m_progSDFTexture4Stroke);
    	
        int uProjection = GLES20.glGetUniformLocation(m_progSDFTexture4Stroke, "u_projection");
        GLES20.glUniformMatrix4fv(uProjection, 1, false, m_projection, 0);
    	
        int uParams = GLES20.glGetUniformLocation(m_progSDFTexture4Stroke, "u_params");
        GLES20.glUniform4fv(uParams, 1, params, 0);

        int aPosition = GLES20.glGetAttribLocation(m_progSDFTexture4Stroke, "a_position");
        GLES20.glVertexAttribPointer(aPosition, 2, GLES20.GL_FLOAT, false, 0, vertexSquare);
        int aTexture = GLES20.glGetAttribLocation(m_progSDFTexture4Stroke, "a_texture");
        GLES20.glVertexAttribPointer(aTexture, 2, GLES20.GL_FLOAT, false, 0, textureSquare);
        
        GLES20.glEnableVertexAttribArray(aPosition);
        GLES20.glEnableVertexAttribArray(aTexture);
        
        GLES20.glEnable(GLES20.GL_TEXTURE_2D);
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texture);
        
        int uSampler = GLES20.glGetUniformLocation(m_progSDFTexture4Stroke, "u_sampler");
        GLES20.glUniform1f(uSampler, 0);

        float[] c = makeColor(color);
        int uColors = GLES20.glGetUniformLocation(m_progSDFTexture4Stroke, "u_color");
        GLES20.glUniform4fv(uColors, 1, c, 0);

        for (int i = 0; i < count; i++) {
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, i * 4, 4);
        }
        GLES20.glDisable(GLES20.GL_TEXTURE_2D);
    }

    public final void drawSDFTexture4Stroke(final GLRender render, final int color) {
		GLES20.glEnable(GLES20.GL_BLEND);
    	for (int i = 0; i < render.m_count; i++) {
    		GLTextureBuffer buffer = render.m_buffers[i];
    		drawSDFTexture4Stroke(buffer.m_handle[0], buffer.m_count, buffer.m_vertexBuffer, buffer.m_textureBuffer, render.m_sdfParams, color);
    	}
    }

    private final void drawSDFTexture4StrokeFill(final int texture, final int count, final Buffer vertexSquare, final Buffer textureSquare, final float[] params, final int[] colors) {
    	GLES20.glEnable(GLES20.GL_BLEND);
    	GLES20.glUseProgram(m_progSDFTexture4StrokeFill);
    	
        int uProjection = GLES20.glGetUniformLocation(m_progSDFTexture4StrokeFill, "u_projection");
        GLES20.glUniformMatrix4fv(uProjection, 1, false, m_projection, 0);
    	
        int uParams = GLES20.glGetUniformLocation(m_progSDFTexture4StrokeFill, "u_params");
        GLES20.glUniform4fv(uParams, 1, params, 0);

        int aPosition = GLES20.glGetAttribLocation(m_progSDFTexture4StrokeFill, "a_position");
        GLES20.glVertexAttribPointer(aPosition, 2, GLES20.GL_FLOAT, false, 0, vertexSquare);
        int aTexture = GLES20.glGetAttribLocation(m_progSDFTexture4StrokeFill, "a_texture");
        GLES20.glVertexAttribPointer(aTexture, 2, GLES20.GL_FLOAT, false, 0, textureSquare);
        
        GLES20.glEnableVertexAttribArray(aPosition);
        GLES20.glEnableVertexAttribArray(aTexture);
        
        GLES20.glEnable(GLES20.GL_TEXTURE_2D);
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texture);
        
        int uSampler = GLES20.glGetUniformLocation(m_progSDFTexture4StrokeFill, "u_sampler");
        GLES20.glUniform1f(uSampler, 0);

        float[] c = makeColor(colors, 5);
        int uColors = GLES20.glGetUniformLocation(m_progSDFTexture4StrokeFill, "u_colors");
        GLES20.glUniform4fv(uColors, 5, c, 0);

        for (int i = 0; i < count; i++) {
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, i * 4, 4);
        }
        GLES20.glDisable(GLES20.GL_TEXTURE_2D);
    }

    public final void drawSDFTexture4StrokeFill(final GLRender render, final int[] colors) {
		GLES20.glEnable(GLES20.GL_BLEND);
    	for (int i = 0; i < render.m_count; i++) {
    		GLTextureBuffer buffer = render.m_buffers[i];
    		drawSDFTexture4StrokeFill(buffer.m_handle[0], buffer.m_count, buffer.m_vertexBuffer, buffer.m_textureBuffer, render.m_sdfParams, colors);
    	}
    }

    private final void drawTexture(final int texture, final int count, final Buffer vertexSquare, final Buffer textureSquare, final boolean negative, final boolean forceSmooth) {
    	GLES20.glUseProgram(m_progTexture);
        
        int uProjection = GLES20.glGetUniformLocation(m_progTexture, "u_projection");
        GLES20.glUniformMatrix4fv(uProjection, 1, false, m_projection, 0);
        
        /*
         float size[] = { vertexSquare[2] - vertexSquare[0], vertexSquare[5] - vertexSquare[1] };
         int uTexture = glGetUniformLocation(self.m_progTexture, "u_texture");
         glUniform2fv(uTexture, 1, size);
         
         BOOL antialias = false;
         int uAntialias = glGetUniformLocation(self.m_progCircleStroke, "u_antialias");
         glUniform1i(uAntialias, antialias ? 1 : 0);
         */
        
        int aPosition = GLES20.glGetAttribLocation(m_progTexture, "a_position");
        GLES20.glVertexAttribPointer(aPosition, 2, GLES20.GL_FLOAT, false, 0, vertexSquare);
        int aTexture = GLES20.glGetAttribLocation(m_progTexture, "a_texture");
        GLES20.glVertexAttribPointer(aTexture, 2, GLES20.GL_FLOAT, false, 0, textureSquare);
        
        GLES20.glEnableVertexAttribArray(aPosition);
        GLES20.glEnableVertexAttribArray(aTexture);
        
        GLES20.glEnable(GLES20.GL_TEXTURE_2D);
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texture);
        if (forceSmooth) {
        	GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
        	GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        }
        
        int uSampler = GLES20.glGetUniformLocation(m_progTexture, "u_sampler");
        GLES20.glUniform1f(uSampler, 0);

        int uNegative = GLES20.glGetUniformLocation(m_progTexture, "u_negative");
        GLES20.glUniform1i(uNegative, negative ? 1 : 0);
        
        for (int i = 0; i < count; i++) {
        	GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, i * 4, 4);
        }
        GLES20.glDisable(GLES20.GL_TEXTURE_2D);
    }
    
    public final void drawTexture(final GLRender render, final boolean negative, final boolean forceSmooth) {
    	for (int i = 0; i < render.m_count; i++) {
    		GLTextureBuffer buffer = render.m_buffers[i];
    		if (buffer.m_alpha) {
    			GLES20.glEnable(GLES20.GL_BLEND);
    		} else {
    			GLES20.glDisable(GLES20.GL_BLEND);
    		}
    		drawTexture(buffer.m_handle[0], buffer.m_count, buffer.m_vertexBuffer, buffer.m_textureBuffer, negative, forceSmooth);
    	}
    }

    public final void drawRect(final int x1, final int y1, final int x2, final int y2, final int color, final boolean negative) {
        if ((color & 0xFF000000) != 0xFF000000) {
        	GLES20.glEnable(GLES20.GL_BLEND);
        } else {
        	GLES20.glDisable(GLES20.GL_BLEND);
        }
        GLES20.glUseProgram(m_progSquareFill);
        
        int uProjection = GLES20.glGetUniformLocation(m_progSquareFill, "u_projection");
        GLES20.glUniformMatrix4fv(uProjection, 1, false, m_projection, 0);
        
        float[] c = makeColor(color);
        int uColor = GLES20.glGetUniformLocation(m_progSquareFill, "u_color");
        GLES20.glUniform4fv(uColor, 1, c, 0);
        
        int uNegative = GLES20.glGetUniformLocation(m_progSquareFill, "u_negative");
        GLES20.glUniform1i(uNegative, negative ? 1 : 0);
        
        int aPosition = GLES20.glGetAttribLocation(m_progSquareFill, "a_position");
        GLES20.glEnableVertexAttribArray(aPosition);
        
        GLES20.glVertexAttribPointer(aPosition, 2, GLES20.GL_FLOAT, false, 0, makeRect(x1, y1, x2, y2));
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
    }

    public final void drawRect(final int x1, final int y1, final int x2, final int y2, final JPaint paint) {
        if ((paint.m_color & 0xFF000000) != 0) {
            if ((paint.m_color & 0xFF000000) != 0xFF000000) {
            	GLES20.glEnable(GLES20.GL_BLEND);
            } else {
            	GLES20.glDisable(GLES20.GL_BLEND);
            }
            GLES20.glUseProgram(m_progSquareFill);
            
            int uProjection = GLES20.glGetUniformLocation(m_progSquareFill, "u_projection");
            GLES20.glUniformMatrix4fv(uProjection, 1, false, m_projection, 0);
            
            float[] color = makeColor(paint.m_color);
            int uColor = GLES20.glGetUniformLocation(m_progSquareFill, "u_color");
            GLES20.glUniform4fv(uColor, 1, color, 0);
            
            int uNegative = GLES20.glGetUniformLocation(m_progSquareFill, "u_negative");
            GLES20.glUniform1i(uNegative, (paint.m_colorFilter == JPaintColorFilter.PAINT_COLOR_FILTER_NEGATIVE) ? 1 : 0);
            
            int aPosition = GLES20.glGetAttribLocation(m_progSquareFill, "a_position");
            GLES20.glEnableVertexAttribArray(aPosition);
            
            GLES20.glVertexAttribPointer(aPosition, 2, GLES20.GL_FLOAT, false, 0, makeRect(x1, y1, x2, y2));
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
        }

        if ((paint.m_strokeColor & 0xFF000000) != 0) {
        	GLES20.glEnable(GLES20.GL_BLEND);
        	GLES20.glUseProgram(m_progSquareStroke);
            
            int uProjection = GLES20.glGetUniformLocation(m_progSquareStroke, "u_projection");
            GLES20.glUniformMatrix4fv(uProjection, 1, false, m_projection, 0);
            
            int half = (int)paint.m_strokeWidth >> 1;
            
            float point1[] = {
            	(float)(x1 - half), (float)(y1 - half)
            };
            int uPoint1 = GLES20.glGetUniformLocation(m_progSquareStroke, "u_point1");
            GLES20.glUniform2fv(uPoint1, 1, point1, 0);
            
            float point2[] = {
                (float)(x2 + half), (float)(y2 + half)
            };
            int uPoint2 = GLES20.glGetUniformLocation(m_progSquareStroke, "u_point2");
            GLES20.glUniform2fv(uPoint2, 1, point2, 0);
            
            float border = paint.m_strokeWidth;
            int uBorder = GLES20.glGetUniformLocation(m_progSquareStroke, "u_border");
            GLES20.glUniform1f(uBorder, border);
            
            float[] color = makeColor(paint.m_strokeColor);
            int uColor = GLES20.glGetUniformLocation(m_progSquareStroke, "u_color");
            GLES20.glUniform4fv(uColor, 1, color, 0);
            
            int uNegative = GLES20.glGetUniformLocation(m_progSquareStroke, "u_negative");
            GLES20.glUniform1i(uNegative, (paint.m_colorFilter == JPaintColorFilter.PAINT_COLOR_FILTER_NEGATIVE) ? 1 : 0);
            
            int aPosition = GLES20.glGetAttribLocation(m_progSquareStroke, "a_position");
            GLES20.glEnableVertexAttribArray(aPosition);
            
            GLES20.glVertexAttribPointer(aPosition, 2, GLES20.GL_FLOAT, false, 0, makeRect(x1-half, y1-half, x2+half, y2+half));
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
        }
    }

    public final void drawVerticalGradient(final int x1, final int y1, final int x2, final int y2, final int color1, final int color2, final boolean negative) {
        if (((color1 & 0xFF000000) != 0xFF000000) ||
            ((color2 & 0xFF000000) != 0xFF000000)) {
        	GLES20.glEnable(GLES20.GL_BLEND);
        } else {
        	GLES20.glDisable(GLES20.GL_BLEND);
        }
        GLES20.glUseProgram(m_progSquareVerticalGradient);
        
        int uProjection = GLES20.glGetUniformLocation(m_progSquareVerticalGradient, "u_projection");
        GLES20.glUniformMatrix4fv(uProjection, 1, false, m_projection, 0);
        
        float point1[] = {
            (float)(x1), (float)(y1)
        };
        int uPoint1 = GLES20.glGetUniformLocation(m_progSquareVerticalGradient, "u_point1");
        GLES20.glUniform2fv(uPoint1, 1, point1, 0);
        
        float point2[] = {
            (float)(x2), (float)(y2)
        };
        int uPoint2 = GLES20.glGetUniformLocation(m_progSquareVerticalGradient, "u_point2");
        GLES20.glUniform2fv(uPoint2, 1, point2, 0);
        
        float[] c1 = makeColor(color1);
        int uColor1 = GLES20.glGetUniformLocation(m_progSquareVerticalGradient, "u_color1");
        GLES20.glUniform4fv(uColor1, 1, c1, 0);
        
        float[] c2 = makeColor(color2);
        int uColor2 = GLES20.glGetUniformLocation(m_progSquareVerticalGradient, "u_color2");
        GLES20.glUniform4fv(uColor2, 1, c2, 0);
        
        int uNegative = GLES20.glGetUniformLocation(m_progSquareStroke, "u_negative");
        GLES20.glUniform1i(uNegative, negative ? 1 : 0);
        
        int aPosition = GLES20.glGetAttribLocation(m_progSquareStroke, "a_position");
        GLES20.glEnableVertexAttribArray(aPosition);
        
        GLES20.glVertexAttribPointer(aPosition, 2, GLES20.GL_FLOAT, false, 0, makeRect(x1, y1, x2, y2));
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
    }

    public final void drawLine(final int x1, final int y1, final int x2, final int y2, final JPaint paint) {
        if ((paint.m_strokeColor & 0xFF000000) != 0) {
            if ((paint.m_strokeColor & 0xFF000000) != 0xFF000000) {
            	GLES20.glEnable(GLES20.GL_BLEND);
            } else {
            	GLES20.glDisable(GLES20.GL_BLEND);
            }
            GLES20.glUseProgram(m_progSquareFill);
            
            int uProjection = GLES20.glGetUniformLocation(m_progSquareFill, "u_projection");
            GLES20.glUniformMatrix4fv(uProjection, 1, false, m_projection, 0);
            
            float[] color = makeColor(paint.m_strokeColor);
            int uColor = GLES20.glGetUniformLocation(m_progSquareFill, "u_color");
            GLES20.glUniform4fv(uColor, 1, color, 0);
            
            int uNegative = GLES20.glGetUniformLocation(m_progSquareFill, "u_negative");
            GLES20.glUniform1i(uNegative, (paint.m_colorFilter == JPaintColorFilter.PAINT_COLOR_FILTER_NEGATIVE) ? 1 : 0);
            
            int aPosition = GLES20.glGetAttribLocation(m_progSquareFill, "a_position");
            GLES20.glEnableVertexAttribArray(aPosition);
            
            GLES20.glVertexAttribPointer(aPosition, 2, GLES20.GL_FLOAT, false, 2*4, makeLine(x1, y1, x2, y2));
            GLES20.glLineWidth(paint.m_strokeWidth);
            GLES20.glDrawArrays(GLES20.GL_LINES, 0, 2);
        }
    }

    public final void drawCircle(final int x, final int y, final float radius, final JPaint paint) {
        if ((paint.m_color & 0xFF000000) != 0) {
            if ((paint.m_antialias) || ((paint.m_color & 0xFF000000) != 0xFF000000)) {
            	GLES20.glEnable(GLES20.GL_BLEND);
            } else {
            	GLES20.glDisable(GLES20.GL_BLEND);
            }

            GLES20.glUseProgram(m_progCircleFill);
            
            int uProjection = GLES20.glGetUniformLocation(m_progCircleFill, "u_projection");
            GLES20.glUniformMatrix4fv(uProjection, 1, false, m_projection, 0);
            
            float center[] = {
                (float)(x), (float)(y)
            };
            int uCenter = GLES20.glGetUniformLocation(m_progCircleFill, "u_center");
            GLES20.glUniform2fv(uCenter, 1, center, 0);
            
            int uRadius = GLES20.glGetUniformLocation(m_progCircleFill, "u_radius");
            GLES20.glUniform1f(uRadius, radius);
            
            float[] color = makeColor(paint.m_color);
            int uColor = GLES20.glGetUniformLocation(m_progCircleFill, "u_color");
            GLES20.glUniform4fv(uColor, 1, color, 0);
            
            int uAntialias = GLES20.glGetUniformLocation(m_progCircleFill, "u_antialias");
            GLES20.glUniform1i(uAntialias, paint.m_antialias ? 1 : 0);
            
            int uNegative = GLES20.glGetUniformLocation(m_progCircleFill, "u_negative");
            GLES20.glUniform1i(uNegative, (paint.m_colorFilter == JPaintColorFilter.PAINT_COLOR_FILTER_NEGATIVE) ? 1 : 0);
            
            int aPosition = GLES20.glGetAttribLocation(m_progCircleFill, "a_position");
            GLES20.glEnableVertexAttribArray(aPosition);
            
            GLES20.glVertexAttribPointer(aPosition, 2, GLES20.GL_FLOAT, false, 0, makeRect((int)Math.round(x-radius-1), (int)Math.round(y-radius-1), (int)Math.round(x+radius+1), (int)Math.round(y+radius+1)));
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
        }
        
        if ((paint.m_strokeColor & 0xFF000000) != 0) {
            if ((paint.m_antialias) || ((paint.m_strokeColor & 0xFF000000) != 0xFF000000)) {
            	GLES20.glEnable(GLES20.GL_BLEND);
            } else {
            	GLES20.glDisable(GLES20.GL_BLEND);
            }

            GLES20.glUseProgram(m_progCircleStroke);
            
            int uProjection = GLES20.glGetUniformLocation(m_progCircleStroke, "u_projection");
            GLES20.glUniformMatrix4fv(uProjection, 1, false, m_projection, 0);
            
            float center[] = {
                (float)(x), (float)(y)
            };
            int uCenter = GLES20.glGetUniformLocation(m_progCircleStroke, "u_center");
            GLES20.glUniform2fv(uCenter, 1, center, 0);
            
            int uRadius = GLES20.glGetUniformLocation(m_progCircleStroke, "u_radius");
            GLES20.glUniform1f(uRadius, radius);
            
            float border = paint.m_strokeWidth;
            int uBorder = GLES20.glGetUniformLocation(m_progCircleStroke, "u_border");
            GLES20.glUniform1f(uBorder, border);
            
            float[] color = makeColor(paint.m_strokeColor);
            int uColor = GLES20.glGetUniformLocation(m_progCircleStroke, "u_color");
            GLES20.glUniform4fv(uColor, 1, color, 0);
            
            int uAntialias = GLES20.glGetUniformLocation(m_progCircleStroke, "u_antialias");
            GLES20.glUniform1i(uAntialias, paint.m_antialias ? 1 : 0);
            
            int uNegative = GLES20.glGetUniformLocation(m_progCircleStroke, "u_negative");
            GLES20.glUniform1i(uNegative, (paint.m_colorFilter == JPaintColorFilter.PAINT_COLOR_FILTER_NEGATIVE) ? 1 : 0);
            
            int aPosition = GLES20.glGetAttribLocation(m_progCircleStroke, "a_position");
            GLES20.glEnableVertexAttribArray(aPosition);
            
            int outer = (((int)paint.m_strokeWidth + 1) >> 1) + 1;
            
            GLES20.glVertexAttribPointer(aPosition, 2, GLES20.GL_FLOAT, false, 0, makeRect((int)Math.round(x-radius-outer), (int)Math.round(y-radius-outer), (int)Math.round(x+radius+outer), (int)Math.round(y+radius+outer)));
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
        }
    }

    // Clipping
    
    public final void beginClipUpdate() {
        GLES20.glColorMask(false, false, false, false);
        GLES20.glEnable(GLES20.GL_STENCIL_TEST);
        GLES20.glStencilFunc(GLES20.GL_ALWAYS, 1, 1);
        GLES20.glStencilOp(GLES20.GL_KEEP, GLES20.GL_KEEP, GLES20.GL_REPLACE);
    }
    
    public final void clearClip() {
    	GLES20.glClearStencil(0);
    	GLES20.glClearColor(0, 0, 0, 1);
    	GLES20.glClear(GLES20.GL_STENCIL_BUFFER_BIT);
    }
    
    public final void endClipUpdate() {
    	GLES20.glDisable(GLES20.GL_STENCIL_TEST);
    	GLES20.glColorMask(true, true, true, true);
    }
    
    public final void beginClip() {
    	beginClip(false);
    }
    
    public final void beginClip(final boolean inverse) {
    	GLES20.glEnable(GLES20.GL_STENCIL_TEST);
    	GLES20.glStencilFunc(inverse ? GLES20.GL_EQUAL : GLES20.GL_NOTEQUAL, 1, 1);
    	GLES20.glStencilOp(GLES20.GL_KEEP, GLES20.GL_KEEP, GLES20.GL_KEEP);
    }
    
    public final void endClip() {
    	GLES20.glDisable(GLES20.GL_STENCIL_TEST);
    }
    
    // Framebuffer (back screen)
    
    private Stack<GLContextEntry> m_contextStack = new Stack<GLContextEntry>();
    
    public final void beginFramebufferUpdate(final GLFramebuffer buffer) {
    	GLContextEntry m_restoreContext = GLContextEntry.create();

    	m_restoreContext.m_size.copyFrom(m_size);
    	m_restoreContext.m_aspect.copyFrom(m_aspect);
    	m_restoreContext.m_client.copyFrom(m_client);
    	m_restoreContext.m_projection.copyFrom(m_projection);
    	
        GLES20.glGetIntegerv(GLES20.GL_SCISSOR_BOX, m_restoreContext.m_scissorBox.v, 0);
        GLES20.glGetIntegerv(GLES20.GL_SCISSOR_TEST, m_restoreContext.m_scissorTest.v, 0);
	    GLES20.glGetIntegerv(GLES20.GL_VIEWPORT, m_restoreContext.m_viewPort.v, 0);
	    GLES20.glGetIntegerv(GLES20.GL_FRAMEBUFFER_BINDING, m_restoreContext.m_frameBuffer.v, 0);
	    
	    m_size[0] = buffer.m_width;
        m_size[1] = buffer.m_height;
        m_aspect[0] = m_aspect[1] = 1.0f;
        m_client[0] = m_client[1] = m_client[4] = m_client[5] = 0;
        m_client[2] = m_client[6] = m_size[0];
        m_client[3] = m_client[7] = m_size[1];
        setProjection(0.0f, 0.0f);
        
	    GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, buffer.m_frameBuffer[0]);
	    GLES20.glScissor(0, 0, m_size[0], m_size[1]);
	    GLES20.glViewport(0, 0, m_size[0], m_size[1]);
	    
	    if (m_restoreContext.m_scissorTest.v[0] != 0) {
	    	GLES20.glDisable(GLES20.GL_SCISSOR_TEST);
	    }

	    m_contextStack.push(m_restoreContext);
    }
    
	public final void fillFramebufferRect(final GLFramebuffer buffer, final int x, final int y, final int w, final int h, final int color) {
		GLES20.glEnable(GLES20.GL_SCISSOR_TEST);
		GLES20.glScissor(x, buffer.m_height - y - h, w, h);
	    GLContext.fill(color);
	    GLES20.glDisable(GLES20.GL_SCISSOR_TEST);
	}

	public final void fillFramebufferRectAlpha(final GLFramebuffer buffer, final int x, final int y, final int w, final int h, final byte alpha) {
		GLES20.glEnable(GLES20.GL_SCISSOR_TEST);
		GLES20.glScissor(x, buffer.m_height - y - h, w, h);
		GLContext.fillAlpha(alpha);
		GLES20.glDisable(GLES20.GL_SCISSOR_TEST);
	}

    public final void endFramebufferUpdate() {
    	if (!m_contextStack.isEmpty()) {
    		GLContextEntry m_restoreContext = m_contextStack.pop();
    		
    		m_restoreContext.m_size.copyTo(m_size);
    		m_restoreContext.m_aspect.copyTo(m_aspect);
    		m_restoreContext.m_client.copyTo(m_client);
    		m_restoreContext.m_projection.copyTo(m_projection);
    		
    		GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, m_restoreContext.m_frameBuffer.v[0]);
    	    GLES20.glScissor(m_restoreContext.m_scissorBox.v[0], m_restoreContext.m_scissorBox.v[1], m_restoreContext.m_scissorBox.v[2], m_restoreContext.m_scissorBox.v[3]);
    	    GLES20.glViewport(m_restoreContext.m_viewPort.v[0], m_restoreContext.m_viewPort.v[1], m_restoreContext.m_viewPort.v[2], m_restoreContext.m_viewPort.v[3]);
    	    
    	    if (m_restoreContext.m_scissorTest.v[0] != 0) {
    	    	GLES20.glEnable(GLES20.GL_SCISSOR_TEST);
    	    } else {
    	    	GLES20.glDisable(GLES20.GL_SCISSOR_TEST);
    	    }
    	    
    	    m_restoreContext.release();
    	}
    }
    
    // Render
    
    private static int[] lastSize = { 0, 0 };
    
    public final boolean onRender(final int screenWidth, final int screenHeight, final float clientWidth, final float clientHeight) {
    	GLES20.glViewport(0, 0, screenWidth, screenHeight);
    	
        if (Float.isNaN(clientWidth) || Float.isInfinite(clientWidth) || (clientWidth == 0)) {
            m_aspect[0] = 1.0f;
        } else {
            m_aspect[0] = (float)screenWidth / clientWidth;
        }
        
        if (Float.isNaN(clientHeight) || Float.isInfinite(clientHeight) || (clientHeight == 0)) {
            m_aspect[1] = 1.0f;
        } else {
            m_aspect[1] = (float)screenHeight / clientHeight;
        }
        
        m_size[0] = (int) Math.ceil(clientWidth);
        m_size[1] = (int) Math.ceil(clientHeight);
        
        resetClient();
        
        if ((lastSize[0] != m_size[0]) || (lastSize[1] != m_size[1])) {
        	lastSize[0] = m_size[0];
        	lastSize[1] = m_size[1];
        	return true;
        }
        return false;
    }
    
    // set view clip, client rect,
    public final void setClient(final int vx1, final int vy1, final int vx2, final int vy2, final int cx1, final int cy1, final int cx2, final int cy2) {
        GLES20.glScissor(
        		(int) Math.ceil((float)(vx1) * m_aspect[0]),
        		(int) Math.round((float)(m_size[1] - vy2) * m_aspect[1]),
        		(int) Math.round((float)(vx2 - vx1) * m_aspect[0]),
        		(int) Math.ceil((float)(vy2 - vy1) * m_aspect[1]));
        GLES20.glEnable(GLES20.GL_SCISSOR_TEST);
        
        m_client[0] = vx1;
        m_client[1] = vy1;
        m_client[2] = vx2;
        m_client[3] = vy2;
        m_client[4] = cx1;
        m_client[5] = cy1;
        m_client[6] = cx2;
        m_client[7] = cy2;

        setProjection(cx1, cy1);
    }
    
    public final void resetClient() {
        setClient(0, 0, m_size[0], m_size[1], 0, 0, m_size[0], m_size[1]);
        GLES20.glDisable(GLES20.GL_SCISSOR_TEST);
    }
    
    public final int[] getClient() {
        return m_client;
    }

}
