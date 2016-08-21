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

#ifndef JAPPSY_ULINKEDHASHSET_H
#define JAPPSY_ULINKEDHASHSET_H

#include <data/uSet.h>

template <typename Type>
class RefLinkedHashSet : public RefSet<Type> {
protected:
	class LinkedHashSetItem : public JRefObject {
	public:
		RefSet<LinkedHashSetItem>* m_parent = NULL;
		
		LinkedHashSetItem* m_prevItem = NULL;
		LinkedHashSetItem* m_nextItem = NULL;
		Type m_value;
		
		inline LinkedHashSetItem() {
		}
		
		inline LinkedHashSetItem(const void* value) {
			m_value = value;
		}
		
		inline LinkedHashSetItem(const Type& value) {
			m_value = value;
		}
		
		inline bool operator==(const Type& value) const {
			return m_value == value;
		}
		
		inline bool operator!=(const Type& value) const {
			return m_value != value;
		}
		
		inline bool operator==(const LinkedHashSetItem& value) const {
			return m_value == value.m_value;
		}
		
		inline bool operator!=(const LinkedHashSetItem& value) const {
			return m_value != value.m_value;
		}
	};
	
protected:
	Set<LinkedHashSetItem>*** m_set = NULL;
	
	LinkedHashSetItem* m_first = NULL;
	LinkedHashSetItem* m_last = NULL;
	
	uint32_t m_count = 0;
	
	inline RefSet<LinkedHashSetItem>* createSet(const Type& object) throw(const char*) {
		uint32_t hash = JObject::hashCode(object);
		if (m_set == NULL) {
			m_set = memAlloc(Set<LinkedHashSetItem> **, m_set, 256 * sizeof(Set<LinkedHashSetItem> **));
			if (m_set == NULL) throw eOutOfMemory;
		}

		Set<LinkedHashSetItem> **s1 = m_set[hash & 0xFF];
		if (s1 == NULL) {
			s1 = memAlloc(Set<LinkedHashSetItem> *, s1, 256 * sizeof(Set<LinkedHashSetItem> *));
			if (s1 == NULL) throw eOutOfMemory;
			m_set[hash & 0xFF] = s1;
		}

		hash >>= 8;
		Set<LinkedHashSetItem> *s2 = s1[hash & 0xFF];
		if (s2 == NULL) {
			RefSet<LinkedHashSetItem> *s3 = NULL;
			try {
				s2 = new Set<LinkedHashSetItem>();
				if (s2 == NULL) throw eOutOfMemory;
				s3 = new RefSet<LinkedHashSetItem>();
				if (s3 == NULL) throw eOutOfMemory;
			} catch (...) {
				if (s2 != NULL) {
					delete s2;
				}
				throw;
			}
			s2->setRef(s3);
			s1[hash & 0xFF] = s2;
		}

		return (RefSet<LinkedHashSetItem>*)(s2->_object);
	}
	
	inline RefSet<LinkedHashSetItem>* findSet(const Type& object) const {
		if (m_set != NULL) {
			uint32_t hash = JObject::hashCode(object);
			Set<LinkedHashSetItem> **s1 = m_set[hash & 0xFF];
			if (s1 != NULL) {
				hash >>= 8;
				Set<LinkedHashSetItem> *s2 = s1[hash & 0xFF];
				return (RefSet<LinkedHashSetItem>*)(s2->_object);
			}
		}
		return NULL;
	}

	inline bool contains(RefSet<LinkedHashSetItem> *set, const Type& object) const {
		if (set != NULL) {
			if (set->m_count > 0) {
				for (int i = set->m_count-1; i >= 0; i--) {
					if (set->m_stack[i] != NULL) {
						if (*(set->m_stack[i]) == object) return true;
					} else if (object == null) {
						return true;
					}
				}
			}
		}
		return false;
	}
	
	inline int32_t search(RefSet<LinkedHashSetItem> *set, const Type& value) const {
		if (set != NULL) {
			if (set->m_count > 0) {
				for (int i = set->m_count-1; i >= 0; i--) {
					if (set->m_stack[i] != NULL) {
						if (*(set->m_stack[i]) == value) return i;
					} else if (value == null) {
						return i;
					}
				}
			}
		}
		return -1;
	}
	
	inline LinkedHashSetItem* release(RefSet<LinkedHashSetItem> *set, const Type& value) {
		if (set != NULL) {
			int32_t index = search(set, value);
			if (index >= 0) {
				set->m_count--;
				LinkedHashSetItem* object = set->m_stack[index];
				if ((set->m_count > 0) && (index < set->m_count)) {
					memmove(set->m_stack + index, set->m_stack + index + 1, (set->m_count - index) * sizeof(LinkedHashSetItem*));
				}
				set->m_stack[set->m_count] = NULL;
				return object;
			}
		}
		return NULL;
	}
	
	Iterator<Type> m_tempIt = new Iterator<Type>();
	
public:
	
	inline RefLinkedHashSet() { THIS.TYPE = TypeLinkedHashSet; }
	
	inline ~RefLinkedHashSet() {
		clear();
	}
	
