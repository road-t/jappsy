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
	List<Object> m_array = new ArrayList<Object>();
	
	inline RefJSONArray() { TYPE = TypeJSONArray; }
	
	inline ~RefJSONArray() { }
	
	inline Object& get(int index) const throw(const char*) {
		try {
			return *(Object*)&(m_array.get(index));
		} catch (...) {
			throw eOutOfRange;
		}
	}
	
	inline Object& opt(int index, Object& fallback) const {
		try {
			return *(Object*)&(m_array.get(index));
		} catch (...) {
			return fallback;
		}
	}
	
	inline JSONArray& getJSONArray(int index) const throw(const char*) {
		Object *o = (Object*)&(m_array.get(index));
		if (o->_object == NULL) {
			throw eNullPointer;
		} else if ((*o).ref().TYPE == TypeJSONArray) {
			return *((JSONArray*)o);
		} else {
			throw eConvert;
		}
	}
	
	inline JSONObject& getJSONObject(int index) const throw(const char*) {
		Object *o = (Object*)&(m_array.get(index));
		if (o->_object == NULL) {
			throw eNullPointer;
		} else if ((*o).ref().TYPE == TypeJSONObject) {
			return *((JSONObject*)o);
		} else {
			throw eConvert;
		}
	}
	
	inline JSONArray& optJSONArray(int index, JSONArray& fallback) const {
		try {
			return THIS.getJSONArray(index);
		} catch (...) {
			return fallback;
		}
	}
	
	inline JSONObject& optJSONObject(int index, JSONObject& fallback) const {
		try {
			return getJSONObject(index);
		} catch (...) {
			return fallback;
		}
	}

	inline bool getBoolean(int index) const throw(const char*) {
		Object* o = (Object*)&(THIS.get(index));
		if (o->_object == NULL) {
			return false;
		} else if ((*o).ref().TYPE == TypeString) {
			return ((String*)o)->operator bool();
		} else if ((*o).ref().TYPE == TypeBoolean) {
			return ((Java::Boolean*)o)->boolValue();
		} else if ((*o).ref().TYPE == TypeByte) {
			return ((Java::Byte*)o)->byteValue() != 0;
		} else if ((*o).ref().TYPE == TypeShort) {
			return ((Short*)o)->shortValue() != 0;
		} else if ((*o).ref().TYPE == TypeInt) {
			return ((Integer*)o)->intValue() != 0;
		} else if ((*o).ref().TYPE == TypeLong) {
			return ((Long*)o)->longValue() != 0;
		} else if ((*o).ref().TYPE == TypeFloat) {
			return ((Float*)o)->floatValue() != 0;
		} else if ((*o).ref().TYPE == TypeDouble) {
			return ((Double*)o)->doubleValue() != 0;
		} else {
			throw eConvert; // TypeMismatch
		}
	}
	
	inline bool optBoolean(int index, bool fallback = false) const {
		try {
			return THIS.getBoolean(index);
		} catch (...) {
			return fallback;
		}
	}
	
	inline int32_t getInt(int index) const throw(const char*) {
		Object* o = (Object*)&(THIS.get(index));
		if (o->_object == NULL) {
			return 0;
		} else if ((*o).ref().TYPE == TypeString) {
			return ((String*)o)->operator int32_t();
		} else if ((*o).ref().TYPE == TypeBoolean) {
			return ((Java::Boolean*)o)->boolValue() ? 1 : 0;
		} else if ((*o).ref().TYPE == TypeByte) {
			return ((Java::Byte*)o)->byteValue();
		} else if ((*o).ref().TYPE == TypeShort) {
			return ((Short*)o)->shortValue();
		} else if ((*o).ref().TYPE == TypeInt) {
			return ((Integer*)o)->intValue();
		} else if ((*o).ref().TYPE == TypeLong) {
			return (int32_t)((Long*)o)->longValue();
		} else if ((*o).ref().TYPE == TypeFloat) {
			return ((Float*)o)->floatValue();
		} else if ((*o).ref().TYPE == TypeDouble) {
			return ((Double*)o)->doubleValue();
		} else {
			throw eConvert; // TypeMismatch
		}
	}
	
	inline int32_t optInt(int index, int fallback = 0) const {
		try {
			return THIS.getInt(index);
		} catch (...) {
			return fallback;
		}
	}
	
	inline int64_t getLong(int index) const throw(const char*) {
		Object* o = (Object*)&(THIS.get(index));
		if (o->_object == NULL) {
			return 0;
		} else if ((*o).ref().TYPE == TypeString) {
			return ((String*)o)->operator int64_t();
		} else if ((*o).ref().TYPE == TypeBoolean) {
			return ((Java::Boolean*)o)->boolValue() ? 1 : 0;
		} else if ((*o).ref().TYPE == TypeByte) {
			return ((Java::Byte*)o)->byteValue();
		} else if ((*o).ref().TYPE == TypeShort) {
			return ((Short*)o)->shortValue();
		} else if ((*o).ref().TYPE == TypeInt) {
			return ((Integer*)o)->intValue();
		} else if ((*o).ref().TYPE == TypeLong) {
			return ((Long*)o)->longValue();
		} else if ((*o).ref().TYPE == TypeFloat) {
			return ((Float*)o)->floatValue();
		} else if ((*o).ref().TYPE == TypeDouble) {
			return ((Double*)o)->doubleValue();
		} else {
			throw eConvert; // TypeMismatch
		}
	}
	
	inline int64_t optLong(int index, int64_t fallback = 0) const {
		try {
			return THIS.getLong(index);
		} catch (...) {
			return fallback;
		}
	}
	
	inline double getDouble(int index) const throw(const char*) {
		Object* o = (Object*)&(THIS.get(index));
		if (o->_object == NULL) {
			return 0;
		} else if ((*o).ref().TYPE == TypeString) {
			return ((String*)o)->operator double();
		} else if ((*o).ref().TYPE == TypeBoolean) {
			return ((Java::Boolean*)o)->boolValue() ? 1 : 0;
		} else if ((*o).ref().TYPE == TypeByte) {
			return ((Java::Byte*)o)->byteValue();
		} else if ((*o).ref().TYPE == TypeShort) {
			return ((Short*)o)->shortValue();
		} else if ((*o).ref().TYPE == TypeInt) {
			return ((Integer*)o)->intValue();
		} else if ((*o).ref().TYPE == TypeLong) {
			return ((Long*)o)->longValue();
		} else if ((*o).ref().TYPE == TypeFloat) {
			return ((Float*)o)->floatValue();
		} else if ((*o).ref().TYPE == TypeDouble) {
			return ((Double*)o)->doubleValue();
		} else {
			throw eConvert; // TypeMismatch
		}
	}
	
	inline double optDouble(int index, double fallback = NAN) const {
		try {
			return THIS.getDouble(index);
		} catch (...) {
			return fallback;
		}
	}
	
	inline const String getString(int index) const throw(const char*) {
		Object* o = (Object*)&(THIS.get(index));
		if (o->_object == NULL) {
			return String();
		} else if ((*o).ref().TYPE == TypeString) {
			return *((String*)o);
		} else if ((*o).ref().TYPE == TypeBoolean) {
			return ((Java::Boolean*)o)->toString();
		} else if ((*o).ref().TYPE == TypeByte) {
			return ((Java::Byte*)o)->toString();
		} else if ((*o).ref().TYPE == TypeShort) {
			return ((Short*)o)->toString();
		} else if ((*o).ref().TYPE == TypeInt) {
			return ((Integer*)o)->toString();
		} else if ((*o).ref().TYPE == TypeLong) {
			return ((Long*)o)->toString();
		} else if ((*o).ref().TYPE == TypeFloat) {
			return ((Float*)o)->toString();
		} else if ((*o).ref().TYPE == TypeDouble) {
			return ((Double*)o)->toString();
		} else {
			throw eConvert; // TypeMismatch
		}
	}
	
	inline const String optString(int index, const String& fallback = null) const {
		try {
			return THIS.getString(index);
		} catch (...) {
			return fallback;
		}
	}
	
	inline bool isNull(int index) const throw(const char*) {
		try {
			Object* o = (Object*)&(THIS.get(index));
			return (o->_object == NULL);
		} catch (...) {
			throw eOutOfRange;
		}
	}
	
	inline int length() const { return m_array.size(); }
	
	inline RefJSONArray& put(bool value) throw(const char*) {
		m_array.push(Java::Boolean(value));
		return *this;
	}
	
	inline RefJSONArray& put(int32_t value) throw(const char*) {
		m_array.push(Integer(value));
		return *this;
	}
	
	inline RefJSONArray& put(int64_t value) throw(const char*) {
		m_array.push(Long(value));
		return *this;
	}
	
	inline RefJSONArray& put(double value) throw(const char*) {
		m_array.push(Double(value));
		return *this;
	}
	
	inline RefJSONArray& put(const String& value) throw(const char*) {
		m_array.push(String(value));
		return *this;
	}
	
	inline RefJSONArray& put(const Object& value) throw(const char*) {
		m_array.push(value);
		return *this;
	}
	
	inline RefJSONArray& put(int index, const Object& value) throw(const char*) {
		int size = m_array.size();
		if (size <= index) {
			while (size < index) {
				m_array.push(Object());
				size++;
			}
			m_array.push(value);
		} else {
			m_array.set(index, value);
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
		int size = m_array.size();
		if (size <= index) {
			while (size < index) {
				m_array.push(Object());
				size++;
			}
			m_array.push(value);
		} else {
			m_array.set(index, String(value));
		}
		return *this;
	}
	
	inline void remove(int index) throw(const char*) {
		m_array.ref().RefStack<Object>::remove(index);
	}
	
	inline String toJSON() const { return m_array.toJSON(); }
};

