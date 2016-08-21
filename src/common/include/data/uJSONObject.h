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

class RefJSONObject : public JRefObject {
public:
	HashMap<JString, JObject> m_object;

	inline void initialize() {
		TYPE = TypeJSONObject;
		m_object = new HashMap<JString, JObject>();
	}

	inline RefJSONObject() { initialize(); }
	
	RefJSONObject(const JString& json) throw(const char*);
	RefJSONObject(const wchar_t* json) throw(const char*);
	RefJSONObject(const char* json) throw(const char*);
	
	inline ~RefJSONObject() { }
	
	inline JObject& get(const JString& name) const throw(const char*) {
		return *(JObject*)&(m_object.get(name));
	}
	
	inline JObject& opt(const JString& name, JObject& fallback) const {
		try {
			return *(JObject*)&(m_object.get(name));
		} catch (...) {
			return fallback;
		}
	}
	
	inline JSONArray& getJSONArray(const JString& name) const throw(const char*) {
		JObject *o = (JObject*)&(THIS.get(name));
		if (o->_object == NULL) {
			throw eNullPointer;
		} else if ((*o).ref().TYPE == TypeJSONArray) {
			return *((JSONArray*)o);
		} else {
			throw eConvert;
		}
	}
	
	inline JSONObject& getJSONObject(const JString& name) const throw(const char*) {
		JObject *o = (JObject*)&(THIS.get(name));
		if (o->_object == NULL) {
			throw eNullPointer;
		} else if ((*o).ref().TYPE == TypeJSONObject) {
			return *((JSONObject*)o);
		} else {
			throw eConvert;
		}
	}
	
	inline JSONArray& optJSONArray(const JString& name, JSONArray& fallback) const {
		try {
			return THIS.getJSONArray(name);
		} catch (...) {
			return fallback;
		}
	}
	
	inline JSONObject& optJSONObject(const JString& name, JSONObject& fallback) const {
		try {
			return THIS.getJSONObject(name);
		} catch (...) {
			return fallback;
		}
	}
	
