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

#ifndef JAPPSY_UMAP_H
#define JAPPSY_UMAP_H

#include <data/uSet.h>
#include <data/uJSON.h>

template <typename K, typename V>
class RefMap : public RefObject {
public:
	Set<K> m_keys;
	List<V> m_values;
	
public:
	
	inline RefMap() throw(const char*) {
		TYPE = TypeMap;
		m_keys = new Set<K>();
		m_values = new List<V>();
	}
	
	inline RefMap(int initialCapacity) throw(const char*) {
		TYPE = TypeMap;
		m_keys = new Set<K>(initialCapacity);
		m_values = new List<V>(initialCapacity);
	}
	
	virtual inline void clear() throw(const char*) {
		m_keys.clear();
		m_values.clear();
	}
	
	virtual inline bool containsKey(const K& key) const {
		return m_keys->RefStack<K>::contains(key);
	}
	
	virtual inline bool containsValue(const V& value) const {
		return m_values->RefStack<V>::contains(value);
	}
	
	virtual inline const V& get(const K& key) const throw(const char*) {
		int32_t index = m_keys->RefStack<K>::search(key);
		if (index >= 0)
			return m_values->RefStack<V>::get(index);
		throw eNotFound;
	}
	
	virtual inline const V& opt(const K& key, const V& defaultValue) const {
		int32_t index = m_keys->RefStack<K>::search(key);
		if (index >= 0) {
			try {
				return m_values->RefStack<V>::get(index);
			} catch (const char* e) {
			}
		}
		return defaultValue;
	}
	
	virtual inline bool isEmpty() const {
		return m_keys->RefStack<K>::empty();
	}
	
	virtual inline const Set<K>& keySet() const {
		return m_keys;
	}
	
	virtual inline V& put(const K& key, const V& value) throw(const char*) {
		int32_t index = m_keys->RefStack<K>::search(key);
		if (index < 0) {
			m_keys->RefStack<K>::push(key);
			return m_values->RefStack<V>::push(value);
		} else {
			return m_values->RefList<V>::set(index, value);
		}
	}
	
	virtual inline void remove(const K& key) throw(const char*) {
		int32_t index = m_keys->RefStack<K>::search(key);
		if (index >= 0) {
			m_keys->RefStack<K>::remove(index);
			m_values->RefStack<V>::remove(index);
		}
	}
	
	virtual inline int32_t size() const {
		return m_keys->RefStack<K>::size();
	}
	
	virtual inline const Collection<V>& values() const {
		return *((Collection<V>*)&m_values);
	}
	
	virtual inline String toJSON() const {
		String json = L"{";
		Iterator<K> itKey = this->keySet().iterator();
		Iterator<V> itVal = this->values().iterator();
		bool first = true;
		while (itKey.hasNext()) {
			if (first) first = false; else json += L",";
			json += JSON::keyify(itKey.next());
			json += L":";
			json += JSON::stringify(itVal.next());
		}
		json += L"}";
		return json;
	}
};

template <typename K, typename V>
class SynchronizedMap;

template <typename K, typename V>
class Map : public Object {
public:
	RefClass(Map, Map<K,V>)
	
	inline Map(uint32_t initialCapacity) {
		RefMap<K,V>* o = new RefMap<K,V>(initialCapacity);
		if (o == NULL) throw eOutOfMemory;
		this->setRef(o);
	}
	
