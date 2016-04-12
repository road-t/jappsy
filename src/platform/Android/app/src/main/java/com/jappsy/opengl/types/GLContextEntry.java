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
