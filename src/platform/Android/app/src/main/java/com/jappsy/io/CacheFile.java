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

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.RandomAccessFile;
import java.util.zip.GZIPInputStream;

import com.jappsy.exceptions.EGlobal;
import com.jappsy.exceptions.EIO;
import com.jappsy.exceptions.EIOFileNotFound;
import com.jappsy.exceptions.EIOFileNotOpen;
import com.jappsy.exceptions.EIOInvalidFormat;
import com.jappsy.exceptions.EIOReadLimit;
import com.jappsy.exceptions.EOutOfMemory;
import com.jappsy.memory.Memory;
import com.jappsy.memory.MemoryManager;

public class CacheFile {

	private File m_file;
	private RandomAccessFile m_handle;
	private String m_access;
	private long m_readLimit = -1;
	private byte[] readBuffer = new byte[8];
	
	private static final MemoryManager m_mem = new MemoryManager();
	
	public static CacheFile create() throws EOutOfMemory {
		CacheFile o = (CacheFile)(m_mem.getObject());
		try {
			if (o == null) {
				o = (CacheFile)(m_mem.addObject(new CacheFile()));
				o.clear();
			}
		} catch (OutOfMemoryError e) {
			throw new EOutOfMemory(e);
		}

		return o;
	}
	
	public static CacheFile create(final File cacheDir, final String fileName) throws EOutOfMemory, EIO {
		return create(cacheDir, fileName, true, false);
	}
	
	public static CacheFile create(final File cacheDir, final String fileName, final boolean readOnly) throws EOutOfMemory, EIO {
		return create(cacheDir, fileName, readOnly, false);
	}
	
	public static CacheFile create(final File cacheDir, final String fileName, final boolean readOnly, final boolean sync) throws EOutOfMemory, EIO {
		CacheFile o = (CacheFile)(m_mem.getObject());
		try {
			if (o == null) {
				o = (CacheFile)(m_mem.addObject(new CacheFile()));
				o.clear();
			}
		} catch (OutOfMemoryError e) {
			throw new EOutOfMemory(e);
		}

		try {
			o.open(cacheDir, fileName, readOnly, sync);
		} catch (EIO | EOutOfMemory e) {
			o.release();
			throw e;
		}

		return o;
	}
	
	public static CacheFile createFromStream(final File cacheDir, final String fileName, InputStream is) throws EIO, EOutOfMemory {
		CacheFile o = create(cacheDir, fileName, false, false);
		try {
			o.setLength(0);
			o.write(is);
			o.seek(0);
		} catch (EGlobal e) {
			File file = o.m_file;
			o.release();
			file.delete();
			throw new EIO(e);
		}
		return o;
	}
	
	protected final void clear() {
		m_file = null;
		m_handle = null;
		m_access = null;
		m_readLimit = -1;
	}
	
	public final void release() {
		try { close(); } catch (EIO e) {};
		m_mem.returnObject(this);
	}
	
	public final void open(final File cacheDir, final String fileName) throws EIO, EOutOfMemory {
		open(cacheDir, fileName, true, false);
	}
	
	public final void open(final File cacheDir, final String fileName, final boolean readOnly) throws EIO, EOutOfMemory {
		open(cacheDir, fileName, readOnly, false);
	}
	
	public final void open(final File cacheDir, final String fileName, final boolean readOnly, final boolean sync) throws EIO, EOutOfMemory {
		File newFile;
		try {
			newFile = new File(cacheDir.getPath(), fileName);
		} catch (OutOfMemoryError e) {
			throw new EOutOfMemory(e);
		}
		String newAccess;
		if (readOnly) {
			newAccess = "r";
		} else if (sync) {
			newAccess = "rws";
		} else {
			newAccess = "rw";
		}
		
		m_readLimit = -1;

		if (m_file != null) {
			if (m_file.equals(newFile)) {
				if (m_access.equals(newAccess)) {
					try {
						m_handle.seek(0);
					} catch (IOException e) {
						close();
						throw new EIO(e);
					}
					return;
				}
			}
			close();
		}
		
		m_file = newFile;
		m_access = newAccess;
		try {
			m_handle = new RandomAccessFile(m_file, m_access);
		} catch (FileNotFoundException e) {
			close();
			throw new EIOFileNotFound(e);
		} catch (OutOfMemoryError e) {
			close();
			throw new EOutOfMemory(e);
		}
	}
	
	public final void close() throws EIO {
		RandomAccessFile temp = m_handle;

		m_readLimit = -1;
		m_access = null;
		m_handle = null;
		if (m_file == null)
			throw new EIOFileNotOpen();
		
		m_file = null;
		if (temp != null) {
			try {
				temp.close();
			} catch (IOException e) {
				throw new EIO(e);
			}
		}
	}
	
	public final String getName() {
		return m_file.getName();
	}

	public final FileDescriptor getFD() throws EIO {
		if (m_file == null)
			throw new EIOFileNotOpen();

		try {
			return m_handle.getFD();
		} catch (IOException e) {
			throw new EIO(e);
		}
	}