	virtual inline void clear() throw(const char*) { CHECKTHIS; THIS->clear(); }
	virtual inline bool containsKey(const K& key) const throw(const char*) { CHECKTHIS; return THIS->containsKey(key); }
	virtual inline bool containsValue(const V& value) const throw(const char*) { CHECKTHIS; return THIS->containsValue(value); }
	// entrySet
	virtual inline const V& get(const K& key) const throw(const char*) { CHECKTHIS; return THIS->get(key); }
	virtual inline const V& opt(const K& key, const V& defaultValue) const throw(const char*) { CHECKTHIS; return THIS->opt(key, defaultValue); }
	virtual inline bool isEmpty() const throw(const char*) { CHECKTHIS; return THIS->isEmpty(); }
	virtual inline const Set<K>& keySet() const throw(const char*) { CHECKTHIS; return THIS->keySet(); }
	virtual inline V& put(const K& key, const V& value) throw(const char*) { CHECKTHIS; return THIS->put(key, value); }
	// putAll
	virtual inline void remove(const K& key) throw(const char*) { CHECKTHIS; THIS->remove(key); }
	virtual inline int32_t size() const throw(const char*) { CHECKTHIS; return THIS->size(); }
	virtual inline const Collection<V>& values() const throw(const char*) { CHECKTHIS; return THIS->values(); }
	
	static SynchronizedMap<K,V> synchronizedMap(Map<K,V>* newMap) {
		return SynchronizedMap<K,V>(newMap);
	}

#ifdef DEBUG
	inline static void _test() {
		Map<Object, Object> map = new Map<Object, Object>();
		map.clear();
		map.containsKey(null);
		map.containsValue(null);
		map.put(null, null);
		map.get(null);
		map.isEmpty();
		map.keySet();
		map.put(null, null);
		map.remove(null);
		map.size();
		map.values();
	}
#endif
};

template <typename K, typename V>
class SynchronizedMap : public Map<K,V> {
public:
	RefClass(SynchronizedMap, Map<K,V>)
	
	virtual inline void clear() throw(const char*) {
		synchronized(this) {
			try {
				THIS->clear();
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
	}
	
	virtual inline bool containsKey(const K& key) const throw(const char*) {
		bool result;
		synchronized(this) {
			result = THIS->containsKey(key);
			this->notifyAll();
		}
		return result;
	}
	
	virtual inline bool containsValue(const V& value) const throw(const char*) {
		bool result;
		synchronized(this) {
			result = THIS->containsValue(value);
			this->notifyAll();
		}
		return result;
	}
	
	virtual inline const V& get(const K& key) const throw(const char*) {
		const V* result;
		synchronized(this) {
			try {
				result = &(THIS->get(key));
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
		return *result;
	}
	
	virtual inline const V& opt(const K& key, const V& defaultValue) const throw(const char*) {
		const V* result;
		synchronized(this) {
			try {
				result = &(THIS->opt(key, defaultValue));
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
		return *result;
	}
	
	virtual inline bool isEmpty() const throw(const char*) {
		bool result;
		synchronized(this) {
			result = THIS->isEmpty();
			this->notifyAll();
		}
		return result;
	}
	
	virtual inline const Set<K>& keySet() const throw(const char*) {
		const Set<K>* result;
		synchronized(this) {
			result = &(THIS->keySet());
			this->notifyAll();
		}
		return *result;
	}
	
	virtual inline V& put(const K& key, const V& value) throw(const char*) {
		V* result;
		synchronized(this) {
			try {
				result = &(THIS->put(key, value));
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
		return *result;
	}
	
	virtual inline void remove(const K& key) throw(const char*) {
		synchronized(this) {
			try {
				THIS->remove(key);
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
	}
	
	virtual inline int32_t size() const throw(const char*) {
		bool result;
		synchronized(this) {
			result = THIS->size();
			this->notifyAll();
		}
		return result;
	}
	
	virtual inline const Collection<V>& values() const throw(const char*) {
		const Collection<V>* result;
		synchronized(this) {
			try {
				result = &(THIS->values());
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
		return *result;
	}

#ifdef DEBUG
	inline static void _test() {
		SynchronizedMap<Object, Object> map = Map<Object, Object>::synchronizedMap(new Map<Object, Object>());
		map.clear();
		map.containsKey(null);
		map.containsValue(null);
		map.put(null, null);
		map.get(null);
		map.isEmpty();
		map.keySet();
		map.put(null, null);
		map.remove(null);
		map.size();
		map.values();
	}
#endif
};

#endif //JAPPSY_UMAP_H