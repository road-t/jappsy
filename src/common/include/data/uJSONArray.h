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

#ifndef JAPPSY_UJSONARRAY_H
#define JAPPSY_UJSONARRAY_H

#include <platform.h>
#include <data/uObject.h>
#include <data/uNumber.h>
#include <data/uList.h>

class JSONArray;
class JSONObject;

class RefJSONArray : public RefObject {
public:
	List<Object> m_array = new ArrayList<Object>;
	
	inline RefJSONArray() { TYPE = TypeJSONArray; }
	
	inline ~RefJSONArray() { }
	
	inline const Object& get(int index) const throw(const char*) {
		try {
			return m_array->get(index);
		} catch (...) {
			throw eOutOfRange;
		}
	}
	
	inline const Object& opt(int index, const Object& fallback) const {
		try {
			return m_array->get(index);
		} catch (...) {
			return fallback;
		}
	}
	
	inline const JSONArray& getJSONArray(int index) const throw(const char*) {
		Object *o = (Object*)&(m_array->get(index));
		if (o->_object == NULL) {
			throw eNullPointer;
		} else if ((*o)->TYPE == TypeJSONArray) {
			return *((JSONArray*)o);
		} else {
			throw eConvert;
		}
	}
	
	inline const JSONObject& getJSONObject(int index) const throw(const char*) {
		Object *o = (Object*)&(m_array->get(index));
		if (o->_object == NULL) {
			throw eNullPointer;
		} else if ((*o)->TYPE == TypeJSONObject) {
			return *((JSONObject*)o);
		} else {
			throw eConvert;
		}
	}
	
	inline const JSONArray& optJSONArray(int index, const JSONArray& fallback) const {
		try {
			return this->getJSONArray(index);
		} catch (...) {
			return fallback;
		}
	}
	
	inline const JSONObject& optJSONObject(int index, const JSONObject& fallback) const {
		try {
			return getJSONObject(index);
		} catch (...) {
			return fallback;
		}
	}
	
	inline bool getBoolean(int index) const throw(const char*) {
		Object* o = (Object*)&(this->get(index));
		if (o->_object == NULL) {
			return false;
		} else if ((*o)->TYPE == TypeString) {
			return ((String*)o)->operator bool();
		} else if ((*o)->TYPE == TypeBoolean) {
			return ((Java::Boolean*)o)->boolValue();
		} else if ((*o)->TYPE == TypeByte) {
			return ((Java::Byte*)o)->byteValue() != 0;
		} else if ((*o)->TYPE == TypeShort) {
			return ((Short*)o)->shortValue() != 0;
		} else if ((*o)->TYPE == TypeInt) {
			return ((Integer*)o)->intValue() != 0;
		} else if ((*o)->TYPE == TypeLong) {
			return ((Long*)o)->longValue() != 0;
		} else if ((*o)->TYPE == TypeFloat) {
			return ((Float*)o)->floatValue() != 0;
		} else if ((*o)->TYPE == TypeDouble) {
			return ((Double*)o)->doubleValue() != 0;
		} else {
			throw eConvert; // TypeMismatch
		}
	}
	
	inline bool optBoolean(int index, bool fallback = false) const {
		try {
			return this->getBoolean(index);
		} catch (...) {
			return fallback;
		}
	}
	
	inline int32_t getInt(int index) const throw(const char*) {
		Object* o = (Object*)&(this->get(index));
		if (o->_object == NULL) {
			return 0;
		} else if ((*o)->TYPE == TypeString) {
			return ((String*)o)->operator int32_t();
		} else if ((*o)->TYPE == TypeBoolean) {
			return ((Java::Boolean*)o)->boolValue() ? 1 : 0;
		} else if ((*o)->TYPE == TypeByte) {
			return ((Java::Byte*)o)->byteValue();
		} else if ((*o)->TYPE == TypeShort) {
			return ((Short*)o)->shortValue();
		} else if ((*o)->TYPE == TypeInt) {
			return ((Integer*)o)->intValue();
		} else if ((*o)->TYPE == TypeLong) {
			return (int32_t)((Long*)o)->longValue();
		} else if ((*o)->TYPE == TypeFloat) {
			return ((Float*)o)->floatValue();
		} else if ((*o)->TYPE == TypeDouble) {
			return ((Double*)o)->doubleValue();
		} else {
			throw eConvert; // TypeMismatch
		}
	}
	
	inline int32_t optInt(int index, int fallback = 0) const {
		try {
			return this->getInt(index);
		} catch (...) {
			return fallback;
		}
	}
	
