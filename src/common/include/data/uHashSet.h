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

#ifndef JAPPSY_UHASHSET_H
#define JAPPSY_UHASHSET_H

#include <data/uSet.h>

template <typename Type>
class RefHashSet : public RefSet<Type> {
public:
	RefSet<Type>*** m_set = NULL;
	
	uint32_t m_count = 0;
	
	inline RefSet<Type>* createSet(const Type& object) throw(const char*) {
		uint32_t hash = object.hashCode();
		if (m_set == NULL) {
			m_set = memAlloc(RefSet<Type> **, m_set, 256 * sizeof(RefSet<Type> **));
			if (m_set == NULL) throw eOutOfMemory;
		}

		RefSet<Type> **s1 = m_set[hash & 0xFF];
		if (s1 == NULL) {
			s1 = memAlloc(RefSet<Type> *, s1, 256 * sizeof(RefSet<Type> *));
			if (s1 == NULL) throw eOutOfMemory;
			m_set[hash & 0xFF] = s1;
		}

		hash >>= 8;
		RefSet<Type> *s2 = s1[hash & 0xFF];
		if (s2 == NULL) {
			s2 = memNew(s2, RefSet<Type>());
			if (s2 == NULL) throw eOutOfMemory;
			s1[hash & 0xFF] = s2;
		}

		return s2;
	}
	
	inline RefSet<Type>* findSet(const Type& object) const {
		if (m_set != NULL) {
			uint32_t hash = object.hashCode();
			RefSet<Type> **s1 = m_set[hash & 0xFF];
			if (s1 != NULL) {
				hash >>= 8;
				RefSet<Type> *s2 = s1[hash & 0xFF];
				return s2;
			}
		}
		return NULL;
	}

protected:
	Iterator<Type> m_tempIt = new Iterator<Type>();
	
public:

	inline RefHashSet() { this->TYPE = TypeHashSet; }
	
	inline ~RefHashSet() {
		clear();
	}
	
	virtual inline void clear() throw(const char*) {
		if (m_set != NULL) {
			for (int i = 0; i < 256; i++) {
				if (m_set[i] != NULL) {
					RefSet<Type> **s1 = m_set[i];
					if (s1 != NULL) {
						for (int j = 0; j < 256; j++) {
							if (s1[j] != NULL) {
								RefSet<Type> *s2 = s1[j];
								if (s2 != NULL) {
									memDelete(s2);
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
		m_count = 0;
	}
	
	virtual inline bool add(const Type& object) throw(const char*) {
		RefSet<Type> *set = createSet(object);
		if (set != NULL) {
			if (!set->contains(object)) {
				m_count++;
				set->push(object);
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
		RefSet<Type> *set = findSet(object);
		if (set != NULL) {
			return set->contains(object);
		}
		return false;
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
		if (m_set != NULL) {
			for (int i = 0; i < 256; i++) {
				if (m_set[i] != NULL) {
					RefSet<Type> **s1 = m_set[i];
					for (int j = 0; j < 256; j++) {
						if (s1[j] != NULL) {
							RefSet<Type> *s2 = s1[j];
							if (s2 != NULL) {
								Iterator<Type> s2it = s2->iterator();
								while (s2it->hasNext()) {
									m_tempIt->push(s2it->next());
								}
							}
						}
					}
				}
			}
		}
		m_tempIt->reset(0);
		return m_tempIt;
	}
	
	virtual inline bool remove(const Type& object) throw(const char*) {
		RefSet<Type> *set = findSet(object);
		if (set != NULL) {
			try {
				if (set->remove(object)) {
					m_count--;
					return true;
				}
			} catch (const char* e) {
				if (e != eOutOfRange) {
					m_count--;
				}
				throw;
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
		
		if (m_set != NULL) {
			for (int i = 0; i < 256; i++) {
				if (m_set[i] != NULL) {
					RefSet<Type> **s1 = m_set[i];
					for (int j = 0; j < 256; j++) {
						if (s1[j] != NULL) {
							RefSet<Type> *s2 = s1[j];
							if (s2 != NULL) {
								Iterator<Type> it = s2->iterator();
								while (it->hasNext()) {
									Type o = it->next();
									if (!collection->contains(o)) {
										it->remove();
										m_count--;
										result = true;
									}
								}
							}
						}
					}
				}
			}
		}
		
		return result;
	}
	
	virtual inline int32_t size() const {
		return m_count;
	}
};

template <typename Type>
class HashSet : public Set<Type> {
public:
	RefClass(HashSet, HashSet<Type>)
	
	virtual inline bool add(const Type& object) throw(const char*) { CHECKTHIS; return THIS->add(object); }
	virtual inline bool addAll(Collection<Type>& collection) throw(const char*) { CHECKTHIS; return THIS->addAll(collection); }
	virtual inline void clear() throw(const char*) { CHECKTHIS; THIS->clear(); }
	virtual inline bool contains(const Type& value) const throw(const char*) { CHECKTHIS; return THIS->contains(value); }
	virtual inline bool containsAll(Collection<Type>& collection) throw(const char*) { CHECKTHIS; return THIS->containsAll(collection); }
	virtual inline bool isEmpty() const throw(const char*) { CHECKTHIS; return THIS->isEmpty(); }
	virtual inline const Iterator<Type> iterator() const throw(const char*) { CHECKTHIS; return THIS->iterator(); }
	virtual inline bool remove(const Type& value) throw(const char*) { CHECKTHIS; return THIS->remove(value); }
	virtual inline bool removeAll(Collection<Type>& collection) throw(const char*) { CHECKTHIS; return THIS->removeAll(collection); }
	virtual inline bool retainAll(Collection<Type>& collection) throw(const char*) { CHECKTHIS; return THIS->retainAll(collection); }
	virtual inline int32_t size() const throw(const char*) { CHECKTHIS; return THIS->size(); }
	
	inline static void _test() {
		HashSet<Object> set = new HashSet<Object>();
		Collection<Object> col = new Collection<Object>();
		set.add(null);
		set.addAll(col);
		set.clear();
		set.contains(null);
		set.containsAll(col);
		set.isEmpty();
		set.iterator();
		set.remove(null);
		set.removeAll(col);
		set.retainAll(col);
		set.size();
	}
};

#endif //JAPPSY_UHASHSET_H