	inline bool getBoolean(const JString& name) const throw(const char*) {
		JObject* o = (JObject*)&(THIS.get(name));
		if (o->_object == NULL) {
			return false;
		} else if ((*o).ref().TYPE == TypeString) {
			return ((JString*)o)->operator bool();
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
	
	inline bool optBoolean(const JString& name, bool fallback = false) const {
		try {
			return THIS.getBoolean(name);
		} catch (...) {
			return fallback;
		}
	}
	
	inline int32_t getInt(const JString& name) const throw(const char*) {
		JObject* o = (JObject*)&(THIS.get(name));
		if (o->_object == NULL) {
			return 0;
		} else if ((*o).ref().TYPE == TypeString) {
			return ((JString*)o)->operator int32_t();
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
	
	inline int32_t optInt(const JString& name, int fallback = 0) const {
		try {
			return THIS.getInt(name);
		} catch (...) {
			return fallback;
		}
	}
	
	inline int64_t getLong(const JString& name) const throw(const char*) {
		JObject* o = (JObject*)&(THIS.get(name));
		if (o->_object == NULL) {
			return 0;
		} else if ((*o).ref().TYPE == TypeString) {
			return ((JString*)o)->operator int64_t();
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
	
	inline int64_t optLong(const JString& name, int64_t fallback = 0) const {
		try {
			return THIS.getLong(name);
		} catch (...) {
			return fallback;
		}
	}
	
	inline double getDouble(const JString& name) const throw(const char*) {
		JObject* o = (JObject*)&(THIS.get(name));
		if (o->_object == NULL) {
			return 0;
		} else if ((*o).ref().TYPE == TypeString) {
			return ((JString*)o)->operator double();
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
	
	inline double optDouble(const JString& name, double fallback = NAN) const {
		try {
			return THIS.getDouble(name);
		} catch (...) {
			return fallback;
		}
	}
	
	inline JString getString(const JString& name) const throw(const char*) {
		JObject* o = (JObject*)&(THIS.get(name));
		if (o->_object == NULL) {
			return JString();
		} else if ((*o).ref().TYPE == TypeString) {
			return *((JString*)o);
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
	
	inline JString optString(const JString& name, const JString& fallback = null) const {
		try {
			return THIS.getString(name);
		} catch (...) {
			return fallback;
		}
	}
	
	inline bool has(const JString& name) const {
		return m_object.containsKey(name);
	}
	
	inline bool isNull(const JString& name) const throw(const char*) {
		try {
			JObject* o = (JObject*)&(THIS.get(name));
			return (o->_object == NULL);
		} catch (...) {
			throw eOutOfRange;
		}
	}
	
	inline const JIterator<JString> keys() const { return m_object.keySet().iterator(); }
	
	inline int length() const { return m_object.size(); }

	inline RefJSONObject& put(const JString& name, const JObject& object) throw(const char*) {
		m_object.put(name, object);
		return *this;
	}
	
	inline RefJSONObject& put(const JString& name, bool value) throw(const char*) {
		m_object.put(name, Java::Boolean(value));
		return *this;
	}
	
	inline RefJSONObject& put(const JString& name, int32_t value) throw(const char*) {
		m_object.put(name, Integer(value));
		return *this;
	}
	
	inline RefJSONObject& put(const JString& name, int64_t value) throw(const char*) {
		m_object.put(name, Long(value));
		return *this;
	}
	
	inline RefJSONObject& put(const JString& name, double value) throw(const char*) {
		m_object.put(name, Double(value));
		return *this;
	}
	
	inline RefJSONObject& put(const JString& name, const JString& value) throw(const char*) {
		m_object.put(name, JString(value));
		return *this;
	}
	
	inline RefJSONObject& putOpt(const JString& name, const JObject& value) {
		if (value != null) {
			m_object.put(name, value);
		}
		return *this;
	}
	
	inline void remove(const JString& name) throw(const char*) {
		m_object.remove(name);
	}
	
	const JString toString(int indentSpaces = 0) const throw(const char*);

	inline JString toJSON() const { return m_object.toJSON(); }
};

class JSONObject : public JObject {
public:
	JRefClass(JSONObject, RefJSONObject)
	
	inline JSONObject(const JString& json) throw(const char*) {
		RefJSONObject* o = new RefJSONObject(json);
		if (o == NULL) throw eOutOfMemory;
		THIS.setRef(o);
	}
	
	inline JSONObject(const wchar_t* json) throw(const char*) {
		RefJSONObject* o = new RefJSONObject(json);
		if (o == NULL) throw eOutOfMemory;
		THIS.setRef(o);
	}
	
	inline JSONObject(const char* json) throw(const char*) {
		RefJSONObject* o = new RefJSONObject(json);
		if (o == NULL) throw eOutOfMemory;
		THIS.setRef(o);
	}

	inline JObject& get(const JString& name) const throw(const char*) { return THIS.ref().get(name); }
	inline JObject& opt(const JString& name, JObject& fallback) const throw(const char*) { return THIS.ref().opt(name, fallback); }
	inline JSONArray& getJSONArray(const JString& name) const throw(const char*) { return THIS.ref().getJSONArray(name); }
	inline JSONObject& getJSONObject(const JString& name) const throw(const char*) { return THIS.ref().getJSONObject(name); }
	inline JSONArray& optJSONArray(const JString& name, JSONArray& fallback) const throw(const char*) { return THIS.ref().optJSONArray(name, fallback); }
	inline JSONObject& optJSONObject(const JString& name, JSONObject& fallback) const throw(const char*) { return THIS.ref().optJSONObject(name, fallback); }
	inline bool getBoolean(const JString& name) const throw(const char*) { return THIS.ref().getBoolean(name); }
	inline bool optBoolean(const JString& name, bool fallback = false) const throw(const char*) { return THIS.ref().optBoolean(name, fallback); }
	inline int32_t getInt(const JString& name) const throw(const char*) { return THIS.ref().getInt(name); }
	inline int32_t optInt(const JString& name, int fallback = 0) const throw(const char*) { return THIS.ref().optInt(name, fallback); }
	inline int64_t getLong(const JString& name) const throw(const char*) { return THIS.ref().getLong(name); }
	inline int64_t optLong(const JString& name, int64_t fallback = 0) const throw(const char*) { return THIS.ref().optLong(name, fallback); }
	inline double getDouble(const JString& name) const throw(const char*) { return THIS.ref().getDouble(name); }
	inline double optDouble(const JString& name, double fallback = NAN) const throw(const char*) { return THIS.ref().optDouble(name, fallback); }
	inline const JString getString(const JString& name) const throw(const char*) { return THIS.ref().getString(name); }
	inline const JString optString(const JString& name, const JString& fallback = null) const throw(const char*) { return THIS.ref().optString(name, fallback); }
	inline bool has(const JString& name) const throw(const char*) { return THIS.ref().has(name); }
	inline bool isNull(const JString& name) const throw(const char*) { return THIS.ref().isNull(name); }
	inline const JIterator<JString> keys() const throw(const char*) { return THIS.ref().keys(); }
	inline int length() const throw(const char*) { return THIS.ref().length(); }
	inline RefJSONObject& put(const JString& name, const JObject& value) throw(const char*) { return THIS.ref().put(name, value); }
	inline RefJSONObject& put(const JString& name, bool value) throw(const char*) { return THIS.ref().put(name, value); }
	inline RefJSONObject& put(const JString& name, int32_t value) throw(const char*) { return THIS.ref().put(name, value); }
	inline RefJSONObject& put(const JString& name, int64_t value) throw(const char*) { return THIS.ref().put(name, value); }
	inline RefJSONObject& put(const JString& name, double value) throw(const char*) { return THIS.ref().put(name, value); }
	inline RefJSONObject& put(const JString& name, const JString& value) throw(const char*) { return THIS.ref().put(name, value); }
	inline RefJSONObject& putOpt(const JString& name, const JObject& value) throw(const char*) { return THIS.ref().putOpt(name, value); }
	inline void remove(const JString& name) throw(const char*) { THIS.ref().remove(name); }
	const JString toString(int indentSpaces = 0) const throw(const char*) { return THIS.ref().toString(); }
};

#endif //JAPPSY_UJSONOBJECT_H