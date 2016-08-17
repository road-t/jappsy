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

#ifndef JAPPSY_ULINKEDHASHMAP_H
#define JAPPSY_ULINKEDHASHMAP_H

#include <data/uMap.h>

template <typename K, typename V>
class RefLinkedHashMap : public RefMap<K,V> {
protected:
	class LinkedHashMapItem : public RefObject {
	public:
		RefMap<K,LinkedHashMapItem>* m_parent = NULL;
		K* m_key = NULL;
		
		LinkedHashMapItem* m_prevItem = NULL;
		LinkedHashMapItem* m_nextItem = NULL;
		
		V m_value;
		
		inline LinkedHashMapItem() {
		}
		
		inline LinkedHashMapItem(const void* value) {
			m_value = V(value);
		}
		
		inline LinkedHashMapItem(const V& value) {
			m_value = value;
		}
		
		inline bool operator==(const V& value) const {
			return m_value == value;
		}
		
		inline bool operator!=(const V& value) const {
			return m_value != value;
		}
		
		inline bool operator==(const LinkedHashMapItem& value) const {
			return m_value == value.m_value;
		}
		
		inline bool operator!=(const LinkedHashMapItem& value) const {
			return m_value != value.m_value;
		}
	};
	
protected:
	typedef Map<K,LinkedHashMapItem>* MapPtr;
	
	Map<K,LinkedHashMapItem>*** m_map = NULL;
	
	LinkedHashMapItem* m_first = NULL;
	LinkedHashMapItem* m_last = NULL;
	
	uint32_t m_count = 0;
	
