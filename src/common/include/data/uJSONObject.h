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

#ifndef JAPPSY_UJSONOBJECT_H
#define JAPPSY_UJSONOBJECT_H

#include <platform.h>
#include <data/uJSONArray.h>
#include <data/uHashMap.h>

class RefJSONObject : public RefObject {
public:
	HashMap<String, Object> m_object = new HashMap<String, Object>();
	
	inline RefJSONObject() { TYPE = TypeJSONObject; }
	
	RefJSONObject(const String& json) throw(const char*);
	RefJSONObject(const wchar_t* json) throw(const char*);
	RefJSONObject(const char* json) throw(const char*);
	
	inline ~RefJSONObject() { }
	
	inline const Object& get(const String& name) const throw(const char*) {
		return m_object->get(name);
	}
	
	inline const Object& opt(const String& name, const Object& fallback) const {
		try {
			return m_object->get(name);
		} catch (...) {
			return fallback;
		}
	}
	
	inline const JSONArray& getJSONArray(const String& name) const throw(const char*) {
		Object *o = (Object*)&(this->get(name));
		if (o->_object == NULL) {
			throw eNullPointer;
		} else if ((*o)->TYPE == TypeJSONArray) {
			return *((JSONArray*)o);
		} else {
			throw eConvert;
		}
	}
	
	inline const JSONObject& getJSONObject(const String& name) const throw(const char*) {
		Object *o = (Object*)&(this->get(name));
		if (o->_object == NULL) {
			throw eNullPointer;
		} else if ((*o)->TYPE == TypeJSONObject) {
			return *((JSONObject*)o);
		} else {
			throw eConvert;
		}
	}
	
	inline const JSONArray& optJSONArray(const String& name, const JSONArray& fallback) const {
		try {
			return this->getJSONArray(name);
		} catch (...) {
			return fallback;
		}
	}
	
	inline const JSONObject& optJSONObject(const String& name, const JSONObject& fallback) const {
		try {
			return this->getJSONObject(name);
		} catch (...) {
			return fallback;
		}
	}
	
