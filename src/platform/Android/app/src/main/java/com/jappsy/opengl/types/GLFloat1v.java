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
