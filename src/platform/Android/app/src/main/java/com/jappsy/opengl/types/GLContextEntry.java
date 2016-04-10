package com.jappsy.opengl.types;

import com.jappsy.memory.MemoryManager;

public class GLContextEntry {
	public GLInt2v m_size = new GLInt2v();
	public GLFloat2v m_aspect = new GLFloat2v();
	public GLInt8v m_client = new GLInt8v();
	public GLProjectionMatrix m_projection = new GLProjectionMatrix();
	public GLInt4v m_scissorBox = new GLInt4v();
	public GLInt1v m_scissorTest = new GLInt1v();
	public GLInt4v m_viewPort = new GLInt4v();
	public GLInt1v m_frameBuffer = new GLInt1v();

	private static final MemoryManager m_mem = new MemoryManager();
	
	public final static GLContextEntry create() {
		GLContextEntry o = (GLContextEntry)(m_mem.getObject());
		if (o == null) o = (GLContextEntry)(m_mem.addObject(new GLContextEntry()));
		return o;
	}
	
	public final void release() {
		m_mem.returnObject(this);
	}
}
