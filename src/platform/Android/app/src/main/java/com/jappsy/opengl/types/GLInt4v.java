package com.jappsy.opengl.types;

import com.jappsy.memory.MemoryManager;

public class GLInt4v {
	public int[] v = new int[4];
	
    // Constructor & Destructor
	private static final MemoryManager m_mem = new MemoryManager();
	
	public static GLInt4v create() {
		GLInt4v o = (GLInt4v)(m_mem.getObject());
		if (o == null) o = (GLInt4v)(m_mem.addObject(new GLInt4v()));
		o.onCreate();
		return o;
	}
	
	private void clear() {
		v[0] = v[1] = v[2] = v[3] = 0;
	}
	
	protected void onCreate() {
		clear();
	}
	
	public void release() {
		clear();
		m_mem.returnObject(this);
	}
	
	public void copyFrom(int[] src) {
		v[0] = src[0];
		v[1] = src[1];
		v[2] = src[2];
		v[3] = src[3];
	}
	
	public void copyTo(int[] dst) {
		dst[0] = v[0];
		dst[1] = v[1];
		dst[2] = v[2];
		dst[3] = v[3];
	}
}
