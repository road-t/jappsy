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

#ifndef JAPPSY_UHASHMAP_H
#define JAPPSY_UHASHMAP_H

#include <data/uMap.h>

template <typename K, typename V>
class JRefHashMap : public JRefMap<K,V> {
protected:
	typedef JMap<K,V>* MapPtr;
	JMap<K,V>*** m_map = NULL;
	uint32_t m_count = 0;
	
	inline JRefMap<K,V>* createMap(const K& key) throw(const char*) {
		uint32_t hash = JObject::hashCode(key);
		if (m_map == NULL) {
			m_map = memAlloc(MapPtr*, m_map, 256 * sizeof(MapPtr*));
			if (m_map == NULL) throw eOutOfMemory;
		}

		JMap<K,V> **m1 = m_map[hash & 0xFF];
		if (m1 == NULL) {
			m1 = memAlloc(MapPtr, m1, 256 * sizeof(MapPtr));
			if (m_map == NULL) throw eOutOfMemory;
			m_map[hash & 0xFF] = m1;
		}

		hash >>= 8;
		JMap<K,V> *m2 = m1[hash & 0xFF];
		if (m2 == NULL) {
			JRefMap<K,V> *m3 = NULL;
			try {
				m2 = new JMap<K,V>();
				if (m2 == NULL) throw eOutOfMemory;
				m3 = new JRefMap<K,V>();
				if (m3 == NULL) throw eOutOfMemory;
			} catch (...) {
				if (m2 != NULL) {
					delete m2;
				}
				throw;
			}
			m2->setRef(m3);
			m1[hash & 0xFF] = m2;
		}

		return (JRefMap<K,V>*)(m2->_object);
	}
	
	inline JRefMap<K,V>* findMap(const K& key) const {
		if (m_map != NULL) {
			uint32_t hash = JObject::hashCode(key);
			JMap<K,V> **m1 = m_map[hash & 0xFF];
			if (m1 != NULL) {
				hash >>= 8;
				JMap<K,V> *m2 = m1[hash & 0xFF];
				if (m2 != NULL)
					return (JRefMap<K,V>*)(m2->_object);
			}
		}
		return NULL;
	}
	
	JSet<K> m_tempSet = new JSet<K>();
	JCollection<V> m_tempValues = new JCollection<V>();
	
public:
	
	inline JRefHashMap() { THIS.TYPE = TypeHashMap; }
	
	inline ~JRefHashMap() {
		clear();
	}
	
	virtual inline void clear() throw(const char*) {
		if (m_map != NULL) {
			for (int i = 0; i < 256; i++) {
				if (m_map[i] != NULL) {
					JMap<K,V> **m1 = m_map[i];
					if (m1 != NULL) {
						for (int j = 0; j < 256; j++) {
							if (m1[j] != NULL) {
								JMap<K,V> *m2 = m1[j];
								if (m2 != NULL) {
									delete m2;
									m1[j] = NULL;
								}
							}
						}
						memFree(m1);
					}
				}
			}
			memFree(m_map);
			m_map = NULL;
		}
		m_count = 0;
	}
	
	virtual inline bool containsKey(const K& key) const {
		JRefMap<K,V> *map = findMap(key);
		if (map != NULL) {
			return map->containsKey(key);
		}
		return false;
	}
	
	virtual inline bool containsValue(const V& value) const {
		if (m_map != NULL) {
			for (int i = 0; i < 256; i++) {
				if (m_map[i] != NULL) {
					JMap<K,V> **m1 = m_map[i];
					for (int j = 0; j < 256; j++) {
						if (m1[j] != NULL) {
							JMap<K,V> *m2 = m1[j];
							if (m2 != NULL) {
								if (m2->containsValue(value)) return true;
							}
						}
					}
				}
			}
		}
		return false;
	}
	
	virtual inline const V& get(const K& key) const throw(const char*) {
		JRefMap<K,V> *map = findMap(key);
		if (map != NULL)
			return map->get(key);
		throw eNotFound;
	}
	