	public final long getLength() throws EIO {
		if (m_file == null)
			throw new EIOFileNotOpen();

		try {
			return m_handle.length();
		} catch (IOException e) {
			throw new EIO(e);
		}
	}

	public final void setLength(long newLength) throws EIO {
		if (m_file == null)
			throw new EIOFileNotOpen();
		
		try {
			m_handle.setLength(newLength);
		} catch (IOException e) {
			throw new EIO(e);
		}
	}

	public final int available() throws EIOFileNotOpen {
		if (m_file == null)
			throw new EIOFileNotOpen();
		
		return 0;
	}
	
	public final void readLimit(long limit) {
		m_readLimit = limit;
	}
	
	public final long skip(long byteCount) throws EIO {
		if (m_file == null)
			throw new EIOFileNotOpen();
		
		if (byteCount < 0)
			return -1;
		
		if (m_readLimit == 0)
			return -1;
		
		if (m_readLimit > 0) {
			if (byteCount > m_readLimit)
				byteCount = m_readLimit;
		}

		long done = 0;
		long skipped;
		while (byteCount > 0) {
			skipped = 0;
			if (byteCount > 0x40000000) {
				try {
					skipped = m_handle.skipBytes(0x40000000);
				} catch (IOException e) {
					throw new EIO(e);
				}
			} else {
				try {
					skipped = m_handle.skipBytes((int)byteCount);
				} catch (IOException e) {
					throw new EIO(e);
				}
			}
			if (skipped == 0) break;
			byteCount -= skipped;
			done += skipped;
		}
		
		if (m_readLimit > 0) 
			m_readLimit -= done;

		return done;
	}
	
	public final long getPosition() throws EIO {
		if (m_file == null)
			throw new EIOFileNotOpen();

		try {
			return m_handle.getFilePointer();
		} catch (IOException e) {
			throw new EIO(e);
		}
	}
	
	public final void seek(long position) throws EIO {
		if (m_file == null)
			throw new EIOFileNotOpen();
		
		m_readLimit = -1;
		try {
			m_handle.seek(position);
		} catch (IOException e) {
			throw new EIO(e);
		}
	}
	
	public final int read() throws EIO {
		if (m_file == null)
			throw new EIOFileNotOpen();
		
		if (m_readLimit == 0)
			return -1;
		
		int actualRead;
		try {
			actualRead = m_handle.read();
		} catch (IOException e) {
			throw new EIO(e);
		}
		
		if (m_readLimit > 0) {
			if (actualRead > 0)
				m_readLimit--;
		}
		
		return actualRead;
	}
	
	public final int read(byte[] buffer) throws EIO {
		if (m_file == null)
			throw new EIOFileNotOpen();
		
		if (m_readLimit == 0)
			return -1;
		
		int actualRead;
		if (m_readLimit > 0) {
			if (buffer.length > m_readLimit) {
				try {
					actualRead = m_handle.read(buffer, 0, (int)m_readLimit);
				} catch (IOException e) {
					throw new EIO(e);
				}
			} else {
				try {
					actualRead = m_handle.read(buffer);
				} catch (IOException e) {
					throw new EIO(e);
				}
			}
			if (actualRead > 0)
				m_readLimit -= actualRead;
		} else {
			try {
				actualRead = m_handle.read(buffer);
			} catch (IOException e) {
				throw new EIO(e);
			}
		}

		return actualRead;
	}
	
	public final int read(byte[] buffer, int byteOffset, int byteCount) throws EIO {
		if (m_file == null)
			throw new EIOFileNotOpen();
		
		if (m_readLimit == 0)
			return -1;
		
		if (byteCount < 0)
            throw new IndexOutOfBoundsException();
		
		int actualRead;
		if (m_readLimit > 0) {
			if (byteCount > m_readLimit) {
				try {
					actualRead = m_handle.read(buffer, byteOffset, (int)m_readLimit);
				} catch (IOException e) {
					throw new EIO(e);
				}
			} else {
				try {
					actualRead = m_handle.read(buffer, byteOffset, byteCount);
				} catch (IOException e) {
					throw new EIO(e);
				}
			}
			if (actualRead > 0)
				m_readLimit -= actualRead;
		} else {
			try {
				actualRead = m_handle.read(buffer, byteOffset, byteCount);
			} catch (IOException e) {
				throw new EIO(e);
			}
		}

		return actualRead;
	}
	
