package com.jappsy.memory;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import com.jappsy.exceptions.EOutOfMemory;

public class Memory {

	private static final MemoryManager m_memBuffer4096 = new MemoryManager();
	private static final MemoryManager m_memBuffer102400 = new MemoryManager();
	
	public static byte[] mmalloc4096() throws EOutOfMemory {
		byte[] o = (byte[])(m_memBuffer4096.getObject());
		if (o == null) {
			try {
				o = (byte[])(m_memBuffer4096.addObject(new byte[4096]));
			} catch (OutOfMemoryError e) {
				throw new EOutOfMemory(e);
			}
		}
		
		return o;
	}
	
	public static void mmfree4096(byte[] o) {
		m_memBuffer4096.returnObject(o);
	}

	public static byte[] mmalloc102400() throws EOutOfMemory {
		byte[] o = (byte[])(m_memBuffer102400.getObject());
		if (o == null) {
			try {
				o = (byte[])(m_memBuffer102400.addObject(new byte[102400]));
			} catch (OutOfMemoryError e) {
				throw new EOutOfMemory(e);
			}
		}

		return o;
	}
	
	public static void mmfree102400(byte[] o) {
		m_memBuffer102400.returnObject(o);
	}

	private static class StackIntItem {
		public int[] ptr;
		public StackIntItem next;
	}
	
	private static final int MM_STACK_INT_MAX = 18; // 1 MB
	private static StackIntItem[] mmStackInt = new StackIntItem[MM_STACK_INT_MAX];
	private static StackIntItem mmCacheInt;
	
	private static final int mmGetIndex(int size) {
		int idx = 0;
		while (size > 0) {
			idx++;
			size >>= 1;
		}
		return idx;
	}
	
	public static final int[] mmVirtualAllocInt(int size) throws EOutOfMemory {
		if (size == 0)
			return null;
		
		int idx = mmGetIndex(size);
		
		if (idx >= MM_STACK_INT_MAX) {
			try {
				return new int[size];
			} catch (OutOfMemoryError e) {
				throw new EOutOfMemory(e);
			}
		}
		
		int[] ptr;
		synchronized (mmStackInt) {
			if (mmStackInt[idx] != null) {
				StackIntItem item = mmStackInt[idx];
				ptr = item.ptr;
				item.ptr = null;
				item.next = mmCacheInt;
				mmCacheInt = item;
				Arrays.fill(ptr, 0);
			} else {
				try {
					ptr = new int[1 << idx];
				} catch (OutOfMemoryError e) {
					throw new EOutOfMemory(e);
				}
			}
		}

		return ptr;
	}
	
	public static final void mmVirtualFreeInt(int[] ptr) throws EOutOfMemory {
		int size = ptr.length;
		
		int idx = mmGetIndex(size);
		
		if (idx < MM_STACK_INT_MAX) {
			synchronized (mmStackInt) {
				StackIntItem item = mmCacheInt;
				if (item != null) {
					mmCacheInt = item.next;
				} else {
					try {
						item = new StackIntItem();
					} catch (OutOfMemoryError e) {
						throw new EOutOfMemory(e);
					}
				}
				
				item.next = mmStackInt[idx];
				item.ptr = ptr;
				mmStackInt[idx] = item;
			}
		}
	}
	
}