	inline RefMap<K,LinkedHashMapItem>* createMap(const K& key) throw(const char*) {
		uint32_t hash = Object::hashCode(key);
		if (m_map == NULL) {
			m_map = memAlloc(MapPtr*, m_map, 256 * sizeof(MapPtr*));
			if (m_map == NULL) throw eOutOfMemory;
		}
		
		Map<K,LinkedHashMapItem> **m1 = m_map[hash & 0xFF];
		if (m1 == NULL) {
			m1 = memAlloc(MapPtr, m1, 256 * sizeof(MapPtr));
			if (m1 == NULL) throw eOutOfMemory;
			m_map[hash & 0xFF] = m1;
		}

		hash >>= 8;
		Map<K,LinkedHashMapItem> *m2 = m1[hash & 0xFF];
		if (m2 == NULL) {
			RefMap<K,LinkedHashMapItem> *m3 = NULL;
			try {
				m2 = new Map<K,LinkedHashMapItem>();
				if (m2 == NULL) throw eOutOfMemory;
				m3 = new RefMap<K,LinkedHashMapItem>();
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

		return (RefMap<K,LinkedHashMapItem>*)(m2->_object);
	}
	
	inline RefMap<K,LinkedHashMapItem>* findMap(const K& key) const {
		if (m_map != NULL) {
			uint32_t hash = Object::hashCode(key);
			Map<K,LinkedHashMapItem> **m1 = m_map[hash & 0xFF];
			if (m1 != NULL) {
				hash >>= 8;
				Map<K,LinkedHashMapItem> *m2 = m1[hash & 0xFF];
				return (RefMap<K,LinkedHashMapItem>*)(m2->_object);
			}
		}
		return NULL;
	}
	
	inline bool containsKey(RefMap<K,LinkedHashMapItem> *map, const K& key) const {
		if (map != NULL) {
			return map->containsKey(key);
		}
		return false;
	}
	
	inline bool containsValue(RefMap<K,LinkedHashMapItem> *map, const V& value) const {
		if (map != NULL) {
			RefList<LinkedHashMapItem> *list = (RefList<LinkedHashMapItem>*)(map->m_values._object);
			if (list->m_count > 0) {
				for (int i = list->m_count-1; i >= 0; i--) {
					if (list->m_stack[i] != NULL) {
						if (*(list->m_stack[i]) == value) return true;
					} else if (Object::isNull(value)) {
						return true;
					}
				}
			}
		}
		return false;
	}
	
	inline void removeKey(RefSet<K> *set, int32_t index) {
		set->m_count--;
		K* key = set->m_stack[index];
		if ((set->m_count > 0) && (index < set->m_count)) {
			memmove(set->m_stack + index, set->m_stack + index + 1, (set->m_count - index) * sizeof(K*));
		}
		set->m_stack[set->m_count] = NULL;
		delete key;
	}
	
	inline LinkedHashMapItem* releaseValue(RefList<LinkedHashMapItem> *list, int32_t index) {
		list->m_count--;
		LinkedHashMapItem* object = list->m_stack[index];
		if ((list->m_count > 0) && (index < list->m_count)) {
			memmove(list->m_stack + index, list->m_stack + index + 1, (list->m_count - index) * sizeof(LinkedHashMapItem*));
		}
		list->m_stack[list->m_count] = NULL;
		return object;
	}

	Set<K> m_tempSet = new Set<K>();
	Collection<V> m_tempValues = new Collection<V>();
	
public:
	
	inline RefLinkedHashMap() { THIS.TYPE = TypeLinkedHashMap; }
	
	inline ~RefLinkedHashMap() {
		clear();
	}
	
	virtual inline void clear() throw(const char*) {
		if (m_map != NULL) {
			for (int i = 0; i < 256; i++) {
				if (m_map[i] != NULL) {
					Map<K,LinkedHashMapItem> **m1 = m_map[i];
					if (m1 != NULL) {
						for (int j = 0; j < 256; j++) {
							if (m1[j] != NULL) {
								Map<K,LinkedHashMapItem> *m2 = m1[j];
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
		m_first = NULL;
		m_last = NULL;
		m_count = 0;
	}
	
	virtual inline bool containsKey(const K& key) const {
		RefMap<K,LinkedHashMapItem> *map = findMap(key);
		return containsKey(map, key);
	}
	
	virtual inline bool containsValue(const V& value) const {
		if (m_map != NULL) {
			for (int i = 0; i < 256; i++) {
				if (m_map[i] != NULL) {
					Map<K,LinkedHashMapItem> **m1 = m_map[i];
					for (int j = 0; j < 256; j++) {
						if (m1[j] != NULL) {
							Map<K,LinkedHashMapItem> *m2 = m1[j];
							if (m2 != NULL) {
								if (containsValue((RefMap<K,LinkedHashMapItem>*)(m2->_object), value)) return true;
							}
						}
					}
				}
			}
		}
		return false;
	}
	
	virtual inline const V& get(const K& key) const throw(const char*) {
		RefMap<K,LinkedHashMapItem> *map = findMap(key);
		if (map != NULL) {
			int32_t index = map->m_keys->search(key);
			if (index >= 0) {
				RefList<LinkedHashMapItem> *list = (RefList<LinkedHashMapItem>*)(map->m_values._object);
				LinkedHashMapItem* item = list->m_stack[index];
				if (item != NULL) {
					return item->m_value;
				}
			}
		}
		throw eNotFound;
	}
	
	virtual inline const V& opt(const K& key, const V& defaultValue) const throw(const char*) {
		RefMap<K,LinkedHashMapItem> *map = findMap(key);
		if (map != NULL) {
			int32_t index = map->m_keys->search(key);
			if (index >= 0) {
				RefList<LinkedHashMapItem> *list = (RefList<LinkedHashMapItem>*)(map->m_values._object);
				LinkedHashMapItem* item = list->m_stack[index];
				if (item != NULL) {
					return item->m_value;
				}
			}
		}
		return defaultValue;
	}
	
	virtual inline bool isEmpty() const {
		return (m_count == 0);
	}
	
	virtual inline const Set<K>& keySet() const throw(const char*) {
		m_tempSet->clear();
		LinkedHashMapItem* item = m_first;
		while (item != NULL) {
			m_tempSet->push(*(item->m_key));
			item = item->m_nextItem;
		}
		return m_tempSet;
	}
	
	virtual inline V& put(const K& key, const V& value) throw(const char*) {
		RefMap<K,LinkedHashMapItem> *map = createMap(key);

		RefSet<K> *set = (RefSet<K>*)(map->m_keys._object);
		RefList<LinkedHashMapItem> *list = (RefList<LinkedHashMapItem>*)(map->m_values._object);

		K* itemKey;
		LinkedHashMapItem* item;
		int32_t index = set->search(key);
		if (index < 0) {
			itemKey = &(set->push(key));
			item = &(list->push(value));
			item->m_parent = map;
			item->m_key = itemKey;
			item->m_prevItem = m_last;
			if (m_last != NULL) {
				m_last->m_nextItem = item;
			}
			if (m_first == NULL) {
				m_first = item;
			}
			m_last = item;
			m_count++;
		} else {
			itemKey = set->m_stack[index];
			item = list->m_stack[index];
			item->m_value = value;
		}
		return item->m_value;
	}
	
	virtual inline void remove(const K& key) throw(const char*) {
		RefMap<K,LinkedHashMapItem> *map = findMap(key);
		if (map != NULL) {
			RefSet<K> *set = (RefSet<K>*)(map->m_keys._object);
			RefList<LinkedHashMapItem> *list = (RefList<LinkedHashMapItem>*)(map->m_values._object);
			if ((set != NULL) && (list != NULL)) {
				int32_t index = set->search(key);
				if (index >= 0) {
					removeKey(set, index);
					LinkedHashMapItem* item = releaseValue(list, index);
					if (item != NULL) {
						if (item->m_prevItem != NULL) {
							item->m_prevItem->m_nextItem = item->m_nextItem;
						}
						if (item->m_nextItem != NULL) {
							item->m_nextItem->m_prevItem = item->m_prevItem;
						}
						if (m_first == item) {
							m_first = item->m_nextItem;
						}
						if (m_last == item) {
							m_last = item->m_prevItem;
						}
						delete item;
						m_count--;
					}
				}
			}
		}
	}
	
	virtual inline int32_t size() const {
		return m_count;
	}
	
	virtual inline const Collection<V>& values() const throw(const char*) {
		m_tempValues->clear();
		LinkedHashMapItem* item = m_first;
		while (item != NULL) {
			m_tempValues->push(item->m_value);
			item = item->m_nextItem;
		}
		return m_tempValues;
	}
};

template <typename K, typename V>
class LinkedHashMap : public Map<K,V> {
public:
	RefTemplate2(LinkedHashMap, LinkedHashMap, RefLinkedHashMap)
	
	inline LinkedHashMap() {
		THIS.initialize();
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
};

#endif //JAPPSY_ULINKEDHASHMAP_H