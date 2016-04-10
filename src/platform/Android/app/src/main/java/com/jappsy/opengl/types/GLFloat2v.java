package com.jappsy.opengl.types;

import com.jappsy.memory.MemoryManager;

public class GLFloat2v {
	public float[] v = new float[2];
	
    // Constructor & Destructor
	private static final MemoryManager m_mem = new MemoryManager();
	
	public static GLFloat2v create() {
		GLFloat2v o = (GLFloat2v)(m_mem.getObject());
		if (o == null) o = (GLFloat2v)(m_mem.addObject(new GLFloat2v()));
		o.onCreate();
		return o;
	}
	
	private void clear() {
		v[0] = v[1] = 0;
	}
	
	protected void onCreate() {
		clear();
	}
	
	public void release() {
		clear();
		m_mem.returnObject(this);
	}
	
	public void copyFrom(float[] src) {
		v[0] = src[0];
		v[1] = src[1];
	}
	
	public void copyTo(float[] dst) {
		dst[0] = v[0];
		dst[1] = v[1];
	}
}
