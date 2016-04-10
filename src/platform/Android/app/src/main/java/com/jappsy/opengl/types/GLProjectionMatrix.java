package com.jappsy.opengl.types;

import com.jappsy.memory.MemoryManager;

public class GLProjectionMatrix {
	public float[] v = new float[16];
	
    // Constructor & Destructor
	private static final MemoryManager m_mem = new MemoryManager();
	
	public static GLProjectionMatrix create() {
		GLProjectionMatrix o = (GLProjectionMatrix)(m_mem.getObject());
		if (o == null) o = (GLProjectionMatrix)(m_mem.addObject(new GLProjectionMatrix()));
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
	
	public void copyFrom(float[] src) {
		for (int i = 0; i < 16; i++) v[i] = src[i];
	}
	
	public void copyTo(float[] dst) {
		for (int i = 0; i < 16; i++) dst[i] = v[i];
	}
}