	virtual inline const V& opt(const K& key, const V& defaultValue) const {
		JRefMap<K,V> *map = findMap(key);
		if (map != NULL)
			return map->opt(key, defaultValue);
		return defaultValue;
	}
	
	virtual inline bool isEmpty() const {
		return (m_count == 0);
	}
	
	
	virtual inline const JSet<K>& keySet() const throw(const char*) {
		m_tempSet.ref().clear();
		if (m_map != NULL) {
			for (int i = 0; i < 256; i++) {
				if (m_map[i] != NULL) {
					JMap<K,V> **m1 = m_map[i];
					for (int j = 0; j < 256; j++) {
						if (m1[j] != NULL) {
							JMap<K,V> *m2 = m1[j];
							if (m2 != NULL) {
								JSet<K> m2set = m2->keySet();
								JIterator<K> m2it = m2set.iterator();
								while (m2it.hasNext()) {
									m_tempSet.ref().push(m2it.next());
								}
							}
						}
					}
				}
			}
		}
		return m_tempSet;
	}
	
	virtual inline V& put(const K& key, const V& value) throw(const char*) {
		JRefMap<K,V> *map = createMap(key);
		int prevSize = map->size();
		try {
			V* result = &(map->put(key, value));
			int newSize = map->size();
			m_count += newSize - prevSize;
			return *result;
		} catch (...) {
			int newSize = map->size();
			m_count += newSize - prevSize;
			throw;
		}
	}
	
	virtual inline void remove(const K& key) throw(const char*) {
		JRefMap<K,V> *map = findMap(key);
		if (map != NULL) {
			int prevSize = map->size();
			try {
				map->remove(key);
				int newSize = map->size();
				m_count += newSize - prevSize;
			} catch (...) {
				int newSize = map->size();
				m_count += newSize - prevSize;
				throw;
			}
		}
	}
	
	virtual inline int32_t size() const {
		return m_count;
	}
	
	virtual inline const JCollection<V>& values() const throw(const char*) {
		m_tempValues.ref().clear();
		if (m_map != NULL) {
			for (int i = 0; i < 256; i++) {
				if (m_map[i] != NULL) {
					JMap<K,V> **m1 = m_map[i];
					for (int j = 0; j < 256; j++) {
						if (m1[j] != NULL) {
							JMap<K,V> *m2 = m1[j];
							if (m2 != NULL) {
								JCollection<V> m2col = m2->values();
								JIterator<V> m2it = m2col.iterator();
								while (m2it.hasNext()) {
									m_tempValues.ref().push(m2it.next());
								}
							}
						}
					}
				}
			}
		}
		return m_tempValues;
	}
};

template <typename K, typename V>
class JHashMap : public JMap<K,V> {
public:
	JRefTemplate2(JHashMap, JHashMap, JRefHashMap)
	
	inline JHashMap() {
		THIS.initialize();
	}
	
	virtual inline void clear() throw(const char*) { THIS.ref().clear(); }
	virtual inline bool containsKey(const K& key) const throw(const char*) { return THIS.ref().containsKey(key); }
	virtual inline bool containsValue(const V& value) const throw(const char*) { return THIS.ref().containsValue(value); }
	// entrySet
	virtual inline const V& get(const K& key) const throw(const char*) { return THIS.ref().get(key); }
	virtual inline const V& opt(const K& key, const V& defaultValue) const throw(const char*) { return THIS.ref().opt(key, defaultValue); }
	virtual inline bool isEmpty() const throw(const char*) { return THIS.ref().isEmpty(); }
	virtual inline const JSet<K>& keySet() const throw(const char*) { return THIS.ref().keySet(); }
	virtual inline V& put(const K& key, const V& value) throw(const char*) { return THIS.ref().put(key, value); }
	// putAll
	virtual inline void remove(const K& key) throw(const char*) { THIS.ref().remove(key); }
	virtual inline int32_t size() const throw(const char*) { return THIS.ref().size(); }
	virtual inline const JCollection<V>& values() const throw(const char*) { return THIS.ref().values(); }
};

#endif //JAPPSY_UHASHMAP_H