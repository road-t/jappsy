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

package com.jappsy.memory;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class MemoryManager {

	private Object m_lock = new Object();
	private List<Object> m_active = Collections.synchronizedList(new ArrayList<Object>());
	private List<Object> m_inactive = Collections.synchronizedList(new ArrayList<Object>());
	
	public Object getObject() {
		Object result = null;
		
		synchronized (m_lock) {
			if (m_inactive.size() > 0) {
				result = m_inactive.remove(0);
				m_active.add(result);
			}
			m_lock.notifyAll();
		}
		
		return result;
	}
	
	public Object addObject(Object o) {
		synchronized (m_lock) {
			m_active.add(o);
			m_lock.notifyAll();
		}
		return o;
	}
	
	public void returnObject(Object o) {
		synchronized (m_lock) {
			int idx = m_active.indexOf(o);
			if (idx >= 0) {
				m_active.remove(idx);
			}
			m_inactive.add(o);
			m_lock.notifyAll();
		}
	}
	
}
