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
class RefMap : public JRefObject {
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
		return m_keys.ref().JRefStack<K>::contains(key);
	}
	
	virtual inline bool containsValue(const V& value) const {
		return m_values.ref().JRefStack<V>::contains(value);
	}
	
	virtual inline const V& get(const K& key) const throw(const char*) {
		int32_t index = m_keys.ref().JRefStack<K>::search(key);
		if (index >= 0)
			return m_values.ref().JRefStack<V>::get(index);
		throw eNotFound;
	}
	
	virtual inline const V& opt(const K& key, const V& defaultValue) const {
		int32_t index = m_keys.ref().JRefStack<K>::search(key);
		if (index >= 0) {
			try {
				return m_values.ref().JRefStack<V>::get(index);
			} catch (const char* e) {
			}
		}
		return defaultValue;
	}
	
	virtual inline bool isEmpty() const {
		return m_keys.ref().JRefStack<K>::empty();
	}
	
	virtual inline const Set<K>& keySet() const {
		return m_keys;
	}
	
	virtual inline V& put(const K& key, const V& value) throw(const char*) {
		int32_t index = m_keys.ref().JRefStack<K>::search(key);
		if (index < 0) {
			m_keys.ref().JRefStack<K>::push(key);
			return m_values.ref().JRefStack<V>::push(value);
		} else {
			return m_values.ref().RefList<V>::set(index, value);
		}
	}
	
	virtual inline void remove(const K& key) throw(const char*) {
		int32_t index = m_keys.ref().JRefStack<K>::search(key);
		if (index >= 0) {
			m_keys.ref().JRefStack<K>::remove(index);
			m_values.ref().JRefStack<V>::remove(index);
		}
	}
	
	virtual inline int32_t size() const {
		return m_keys.ref().JRefStack<K>::size();
	}
	
	virtual inline const Collection<V>& values() const {
		return *((Collection<V>*)&m_values);
	}
	
	virtual inline JString toJSON() const {
		JString json = L"{";
		Iterator<K> itKey = THIS.keySet().iterator();
		Iterator<V> itVal = THIS.values().iterator();
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
class Map : public JObject {
public:
	JRefTemplate2(Map, Map, RefMap)
	
	inline Map() {
		THIS.initialize();
	}
	
	inline Map(uint32_t initialCapacity) {
		THIS.initialize();
		RefMap<K,V>* o = new RefMap<K,V>(initialCapacity);
		if (o == NULL) throw eOutOfMemory;
		THIS.setRef(o);
	}
	
	virtual inline void clear() throw(const char*) { THIS.ref().clear(); }
	virtual inline bool containsKey(const K& key) const throw(const char*) { return THIS.ref().containsKey(key); }
	virtual inline bool containsValue(const V& value) const throw(const char*) { return THIS.ref().containsValue(value); }
	// entrySet
	virtual inline const V& get(const K& key) const throw(const char*) { return THIS.ref().get(key); }
	virtual inline const V& opt(const K& key, const V& defaultValue) const throw(const char*) { return THIS.ref().opt(key, defaultValue); }
	virtual inline bool isEmpty() const throw(const char*) { return THIS.ref().isEmpty(); }
	virtual inline const Set<K>& keySet() const throw(const char*) { return THIS.ref().keySet(); }
	virtual inline V& put(const K& key, const V& value) throw(const char*) { return THIS.ref().put(key, value); }
	// putAll
	virtual inline void remove(const K& key) throw(const char*) { THIS.ref().remove(key); }
	virtual inline int32_t size() const throw(const char*) { return THIS.ref().size(); }
	virtual inline const Collection<V>& values() const throw(const char*) { return THIS.ref().values(); }
	
	static SynchronizedMap<K,V> synchronizedMap(Map<K,V>* newMap) {
		return SynchronizedMap<K,V>(newMap);
	}
};

template <typename K, typename V>
class SynchronizedMap : public Map<K,V> {
public:
	JRefTemplate2(SynchronizedMap, Map, RefMap)
	
	virtual inline void clear() throw(const char*) {
		synchronized(*this) {
			try {
				THIS.ref().clear();
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
	}
	
	virtual inline bool containsKey(const K& key) const throw(const char*) {
		bool result;
		synchronized(*this) {
			result = THIS.ref().containsKey(key);
			THIS.notifyAll();
		}
		return result;
	}
	
	virtual inline bool containsValue(const V& value) const throw(const char*) {
		bool result;
		synchronized(*this) {
			result = THIS.ref().containsValue(value);
			THIS.notifyAll();
		}
		return result;
	}
	
	virtual inline const V& get(const K& key) const throw(const char*) {
		const V* result;
		synchronized(*this) {
			try {
				result = &(THIS.ref().get(key));
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
		return *result;
	}
	
	virtual inline const V& opt(const K& key, const V& defaultValue) const throw(const char*) {
		const V* result;
		synchronized(*this) {
			try {
				result = &(THIS.ref().opt(key, defaultValue));
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
		return *result;
	}
	
	virtual inline bool isEmpty() const throw(const char*) {
		bool result;
		synchronized(*this) {
			result = THIS.ref().isEmpty();
			THIS.notifyAll();
		}
		return result;
	}
	
	virtual inline const Set<K>& keySet() const throw(const char*) {
		const Set<K>* result;
		synchronized(*this) {
			result = &(THIS.ref().keySet());
			THIS.notifyAll();
		}
		return *result;
	}
	
	virtual inline V& put(const K& key, const V& value) throw(const char*) {
		V* result;
		synchronized(*this) {
			try {
				result = &(THIS.ref().put(key, value));
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
		return *result;
	}
	
	virtual inline void remove(const K& key) throw(const char*) {
		synchronized(*this) {
			try {
				THIS.ref().remove(key);
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
	}
	
	virtual inline int32_t size() const throw(const char*) {
		bool result;
		synchronized(*this) {
			result = THIS.ref().size();
			THIS.notifyAll();
		}
		return result;
	}
	
	virtual inline const Collection<V>& values() const throw(const char*) {
		const Collection<V>* result;
		synchronized(*this) {
			try {
				result = &(THIS.ref().values());
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
		return *result;
	}
};

#endif //JAPPSY_UMAP_H