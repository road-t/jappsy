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
class JRefLinkedHashSet : public JRefSet<Type> {
protected:
	class JLinkedHashSetItem : public JRefObject {
	public:
		JRefSet<JLinkedHashSetItem>* m_parent = NULL;
		
		JLinkedHashSetItem* m_prevItem = NULL;
		JLinkedHashSetItem* m_nextItem = NULL;
		Type m_value;
		
		inline JLinkedHashSetItem() {
		}
		
		inline JLinkedHashSetItem(const void* value) {
			m_value = value;
		}
		
		inline JLinkedHashSetItem(const Type& value) {
			m_value = value;
		}
		
		inline bool operator==(const Type& value) const {
			return m_value == value;
		}
		
		inline bool operator!=(const Type& value) const {
			return m_value != value;
		}
		
		inline bool operator==(const JLinkedHashSetItem& value) const {
			return m_value == value.m_value;
		}
		
		inline bool operator!=(const JLinkedHashSetItem& value) const {
			return m_value != value.m_value;
		}
	};
	
protected:
	JSet<JLinkedHashSetItem>*** m_set = NULL;
	
	JLinkedHashSetItem* m_first = NULL;
	JLinkedHashSetItem* m_last = NULL;
	
	uint32_t m_count = 0;
	
	inline JRefSet<JLinkedHashSetItem>* createSet(const Type& object) throw(const char*) {
		uint32_t hash = JObject::hashCode(object);
		if (m_set == NULL) {
			m_set = memAlloc(JSet<JLinkedHashSetItem> **, m_set, 256 * sizeof(JSet<JLinkedHashSetItem> **));
			if (m_set == NULL) throw eOutOfMemory;
		}

		JSet<JLinkedHashSetItem> **s1 = m_set[hash & 0xFF];
		if (s1 == NULL) {
			s1 = memAlloc(JSet<JLinkedHashSetItem> *, s1, 256 * sizeof(JSet<JLinkedHashSetItem> *));
			if (s1 == NULL) throw eOutOfMemory;
			m_set[hash & 0xFF] = s1;
		}

		hash >>= 8;
		JSet<JLinkedHashSetItem> *s2 = s1[hash & 0xFF];
		if (s2 == NULL) {
			JRefSet<JLinkedHashSetItem> *s3 = NULL;
			try {
				s2 = new JSet<JLinkedHashSetItem>();
				if (s2 == NULL) throw eOutOfMemory;
				s3 = new JRefSet<JLinkedHashSetItem>();
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

		return (JRefSet<JLinkedHashSetItem>*)(s2->_object);
	}
	
	inline JRefSet<JLinkedHashSetItem>* findSet(const Type& object) const {
		if (m_set != NULL) {
			uint32_t hash = JObject::hashCode(object);
			JSet<JLinkedHashSetItem> **s1 = m_set[hash & 0xFF];
			if (s1 != NULL) {
				hash >>= 8;
				JSet<JLinkedHashSetItem> *s2 = s1[hash & 0xFF];
				return (JRefSet<JLinkedHashSetItem>*)(s2->_object);
			}
		}
		return NULL;
	}

	inline bool contains(JRefSet<JLinkedHashSetItem> *set, const Type& object) const {
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
	
	inline int32_t search(JRefSet<JLinkedHashSetItem> *set, const Type& value) const {
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
	
	inline JLinkedHashSetItem* release(JRefSet<JLinkedHashSetItem> *set, const Type& value) {
		if (set != NULL) {
			int32_t index = search(set, value);
			if (index >= 0) {
				set->m_count--;
				JLinkedHashSetItem* object = set->m_stack[index];
				if ((set->m_count > 0) && (index < set->m_count)) {
					memmove(set->m_stack + index, set->m_stack + index + 1, (set->m_count - index) * sizeof(JLinkedHashSetItem*));
				}
				set->m_stack[set->m_count] = NULL;
				return object;
			}
		}
		return NULL;
	}
	
	JIterator<Type> m_tempIt = new JIterator<Type>();
	
public:
	
	inline JRefLinkedHashSet() { THIS.TYPE = TypeLinkedHashSet; }
	
	inline ~JRefLinkedHashSet() {
		clear();
	}
	
	virtual inline void clear() throw(const char*) {
		if (m_set != NULL) {
			for (int i = 0; i < 256; i++) {
				if (m_set[i] != NULL) {
					JSet<JLinkedHashSetItem> **s1 = m_set[i];
					if (s1 != NULL) {
						for (int j = 0; j < 256; j++) {
							if (s1[j] != NULL) {
								JSet<JLinkedHashSetItem> *s2 = s1[j];
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
		JRefSet<JLinkedHashSetItem> *set = createSet(object);
		if (set != NULL) {
			if (!contains(set, object)) {
				JLinkedHashSetItem* item = &(set->push(object));
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
	
	virtual inline bool addAll(JCollection<Type>& collection) throw(const char*) {
		bool result = false;
		JIterator<Type> it = collection->iterator();
		while (it->hasNext()) {
			result |= add(it->next());
		}
		return result;
	}
	
	virtual inline bool contains(const Type& object) const {
		JRefSet<JLinkedHashSetItem> *set = findSet(object);
		return contains(set, object);
	}
	
	virtual inline bool containsAll(JCollection<Type>& collection) throw(const char*) {
		JIterator<Type> it = collection->iterator();
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
	
	virtual inline const JIterator<Type> iterator() const throw(const char*) {
		m_tempIt->clear();
		JLinkedHashSetItem* item = m_first;
		while (item != NULL) {
			m_tempIt->push(item->m_value);
			item = item->m_nextItem;
		}
		m_tempIt->reset(0);
		return m_tempIt;
	}
	
	virtual inline bool remove(const Type& object) throw(const char*) {
		JRefSet<JLinkedHashSetItem> *set = findSet(object);
		if (set != NULL) {
			JLinkedHashSetItem* item = release(set, object);
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
	
	virtual inline bool removeAll(JCollection<Type>& collection) throw(const char*) {
		bool result = false;
		JIterator<Type> it = collection->iterator();
		while (it->hasNext()) {
			result |= remove(it->next());
		}
		return result;
	}
	
	virtual inline bool retainAll(JCollection<Type>& collection) throw(const char*) {
		bool result = false;
		
		JLinkedHashSetItem* it = m_first;
		while (it != NULL) {
			if (!collection->contains(it->m_value)) {
				JRefSet<JLinkedHashSetItem> *set = it->m_parent;
				JLinkedHashSetItem* item = release(set, it->m_value);
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
class JLinkedHashSet : public JSet<Type> {
public:
	JRefTemplate(JLinkedHashSet, JLinkedHashSet, JRefLinkedHashSet)
	
	inline JLinkedHashSet() {
		THIS.initialize();
	}
	
	virtual inline bool add(const Type& object) throw(const char*) { return THIS.ref().add(object); }
	virtual inline bool addAll(JCollection<Type>& collection) throw(const char*) { return THIS.ref().addAll(collection); }
	virtual inline void clear() throw(const char*) { THIS.ref().clear(); }
	virtual inline bool contains(const Type& value) const throw(const char*) { return THIS.ref().contains(value); }
	virtual inline bool containsAll(JCollection<Type>& collection) throw(const char*) { return THIS.ref().containsAll(collection); }
	virtual inline bool isEmpty() const throw(const char*) { return THIS.ref().isEmpty(); }
	virtual inline const JIterator<Type> iterator() const throw(const char*) { return THIS.ref().iterator(); }
	virtual inline bool remove(const Type& value) throw(const char*) { return THIS.ref().remove(value); }
	virtual inline bool removeAll(JCollection<Type>& collection) throw(const char*) { return THIS.ref().removeAll(collection); }
	virtual inline bool retainAll(JCollection<Type>& collection) throw(const char*) { return THIS.ref().retainAll(collection); }
	virtual inline int32_t size() const throw(const char*) { return THIS.ref().size(); }
};

#endif //JAPPSY_ULINKEDHASHSET_H