class JSONArray : public Object {
public:
	RefClass(JSONArray, RefJSONArray)
	
	inline Object& get(int index) const throw(const char*) { return THIS.ref().get(index); }
	inline Object& opt(int index, Object& fallback) const { return THIS.ref().opt(index, fallback); }
	inline JSONArray& getJSONArray(int index) const throw(const char*) { return THIS.ref().getJSONArray(index); }
	inline JSONObject& getJSONObject(int index) const throw(const char*) { return THIS.ref().getJSONObject(index); }
	inline JSONArray& optJSONArray(int index, JSONArray& fallback) const { return THIS.ref().optJSONArray(index, fallback); }
	inline JSONObject& optJSONObject(int index, JSONObject& fallback) const { return THIS.ref().optJSONObject(index, fallback); }
	inline bool getBoolean(int index) const throw(const char*) { return THIS.ref().getBoolean(index); }
	inline bool optBoolean(int index, bool fallback = false) const { return THIS.ref().optBoolean(index, fallback); }
	inline int32_t getInt(int index) const throw(const char*) { return THIS.ref().getInt(index); }
	inline int32_t optInt(int index, int fallback = 0) const { return THIS.ref().optInt(index, fallback); }
	inline int64_t getLong(int index) const throw(const char*) { return THIS.ref().getLong(index); }
	inline int64_t optLong(int index, int64_t fallback = 0) const { return THIS.ref().optLong(index, fallback); }
	inline double getDouble(int index) const throw(const char*) { return THIS.ref().getDouble(index); }
	inline double optDouble(int index, double fallback = NAN) const { return THIS.ref().optDouble(index, fallback); }
	inline const String getString(int index) const throw(const char*) { return THIS.ref().getString(index); }
	inline const String optString(int index, const String& fallback = null) const { return THIS.ref().optString(index, fallback); }
	inline bool isNull(int index) const throw(const char*) { return THIS.ref().isNull(index); }
	inline int length() const { return THIS.ref().length(); }
	inline RefJSONArray& put(bool value) throw(const char*) { return THIS.ref().put(value); }
	inline RefJSONArray& put(int32_t value) throw(const char*) { return THIS.ref().put(value); }
	inline RefJSONArray& put(int64_t value) throw(const char*) { return THIS.ref().put(value); }
	inline RefJSONArray& put(double value) throw(const char*) { return THIS.ref().put(value); }
	inline RefJSONArray& put(const String& value) throw(const char*) { return THIS.ref().put(value); }
	inline RefJSONArray& put(const Object& value) throw(const char*) { return THIS.ref().put(value); }
	inline RefJSONArray& put(int index, const Object& value) throw(const char*) { return THIS.ref().put(index, value); }
	inline RefJSONArray& put(int index, bool value) throw(const char*) { return THIS.ref().put(index, value); }
	inline RefJSONArray& put(int index, int32_t value) throw(const char*) { return THIS.ref().put(index, value); }
	inline RefJSONArray& put(int index, int64_t value) throw(const char*) { return THIS.ref().put(index, value); }
	inline RefJSONArray& put(int index, double value) throw(const char*) { return THIS.ref().put(index, value); }
	inline RefJSONArray& put(int index, const String& value) throw(const char*) { return THIS.ref().put(index, value); }
	inline void remove(int index) throw(const char*) { THIS.ref().remove(index); }
};

#endif //JAPPSY_UJSONARRAY_H