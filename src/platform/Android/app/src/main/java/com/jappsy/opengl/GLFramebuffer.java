package com.jappsy.opengl;

import com.jappsy.exceptions.EOutOfMemory;
import com.jappsy.memory.MemoryManager;
import com.jappsy.opengl.image.GLImage;

public class GLFramebuffer extends GLImage {

	public int[] m_frameBuffer = new int[1];
	private int[] m_stencilBuffer = new int[1];

	private static final MemoryManager m_mem = new MemoryManager();
	
	public static final GLFramebuffer create(int w, int h) throws EOutOfMemory {
		return create(w, h, false);
	}

	public static final GLFramebuffer create(int width, int height, boolean smooth) throws EOutOfMemory {
		GLFramebuffer o = (GLFramebuffer)(m_mem.getObject());
		if (o == null) {
			try {
				o = (GLFramebuffer)(m_mem.addObject(new GLFramebuffer()));
			} catch (OutOfMemoryError e) {
				throw new EOutOfMemory(e);
			}
		}
		
		try {
			int[] handle = {0};
			GLContext.createFramebuffer(width, height, handle, o.m_frameBuffer, o.m_stencilBuffer, smooth);
			try {
				o.initFromHandle(handle, GLImage.GLImageStyleFlip, width, height, width, height);
			} catch (EOutOfMemory e) {
				GLContext.destroyFramebuffer(o.m_frameBuffer, o.m_stencilBuffer);
				GLContext.destroyTexture(handle);
				throw e;
			}
		} catch (EOutOfMemory e) {
			o.release();
			throw e;
		}

		return o;
	}
	
	public void release() throws EOutOfMemory {
		try {
			clear();
		} catch (EOutOfMemory e) {
			throw e;
		} finally {
			GLContext.destroyFramebuffer(m_frameBuffer, m_stencilBuffer);
			m_mem.returnObject(this);
		}
	}

}