	inline bool getBoolean(const String& name) const throw(const char*) {
		Object* o = (Object*)&(this->get(name));
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
	
	inline bool optBoolean(const String& name, bool fallback = false) const {
		try {
			return this->getBoolean(name);
		} catch (...) {
			return fallback;
		}
	}
	
	inline int32_t getInt(const String& name) const throw(const char*) {
		Object* o = (Object*)&(this->get(name));
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
	
	inline int32_t optInt(const String& name, int fallback = 0) const {
		try {
			return this->getInt(name);
		} catch (...) {
			return fallback;
		}
	}
	
	inline int64_t getLong(const String& name) const throw(const char*) {
		Object* o = (Object*)&(this->get(name));
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
	
	inline int64_t optLong(const String& name, int64_t fallback = 0) const {
		try {
			return this->getLong(name);
		} catch (...) {
			return fallback;
		}
	}
	
	inline double getDouble(const String& name) const throw(const char*) {
		Object* o = (Object*)&(this->get(name));
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
	
	inline double optDouble(const String& name, double fallback = NAN) const {
		try {
			return this->getDouble(name);
		} catch (...) {
			return fallback;
		}
	}
	
	inline String getString(const String& name) const throw(const char*) {
		Object* o = (Object*)&(this->get(name));
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
	
	inline String optString(const String& name, const String& fallback = null) const {
		try {
			return this->getString(name);
		} catch (...) {
			return fallback;
		}
	}
	
	inline bool has(const String& name) const {
		return m_object->containsKey(name);
	}
	
	inline bool isNull(const String& name) const throw(const char*) {
		try {
			Object* o = (Object*)&(this->get(name));
			return (o->_object == NULL);
		} catch (...) {
			throw eOutOfRange;
		}
	}
	
	inline const Iterator<String> keys() const { return m_object->keySet()->iterator(); }
	
	inline int length() const { return m_object->size(); }

	inline RefJSONObject& put(const String& name, const Object& object) throw(const char*) {
		m_object->put(name, object);
		return *this;
	}
	
	inline RefJSONObject& put(const String& name, bool value) throw(const char*) {
		m_object->put(name, Java::Boolean(value));
		return *this;
	}
	
	inline RefJSONObject& put(const String& name, int32_t value) throw(const char*) {
		m_object->put(name, Integer(value));
		return *this;
	}
	
	inline RefJSONObject& put(const String& name, int64_t value) throw(const char*) {
		m_object->put(name, Long(value));
		return *this;
	}
	
	inline RefJSONObject& put(const String& name, double value) throw(const char*) {
		m_object->put(name, Double(value));
		return *this;
	}
	
	inline RefJSONObject& put(const String& name, const String& value) throw(const char*) {
		m_object->put(name, String(value));
		return *this;
	}
	
	inline RefJSONObject& putOpt(const String& name, const Object& value) {
		if (value != null) {
			m_object->put(name, value);
		}
		return *this;
	}
	
	inline void remove(const String& name) throw(const char*) {
		m_object->remove(name);
	}
	
	const String toString(int indentSpaces = 0) const throw(const char*);

	inline String toJSON() const { return m_object.toJSON(); }
};

class JSONObject : public Object {
public:
	RefClass(JSONObject, JSONObject)
	
	inline JSONObject(const String& json) throw(const char*) {
		RefJSONObject* o = new RefJSONObject(json);
		if (o == NULL) throw eOutOfMemory;
		this->setRef(o);
	}
	
	inline JSONObject(const wchar_t* json) throw(const char*) {
		RefJSONObject* o = new RefJSONObject(json);
		if (o == NULL) throw eOutOfMemory;
		this->setRef(o);
	}
	
	inline JSONObject(const char* json) throw(const char*) {
		RefJSONObject* o = new RefJSONObject(json);
		if (o == NULL) throw eOutOfMemory;
		this->setRef(o);
	}

	inline const Object& get(const String& name) const throw(const char*) { CHECKTHIS; return THIS->get(name); }
	inline const Object& opt(const String& name, const Object& fallback) const throw(const char*) { CHECKTHIS; return THIS->opt(name, fallback); }
	inline const JSONArray& getJSONArray(const String& name) const throw(const char*) { CHECKTHIS; return THIS->getJSONArray(name); }
	inline const JSONObject& getJSONObject(const String& name) const throw(const char*) { CHECKTHIS; return THIS->getJSONObject(name); }
	inline const JSONArray& optJSONArray(const String& name, const JSONArray& fallback) const throw(const char*) { CHECKTHIS; return THIS->optJSONArray(name, fallback); }
	inline const JSONObject& optJSONObject(const String& name, const JSONObject& fallback) const throw(const char*) { CHECKTHIS; return THIS->optJSONObject(name, fallback); }
	inline bool getBoolean(const String& name) const throw(const char*) { CHECKTHIS; return THIS->getBoolean(name); }
	inline bool optBoolean(const String& name, bool fallback = false) const throw(const char*) { CHECKTHIS; return THIS->optBoolean(name, fallback); }
	inline int32_t getInt(const String& name) const throw(const char*) { CHECKTHIS; return THIS->getInt(name); }
	inline int32_t optInt(const String& name, int fallback = 0) const throw(const char*) { CHECKTHIS; return THIS->optInt(name, fallback); }
	inline int64_t getLong(const String& name) const throw(const char*) { CHECKTHIS; return THIS->getLong(name); }
	inline int64_t optLong(const String& name, int64_t fallback = 0) const throw(const char*) { CHECKTHIS; return THIS->optLong(name, fallback); }
	inline double getDouble(const String& name) const throw(const char*) { CHECKTHIS; return THIS->getDouble(name); }
	inline double optDouble(const String& name, double fallback = NAN) const throw(const char*) { CHECKTHIS; return THIS->optDouble(name, fallback); }
	inline const String getString(const String& name) const throw(const char*) { CHECKTHIS; return THIS->getString(name); }
	inline const String optString(const String& name, const String& fallback = null) const throw(const char*) { CHECKTHIS; return THIS->optString(name, fallback); }
	inline bool has(const String& name) const throw(const char*) { CHECKTHIS; return THIS->has(name); }
	inline bool isNull(const String& name) const throw(const char*) { CHECKTHIS; return THIS->isNull(name); }
	inline const Iterator<String> keys() const throw(const char*) { CHECKTHIS; return THIS->keys(); }
	inline int length() const throw(const char*) { CHECKTHIS; return THIS->length(); }
	inline RefJSONObject& put(const String& name, const Object& value) throw(const char*) { CHECKTHIS; return THIS->put(name, value); }
	inline RefJSONObject& put(const String& name, bool value) throw(const char*) { CHECKTHIS; return THIS->put(name, value); }
	inline RefJSONObject& put(const String& name, int32_t value) throw(const char*) { CHECKTHIS; return THIS->put(name, value); }
	inline RefJSONObject& put(const String& name, int64_t value) throw(const char*) { CHECKTHIS; return THIS->put(name, value); }
	inline RefJSONObject& put(const String& name, double value) throw(const char*) { CHECKTHIS; return THIS->put(name, value); }
	inline RefJSONObject& put(const String& name, const String& value) throw(const char*) { CHECKTHIS; return THIS->put(name, value); }
	inline RefJSONObject& putOpt(const String& name, const Object& value) throw(const char*) { CHECKTHIS; return THIS->putOpt(name, value); }
	inline void remove(const String& name) throw(const char*) { CHECKTHIS; THIS->remove(name); }
	const String toString(int indentSpaces = 0) const throw(const char*) { CHECKTHIS; return THIS->toString(); }
};

#endif //JAPPSY_UJSONOBJECT_H