	inline int64_t getLong(int index) const throw(const char*) {
		Object* o = (Object*)&(this->get(index));
		if (o->_object == NULL) {
			return 0;
		} else if ((*o)->TYPE == TypeString) {
			return ((String*)o)->operator int64_t();
		} else if ((*o)->TYPE == TypeBoolean) {
			return ((Java::Boolean*)o)->boolValue() ? 1 : 0;
		} else if ((*o)->TYPE == TypeByte) {
			return ((Java::Byte*)o)->byteValue();
		} else if ((*o)->TYPE == TypeShort) {
			return ((Short*)o)->shortValue();
		} else if ((*o)->TYPE == TypeInt) {
			return ((Integer*)o)->intValue();
		} else if ((*o)->TYPE == TypeLong) {
			return ((Long*)o)->longValue();
		} else if ((*o)->TYPE == TypeFloat) {
			return ((Float*)o)->floatValue();
		} else if ((*o)->TYPE == TypeDouble) {
			return ((Double*)o)->doubleValue();
		} else {
			throw eConvert; // TypeMismatch
		}
	}
	
	inline int64_t optLong(int index, int64_t fallback = 0) const {
		try {
			return this->getLong(index);
		} catch (...) {
			return fallback;
		}
	}
	
	inline double getDouble(int index) const throw(const char*) {
		Object* o = (Object*)&(this->get(index));
		if (o->_object == NULL) {
			return 0;
		} else if ((*o)->TYPE == TypeString) {
			return ((String*)o)->operator double();
		} else if ((*o)->TYPE == TypeBoolean) {
			return ((Java::Boolean*)o)->boolValue() ? 1 : 0;
		} else if ((*o)->TYPE == TypeByte) {
			return ((Java::Byte*)o)->byteValue();
		} else if ((*o)->TYPE == TypeShort) {
			return ((Short*)o)->shortValue();
		} else if ((*o)->TYPE == TypeInt) {
			return ((Integer*)o)->intValue();
		} else if ((*o)->TYPE == TypeLong) {
			return ((Long*)o)->longValue();
		} else if ((*o)->TYPE == TypeFloat) {
			return ((Float*)o)->floatValue();
		} else if ((*o)->TYPE == TypeDouble) {
			return ((Double*)o)->doubleValue();
		} else {
			throw eConvert; // TypeMismatch
		}
	}
	
	inline double optDouble(int index, double fallback = NAN) const {
		try {
			return this->getDouble(index);
		} catch (...) {
			return fallback;
		}
	}
	
	inline const String getString(int index) const throw(const char*) {
		Object* o = (Object*)&(this->get(index));
		if (o->_object == NULL) {
			return String();
		} else if ((*o)->TYPE == TypeString) {
			return *((String*)o);
		} else if ((*o)->TYPE == TypeBoolean) {
			return ((Java::Boolean*)o)->toString();
		} else if ((*o)->TYPE == TypeByte) {
			return ((Java::Byte*)o)->toString();
		} else if ((*o)->TYPE == TypeShort) {
			return ((Short*)o)->toString();
		} else if ((*o)->TYPE == TypeInt) {
			return ((Integer*)o)->toString();
		} else if ((*o)->TYPE == TypeLong) {
			return ((Long*)o)->toString();
		} else if ((*o)->TYPE == TypeFloat) {
			return ((Float*)o)->toString();
		} else if ((*o)->TYPE == TypeDouble) {
			return ((Double*)o)->toString();
		} else {
			throw eConvert; // TypeMismatch
		}
	}
	
	inline const String optString(int index, const String& fallback = null) const {
		try {
			return this->getString(index);
		} catch (...) {
			return fallback;
		}
	}
	
	inline bool isNull(int index) const throw(const char*) {
		try {
			Object* o = (Object*)&(this->get(index));
			return (o->_object == NULL);
		} catch (...) {
			throw eOutOfRange;
		}
	}
	
	inline int length() const { return m_array->size(); }
	
	inline RefJSONArray& put(bool value) throw(const char*) {
		m_array->push(Java::Boolean(value));
		return *this;
	}
	
	inline RefJSONArray& put(int32_t value) throw(const char*) {
		m_array->push(Integer(value));
		return *this;
	}
	
	inline RefJSONArray& put(int64_t value) throw(const char*) {
		m_array->push(Long(value));
		return *this;
	}
	
	inline RefJSONArray& put(double value) throw(const char*) {
		m_array->push(Double(value));
		return *this;
	}
	
	inline RefJSONArray& put(const String& value) throw(const char*) {
		m_array->push(String(value));
		return *this;
	}
	
	inline RefJSONArray& put(const Object& value) throw(const char*) {
		m_array->push(value);
		return *this;
	}
	
	inline RefJSONArray& put(int index, const Object& value) throw(const char*) {
		int size = m_array->size();
		if (size <= index) {
			while (size < index) {
				m_array->push(Object());
				size++;
			}
			m_array->push(value);
		} else {
			m_array->set(index, value);
		}
		return *this;
	}
	
	inline RefJSONArray& put(int index, bool value) throw(const char*) {
		return put(index, Java::Boolean(value));
	}
	