	virtual inline void clear() throw(const char*) {
		if (m_set != NULL) {
			for (int i = 0; i < 256; i++) {
				if (m_set[i] != NULL) {
					Set<LinkedHashSetItem> **s1 = m_set[i];
					if (s1 != NULL) {
						for (int j = 0; j < 256; j++) {
							if (s1[j] != NULL) {
								Set<LinkedHashSetItem> *s2 = s1[j];
								if (s2 != NULL) {
									delete s2;
									s1[j] = NULL;
								}
							}
						}
						memFree(s1);
					}
				}
			}
			memFree(m_set);
			m_set = NULL;
		}
		m_first = NULL;
		m_last = NULL;
		m_count = 0;
	}
	
	virtual inline bool add(const Type& object) throw(const char*) {
		RefSet<LinkedHashSetItem> *set = createSet(object);
		if (set != NULL) {
			if (!contains(set, object)) {
				LinkedHashSetItem* item = &(set->push(object));
				item->m_parent = set;
				item->m_prevItem = m_last;
				
				if (m_last != NULL)
					m_last->m_nextItem = item;
				if (m_first == NULL)
					m_first = item;
					
				m_last = item;
				m_count++;
				return true;
			}
		}
		return false;
	}
	
	virtual inline bool addAll(Collection<Type>& collection) throw(const char*) {
		bool result = false;
		Iterator<Type> it = collection->iterator();
		while (it->hasNext()) {
			result |= add(it->next());
		}
		return result;
	}
	
	virtual inline bool contains(const Type& object) const {
		RefSet<LinkedHashSetItem> *set = findSet(object);
		return contains(set, object);
	}
	
	virtual inline bool containsAll(Collection<Type>& collection) throw(const char*) {
		Iterator<Type> it = collection->iterator();
		while (it->hasNext()) {
			if (!contains(it->next())) {
				return false;
			}
		}
		return true;
	}
	
	virtual inline bool isEmpty() const {
		return (m_count == 0);
	}
	
	virtual inline const Iterator<Type> iterator() const throw(const char*) {
		m_tempIt->clear();
		LinkedHashSetItem* item = m_first;
		while (item != NULL) {
			m_tempIt->push(item->m_value);
			item = item->m_nextItem;
		}
		m_tempIt->reset(0);
		return m_tempIt;
	}
	
	virtual inline bool remove(const Type& object) throw(const char*) {
		RefSet<LinkedHashSetItem> *set = findSet(object);
		if (set != NULL) {
			LinkedHashSetItem* item = release(set, object);
			if (item != NULL) {
				if (item->m_prevItem != NULL)
					item->m_prevItem->m_nextItem = item->m_nextItem;
				if (item->m_nextItem != NULL)
					item->m_nextItem->m_prevItem = item->m_prevItem;
				if (m_first == item)
					m_first = item->m_nextItem;
				if (m_last == item)
					m_last = item->m_prevItem;

				delete item;
				m_count--;
				return true;
			}
		}
		return false;
	}
	
	virtual inline bool removeAll(Collection<Type>& collection) throw(const char*) {
		bool result = false;
		Iterator<Type> it = collection->iterator();
		while (it->hasNext()) {
			result |= remove(it->next());
		}
		return result;
	}
	
	virtual inline bool retainAll(Collection<Type>& collection) throw(const char*) {
		bool result = false;
		
		LinkedHashSetItem* it = m_first;
		while (it != NULL) {
			if (!collection->contains(it->m_value)) {
				RefSet<LinkedHashSetItem> *set = it->m_parent;
				LinkedHashSetItem* item = release(set, it->m_value);
				if (item != NULL) {
					if (item->m_prevItem != NULL)
						item->m_prevItem->m_nextItem = item->m_nextItem;
					if (item->m_nextItem != NULL)
						item->m_nextItem->m_prevItem = item->m_prevItem;
					if (m_first == item)
						m_first = item->m_nextItem;
					if (m_last == item)
						m_last = item->m_prevItem;
					delete item;
					m_count--;
					result = true;
				}
			}
			it = it->m_nextItem;
		}
		
		return result;
	}
	
	virtual inline int32_t size() const {
		return m_count;
	}
};

template <typename Type>
class LinkedHashSet : public Set<Type> {
public:
	JRefTemplate(LinkedHashSet, LinkedHashSet, RefLinkedHashSet)
	
	inline LinkedHashSet() {
		THIS.initialize();
	}
	
	virtual inline bool add(const Type& object) throw(const char*) { return THIS.ref().add(object); }
	virtual inline bool addAll(Collection<Type>& collection) throw(const char*) { return THIS.ref().addAll(collection); }
	virtual inline void clear() throw(const char*) { THIS.ref().clear(); }
	virtual inline bool contains(const Type& value) const throw(const char*) { return THIS.ref().contains(value); }
	virtual inline bool containsAll(Collection<Type>& collection) throw(const char*) { return THIS.ref().containsAll(collection); }
	virtual inline bool isEmpty() const throw(const char*) { return THIS.ref().isEmpty(); }
	virtual inline const Iterator<Type> iterator() const throw(const char*) { return THIS.ref().iterator(); }
	virtual inline bool remove(const Type& value) throw(const char*) { return THIS.ref().remove(value); }
	virtual inline bool removeAll(Collection<Type>& collection) throw(const char*) { return THIS.ref().removeAll(collection); }
	virtual inline bool retainAll(Collection<Type>& collection) throw(const char*) { return THIS.ref().retainAll(collection); }
	virtual inline int32_t size() const throw(const char*) { return THIS.ref().size(); }
};

#endif //JAPPSY_ULINKEDHASHSET_H

