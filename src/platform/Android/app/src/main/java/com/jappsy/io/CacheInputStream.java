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

package com.jappsy.io;

import java.io.IOException;
import java.io.InputStream;

import com.jappsy.exceptions.EIO;
import com.jappsy.exceptions.EIOFileNotOpen;
import com.jappsy.exceptions.EOutOfMemory;
import com.jappsy.memory.MemoryManager;

public class CacheInputStream extends InputStream {

	private CacheFile m_file;
	
	private static final MemoryManager m_mem = new MemoryManager();
	
	public static final CacheInputStream create(final CacheFile file) throws EIOFileNotOpen, EOutOfMemory  {
		if (file == null)
			throw new EIOFileNotOpen();
		
		CacheInputStream o = (CacheInputStream)(m_mem.getObject());
		try {
			if (o == null) {
				o = (CacheInputStream)(m_mem.addObject(new CacheInputStream()));
				o.m_file = file;
			}
		} catch (OutOfMemoryError e) {
			throw new EOutOfMemory(e);
		}
		
		return o;
	}
	
	public final void release() {
		close();
		m_mem.returnObject(this);
	}

	@Override
	public final int available() throws IOException {
		if (m_file != null) {
			try {
				return m_file.available();
			} catch (EIOFileNotOpen e) {
				throw new IOException(e);
			}
		}
		
		throw new IOException(EIOFileNotOpen.eIOErrorFileNotOpen);
	}
	
	@Override
	public final void close() {
		m_file = null;
	}
	
	@Override
	public final int read(byte[] buffer) throws IOException {
		if (m_file != null) {
			try {
				return m_file.read(buffer);
			} catch (EIO e) {
				throw new IOException(e);
			}
		}
		
		throw new IOException(EIOFileNotOpen.eIOErrorFileNotOpen);
	}
	
	@Override
	public final int read() throws IOException {
		if (m_file != null) {
			try {
				return m_file.read();
			} catch (EIO e) {
				throw new IOException(e);
			}
		}
		
		throw new IOException(EIOFileNotOpen.eIOErrorFileNotOpen);
	}
	
	@Override
	public final int read(byte[] buffer, int byteOffset, int byteCount) throws IOException {
		if (m_file != null) {
			try {
				return m_file.read(buffer, byteOffset, byteCount);
			} catch (EIO e) {
				throw new IOException(e);
			}
		}
		
		throw new IOException(EIOFileNotOpen.eIOErrorFileNotOpen);
	}

	@Override
	public final long skip(long byteCount) throws IOException {
		if (m_file != null) {
			try {
				return m_file.skip(byteCount);
			} catch (EIO e) {
				throw new IOException(e);
			}
		}
		
		throw new IOException(EIOFileNotOpen.eIOErrorFileNotOpen);
	}
	
}
