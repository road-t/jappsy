package com.jappsy.opengl.types;

import com.jappsy.memory.MemoryManager;

public class GLInt8v {
	public int[] v = new int[8];
	
    // Constructor & Destructor
	private static final MemoryManager m_mem = new MemoryManager();
	
	public static GLInt8v create() {
		GLInt8v o = (GLInt8v)(m_mem.getObject());
		if (o == null) o = (GLInt8v)(m_mem.addObject(new GLInt8v()));
		o.onCreate();
		return o;
	}
	
	private void clear() {
		for (int i = 0; i < 16; i++) v[i] = 0;
	}
	
	protected void onCreate() {
		clear();
	}
	
	public void release() {
		clear();
		m_mem.returnObject(this);
	}
	
	public void copyFrom(int[] src) {
		for (int i = 0; i < 8; i++) v[i] = src[i];
	}
	
	public void copyTo(int[] dst) {
		for (int i = 0; i < 8; i++) dst[i] = v[i];
	}
}