	public final void readFully(byte[] buffer, int byteOffset, int byteCount) throws EIO {
		if (m_file == null)
			throw new EIOFileNotOpen();
		
		if (m_readLimit == 0)
			throw new EIOReadLimit();
		
        if (byteCount < 0)
            throw new IndexOutOfBoundsException();
        
        if (m_readLimit > 0) {
        	if (byteCount > m_readLimit)
        		throw new EIOReadLimit();
        }
        
    	long pos;
		try {
			pos = m_handle.getFilePointer();
		} catch (IOException e) {
			throw new EIO(e);
		}
        int n = 0;
        while (n < byteCount) {
            int count;
			try {
				count = m_handle.read(buffer, byteOffset + n, byteCount - n);
			} catch (IOException e) {
				throw new EIO(e);
			}
            if (count < 0) {
            	try {
					m_handle.seek(pos);
				} catch (IOException e) {
					throw new EIO(e);
				}
            }
            n += count;
        }
        
        if (m_readLimit > 0) {
        	m_readLimit -= byteCount;
        }
    }
	
	public final void readFully(byte[] buffer) throws EIO {
		readFully(buffer, 0, buffer.length);
    }
	
	public final boolean readBoolean() throws EIO {
        int ch = read();
        if (ch < 0)
        	throw new EIOReadLimit();
        return (ch != 0);
    }
	
	public final byte readByte() throws EIO {
        int ch = read();
        if (ch < 0)
        	throw new EIOReadLimit();
        return (byte)(ch);
    }
	
	public final int readUnsignedByte() throws EIO {
        int ch = read();
        if (ch < 0)
        	throw new EIOReadLimit();
        return ch;
    }
	
	public final short readShort() throws EIO {
        int ch1 = read();
        int ch2 = read();
        if ((ch1 | ch2) < 0)
        	throw new EIOReadLimit();
        return (short)((ch2 << 8) + (ch1 << 0));
    }
	
	public final int readUnsignedShort() throws EIO {
        int ch1 = read();
        int ch2 = read();
        if ((ch1 | ch2) < 0)
        	throw new EIOReadLimit();
        return (ch2 << 8) + (ch1 << 0);
    }
	
	public final char readChar() throws EIO {
        int ch1 = read();
        int ch2 = read();
        if ((ch1 | ch2) < 0)
        	throw new EIOReadLimit();
        return (char)((ch2 << 8) + (ch1 << 0));
    }
	
	public final int readInt() throws EIO {
        int ch1 = read();
        int ch2 = read();
        int ch3 = read();
        int ch4 = read();
        if ((ch1 | ch2 | ch3 | ch4) < 0)
        	throw new EIOReadLimit();
        return ((ch4 << 24) + (ch3 << 16) + (ch2 << 8) + (ch1 << 0));
    }
	
	public final long readLong() throws EIO {
        readFully(readBuffer, 0, 8);
        return (((long)readBuffer[7] << 56) +
                ((long)(readBuffer[6] & 255) << 48) +
                ((long)(readBuffer[5] & 255) << 40) +
                ((long)(readBuffer[4] & 255) << 32) +
                ((long)(readBuffer[3] & 255) << 24) +
                ((readBuffer[2] & 255) << 16) +
                ((readBuffer[1] & 255) <<  8) +
                ((readBuffer[0] & 255) <<  0));
    }
	
    public final float readFloat() throws EIO {
    	return Float.intBitsToFloat(readInt());
    }
    
    public final double readDouble() throws EIO {
        return Double.longBitsToDouble(readLong());
    }

    // EIOInvalidFormat for non GZIP format
    public final GZIPInputStream createGZIPInputStream(long readLimit) throws EOutOfMemory, EIO {
    	if (m_file == null)
    		throw new EIOFileNotOpen();
    	
    	m_readLimit = -1;
    	long pos = getPosition();
    	int head = 0;
    	try {
    		head = readUnsignedShort();
    	} finally {
    		seek(pos);
        	m_readLimit = readLimit;
    	}
    	
        if( GZIPInputStream.GZIP_MAGIC == head )  {
        	CacheInputStream cis = CacheInputStream.create(this);
    		try {
				return new GZIPInputStream(cis);
			} catch (IOException e) {
				throw new EIO(e);
			} catch (OutOfMemoryError e) {
				throw new EOutOfMemory(e);
			}
        }
        
        throw new EIOInvalidFormat();
    }
    
    public final InputStream createInputStream(long readLimit) throws EIOFileNotOpen, EOutOfMemory {
    	if (m_file == null)
    		throw new EIOFileNotOpen();
    	
    	m_readLimit = readLimit;
    	
       	return CacheInputStream.create(this);
    }
    
    public final void closeStream(InputStream stream) throws EIO {
    	if (m_readLimit > 0)
    		skip(m_readLimit);
    	
    	m_readLimit = -1;
    	try {
			stream.close();
		} catch (IOException e) {
			throw new EIO(e);
		}
    }
    
    private final long write(InputStream stream) throws EOutOfMemory, EIO {
    	if (m_file == null)
    		throw new EIOFileNotOpen();
    	
    	long size = 0;
        byte[] buffer = Memory.mmalloc102400();
        int length;
        try {
			while ((length = stream.read(buffer)) >= 0) {
				m_handle.write(buffer, 0, length);
				size += length;
			}
		} catch (IOException e) {
			throw new EIO(e);
		} finally {
			Memory.mmfree102400(buffer);
		}
        return size;
    }
}
