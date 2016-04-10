package com.jappsy.opengl.types;

import com.jappsy.memory.MemoryManager;

public class GLFloat1v {
	public float[] v = new float[1];
	
    // Constructor & Destructor
	private static final MemoryManager m_mem = new MemoryManager();
	
	public static GLFloat1v create() {
		GLFloat1v o = (GLFloat1v)(m_mem.getObject());
		if (o == null) o = (GLFloat1v)(m_mem.addObject(new GLFloat1v()));
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

	public void copyFrom(float[] src) {
		v[0] = src[0];
	}
	
	public void copyTo(float[] dst) {
		dst[0] = v[0];
	}
}