	inline RefJSONArray& put(int index, int32_t value) throw(const char*) {
		return put(index, Integer(value));
	}
	
	inline RefJSONArray& put(int index, int64_t value) throw(const char*) {
		return put(index, Long(value));
	}
	
	inline RefJSONArray& put(int index, double value) throw(const char*) {
		if (isnan(value)) {
			throw eInvalidParams;
		} else if (isinf(value)) {
			throw eInvalidParams;
		}
		return put(index, Double(value));
	}
	
	inline RefJSONArray& put(int index, const String& value) {
		int size = m_array->size();
		if (size <= index) {
			while (size < index) {
				m_array->push(Object());
				size++;
			}
			m_array->push(value);
		} else {
			m_array->set(index, String(value));
		}
		return *this;
	}
	
	inline void remove(int index) throw(const char*) {
		m_array->RefStack<Object>::remove(index);
	}
	
	inline String toJSON() const { return m_array.toJSON(); }
};

class JSONArray : public Object {
public:
	RefClass(JSONArray, JSONArray)
	
	inline const Object& get(int index) const throw(const char*) { CHECKTHIS; return THIS->get(index); }
	inline const Object& opt(int index, const Object& fallback) const { CHECKTHIS; return THIS->opt(index, fallback); }
	inline const JSONArray& getJSONArray(int index) const throw(const char*) { CHECKTHIS; return THIS->getJSONArray(index); }
	inline const JSONObject& getJSONObject(int index) const throw(const char*) { CHECKTHIS; return THIS->getJSONObject(index); }
	inline const JSONArray& optJSONArray(int index, const JSONArray& fallback) const { CHECKTHIS; return THIS->optJSONArray(index, fallback); }
	inline const JSONObject& optJSONObject(int index, const JSONObject& fallback) const { CHECKTHIS; return THIS->optJSONObject(index, fallback); }
	inline bool getBoolean(int index) const throw(const char*) { CHECKTHIS; return THIS->getBoolean(index); }
	inline bool optBoolean(int index, bool fallback = false) const { CHECKTHIS; return THIS->optBoolean(index, fallback); }
	inline int32_t getInt(int index) const throw(const char*) { CHECKTHIS; return THIS->getInt(index); }
	inline int32_t optInt(int index, int fallback = 0) const { CHECKTHIS; return THIS->optInt(index, fallback); }
	inline int64_t getLong(int index) const throw(const char*) { CHECKTHIS; return THIS->getLong(index); }
	inline int64_t optLong(int index, int64_t fallback = 0) const { CHECKTHIS; return THIS->optLong(index, fallback); }
	inline double getDouble(int index) const throw(const char*) { CHECKTHIS; return THIS->getDouble(index); }
	inline double optDouble(int index, double fallback = NAN) const { CHECKTHIS; return THIS->optDouble(index, fallback); }
	inline const String getString(int index) const throw(const char*) { CHECKTHIS; return THIS->getString(index); }
	inline const String optString(int index, const String& fallback = null) const { CHECKTHIS; return THIS->optString(index, fallback); }
	inline bool isNull(int index) const throw(const char*) { CHECKTHIS; return THIS->isNull(index); }
	inline int length() const { CHECKTHIS; return THIS->length(); }
	inline RefJSONArray& put(bool value) throw(const char*) { CHECKTHIS; return THIS->put(value); }
	inline RefJSONArray& put(int32_t value) throw(const char*) { CHECKTHIS; return THIS->put(value); }
	inline RefJSONArray& put(int64_t value) throw(const char*) { CHECKTHIS; return THIS->put(value); }
	inline RefJSONArray& put(double value) throw(const char*) { CHECKTHIS; return THIS->put(value); }
	inline RefJSONArray& put(const String& value) throw(const char*) { CHECKTHIS; return THIS->put(value); }
	inline RefJSONArray& put(const Object& value) throw(const char*) { CHECKTHIS; return THIS->put(value); }
	inline RefJSONArray& put(int index, const Object& value) throw(const char*) { CHECKTHIS; return THIS->put(index, value); }
	inline RefJSONArray& put(int index, bool value) throw(const char*) { CHECKTHIS; return THIS->put(index, value); }
	inline RefJSONArray& put(int index, int32_t value) throw(const char*) { CHECKTHIS; return THIS->put(index, value); }
	inline RefJSONArray& put(int index, int64_t value) throw(const char*) { CHECKTHIS; return THIS->put(index, value); }
	inline RefJSONArray& put(int index, double value) throw(const char*) { CHECKTHIS; return THIS->put(index, value); }
	inline RefJSONArray& put(int index, const String& value) throw(const char*) { CHECKTHIS; return THIS->put(index, value); }
	inline void remove(int index) throw(const char*) { CHECKTHIS; THIS->remove(index); }
};

#endif //JAPPSY_UJSONARRAY_H