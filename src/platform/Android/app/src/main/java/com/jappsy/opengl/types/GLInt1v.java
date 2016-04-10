package com.jappsy.opengl.types;

import com.jappsy.memory.MemoryManager;

public class GLInt1v {
	public int[] v = new int[1];
	
    // Constructor & Destructor
	private static final MemoryManager m_mem = new MemoryManager();
	
	public static GLInt1v create() {
		GLInt1v o = (GLInt1v)(m_mem.getObject());
		if (o == null) o = (GLInt1v)(m_mem.addObject(new GLInt1v()));
		o.onCreate();
		return o;
	}
	
	private void clear() {
		v[0] = 0;
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
	}
	
	public void copyTo(int[] dst) {
		dst[0] = v[0];
	}
}
