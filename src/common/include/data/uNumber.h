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

#ifndef JAPPSY_UNUMBER_H
#define JAPPSY_UNUMBER_H

#include <platform.h>
#include <cipher/uCrc.h>
#include <data/uObject.h>
#include <data/uString.h>
#include <math.h>

enum NumberFormat { nfBool, nfFloat, nfInt };

class RefNumber : public RefObject {
protected:
	union {
		bool				bValue;
		int8_t				cValue;
		uint8_t             ucValue;
		int16_t             siValue;
		uint16_t            usiValue;
		int32_t				iValue;
		uint32_t            uiValue;
		int64_t             iValue64;
		uint64_t            uiValue64;
		float				fValue;
		double				dValue;
	};
	
	NumberFormat	format;
	
public:
	RefNumber() { TYPE = TypeNumber; }
	
	inline bool boolValue() const { return (format == nfInt) ? (iValue64 != 0) : ((format == nfFloat) ? (dValue != 0) : bValue); }
	inline int8_t byteValue() const { return (format == nfInt) ? cValue : ((format == nfFloat) ? ((int8_t)dValue) : (bValue ? 1 : 0)); }
	inline int16_t shortValue() const { return (format == nfInt) ? siValue : ((format == nfFloat) ? ((int32_t)dValue) : (bValue ? 1 : 0)); }
	inline int32_t intValue() const { return (format == nfInt) ? iValue : ((format == nfFloat) ? ((int32_t)dValue) : (bValue ? 1 : 0)); }
	inline int64_t longValue() const { return (format == nfInt) ? iValue64 : ((format == nfFloat) ? ((int64_t)dValue) : (bValue ? 1 : 0)); }
	inline float floatValue() const { return (format == nfFloat) ? dValue : ((format == nfInt) ? ((float)iValue64) : (bValue ? 1 : 0)); }
	inline double doubleValue() const { return (format == nfFloat) ? dValue : ((format == nfInt) ? ((double)iValue64) : (bValue ? 1 : 0)); }
	
	inline uint32_t hashCode() const {
		return mmcrc32(0xFFFFFFFF, (void*)&uiValue64, sizeof(uiValue64) + sizeof(format));
	}
};

class Number : public Object {
public:
	RefClass(Number, Number)
	
	inline bool boolValue() const { return THIS->boolValue(); }
	inline int8_t byteValue() const { return THIS->byteValue(); }
	inline int16_t shortValue() const { return THIS->shortValue(); }
	inline int32_t intValue() const { return THIS->intValue(); }
	inline int64_t longValue() const { return THIS->longValue(); }
	inline float floatValue() const { return THIS->floatValue(); }
	inline double doubleValue() const { return THIS->doubleValue(); }
};

#if defined(__IOS__)
namespace Java {
#endif
	
	class Boolean;
	
	class RefBoolean : public RefNumber {
	public:
		static const int SIZE = 8;
		
		inline RefBoolean() { TYPE = TypeBoolean; }
		inline RefBoolean(bool value) { TYPE = TypeBoolean; bValue = value; format = nfBool; }
		inline RefBoolean(const String& value) { TYPE = TypeShort; bValue = (bool)value; format = nfBool; }
		inline static int compare(bool lhs, bool rhs) { return (lhs == rhs) ? 0 : ((lhs == true) ? 1 : -1); }
		int compareTo(const Boolean& object) const;
		static Boolean decode(const String& value);
		inline String toString() const { return String(bValue); }
	};
	
	class Boolean : public Number {
	public:
		RefClass(Boolean, Boolean)
		
		inline Boolean(bool value) { setRef(new RefBoolean(value)); }
		inline Boolean(const String& value) { setRef(new RefBoolean(value)); }
		
		inline static String toString(bool value) { return String(value); }
		
		inline static int compare(bool lhs, bool rhs) { return (lhs == rhs) ? 0 : ((lhs == true) ? 1 : -1); }
		inline int compareTo(const Boolean& object) const {
			if (_object == NULL) {
				if (object._object == NULL) {
					return 0;
				} else {
					return -1;
				}
			} else if (object._object == NULL) {
				return 1;
			}
			return THIS->compareTo(object);
		}
		inline static Boolean decode(const String& value) { return RefBoolean::decode(value); }
		inline String toString() const { return THIS->toString(); }
		inline static Boolean valueOf(bool value) { return Boolean(value); }
		
		inline bool operator ==(const Boolean& src) const { return compareTo(src) == 0; }
	};
	
	class Byte;
	
	class RefByte : public RefNumber {
	public:
		static const int8_t MAX_VALUE = 0x7F;
		static const int8_t MIN_VALUE = 0x80;
		static const int SIZE = 8;
		
		inline RefByte() { TYPE = TypeByte; }
		inline RefByte(int8_t value) { TYPE = TypeByte; iValue64 = value; format = nfInt; }
		inline RefByte(const String& value) { TYPE = TypeByte; iValue64 = (int64_t)value; format = nfInt; }
		inline static int compare(int8_t lhs, int8_t rhs) { return (lhs < rhs) ? -1 : ((lhs == rhs) ? 0 : 1); }
		int compareTo(const Byte& object) const;
		static Byte decode(const String& value);
		inline String toString() const { return String(bValue); }
	};
	
	class Byte : public Number {
	public:
		RefClass(Byte, Byte)
		
		inline Byte(int8_t value) { setRef(new RefByte(value)); }
		inline Byte(const String& value) { setRef(new RefByte(value)); }
		
		inline static String toString(int8_t value) { return String(value); }
		
		inline static int compare(int8_t lhs, int8_t rhs) { return (lhs < rhs) ? -1 : ((lhs == rhs) ? 0 : 1); }
		inline int compareTo(const Byte& object) const {
			if (_object == NULL) {
				if (object._object == NULL) {
					return 0;
				} else {
					return -1;
				}
			} else if (object._object == NULL) {
				return 1;
			}
			return THIS->compareTo(object);
		}
		inline static Byte decode(const String& value) { return Byte::decode(value); }
		inline String toString() const { return THIS->toString(); }
		inline static Byte valueOf(int8_t value) { return Byte(value); }
		
		inline bool operator ==(const Byte& src) const { return compareTo(src) == 0; }
	};
	
#if defined(__IOS__)
}
#endif

class Short;

class RefShort : public RefNumber {
public:
	static const int16_t MAX_VALUE = 0x7FFF;
	static const int16_t MIN_VALUE = 0x8000;
	static const int SIZE = 16;
	
	inline RefShort() { TYPE = TypeShort; }
	inline RefShort(int16_t value) { TYPE = TypeShort; iValue64 = value; format = nfInt; }
	inline RefShort(const String& value) { TYPE = TypeShort; iValue64 = (int64_t)value; format = nfInt; }
	inline static int compare(int16_t lhs, int16_t rhs) { return (lhs < rhs) ? -1 : ((lhs == rhs) ? 0 : 1); }
	int compareTo(const Short& object) const;
	static Short decode(const String& value);
	inline String toString() const { return String(siValue); }
};

class Short : public Number {
public:
	RefClass(Short, Short)
	
	inline Short(int16_t value) { setRef(new RefShort(value)); }
	inline Short(const String& value) { setRef(new RefShort(value)); }
	
	inline static String toString(int16_t value) { return String(value); }
	
	inline static int compare(int16_t lhs, int16_t rhs) { return (lhs < rhs) ? -1 : ((lhs == rhs) ? 0 : 1); }
	inline int compareTo(const Short& object) const {
		if (_object == NULL) {
			if (object._object == NULL) {
				return 0;
			} else {
				return -1;
			}
		} else if (object._object == NULL) {
			return 1;
		}
		return THIS->compareTo(object);
	}
	inline static Short decode(const String& value) { return Short::decode(value); }
	inline String toString() const { return THIS->toString(); }
	inline static Short valueOf(int16_t value) { return Short(value); }
	
	inline bool operator ==(const Short& src) const { return compareTo(src) == 0; }
};

class Integer;

class RefInteger : public RefNumber {
public:
	static const int32_t MAX_VALUE = 0x7FFFFFFF;
	static const int32_t MIN_VALUE = 0x80000000;
	static const int SIZE = 32;
	
	inline RefInteger() { TYPE = TypeInt; }
	inline RefInteger(int32_t value) { TYPE = TypeInt; iValue64 = value; format = nfInt; }
	inline RefInteger(const String& value) { TYPE = TypeInt; iValue64 = (int64_t)value; format = nfInt; }
	inline static int compare(int32_t lhs, int32_t rhs) { return (lhs < rhs) ? -1 : ((lhs == rhs) ? 0 : 1); }
	int compareTo(const Integer& object) const;
	static Integer decode(const String& value);
	inline String toString() const { return String(iValue); }
};

class Integer : public Number {
public:
	RefClass(Integer, Integer)
	
	inline Integer(int32_t value) { setRef(new RefInteger(value)); }
	inline Integer(const String& value) { setRef(new RefInteger(value)); }
	
	inline static String toString(int32_t value) { return String(value); }
	
	inline static int compare(int32_t lhs, int32_t rhs) { return (lhs < rhs) ? -1 : ((lhs == rhs) ? 0 : 1); }
	inline int compareTo(const Integer& object) const {
		if (_object == NULL) {
			if (object._object == NULL) {
				return 0;
			} else {
				return -1;
			}
		} else if (object._object == NULL) {
			return 1;
		}
		return THIS->compareTo(object);
	}
	inline static Integer decode(const String& value) { return Integer::decode(value); }
	inline String toString() const { return THIS->toString(); }
	inline static Integer valueOf(int32_t value) { return Integer(value); }
	inline static Integer valueOf(const String& value, int radix = 10) {
		if (radix == 10) {
			return valueOf((int32_t)value);
		} else {
			return valueOf((int32_t)(String(L"0x") += value));
		}
	}
	
	inline bool operator ==(const Integer& src) const { return compareTo(src) == 0; }
};

class Long;

class RefLong : public RefNumber {
public:
	static const int64_t MAX_VALUE = 0x7FFFFFFFFFFFFFFFLL;
	static const int64_t MIN_VALUE = 0x8000000000000000LL;
	static const int SIZE = 64;
	
	inline RefLong() { TYPE = TypeLong; }
	inline RefLong(int64_t value) { TYPE = TypeLong; iValue64 = value; format = nfInt; }
	inline RefLong(const String& value) { TYPE = TypeLong; iValue64 = (int64_t)value; format = nfInt; }
	inline static int compare(int64_t lhs, int64_t rhs) { return (lhs < rhs) ? -1 : ((lhs == rhs) ? 0 : 1); }
	int compareTo(const Long& object) const;
	static Long decode(const String& value);
	inline String toString() const { return String(iValue64); }
};

class Long : public Number {
public:
	RefClass(Long, Long)
	
	inline Long(int64_t value) { setRef(new RefLong(value)); }
	inline Long(const String& value) { setRef(new RefLong(value)); }
	
	inline static String toString(int64_t value) { return String(value); }
	
	inline static int compare(int64_t lhs, int64_t rhs) { return (lhs < rhs) ? -1 : ((lhs == rhs) ? 0 : 1); }
	inline int compareTo(const Long& object) const {
		if (_object == NULL) {
			if (object._object == NULL) {
				return 0;
			} else {
				return -1;
			}
		} else if (object._object == NULL) {
			return 1;
		}
		return THIS->compareTo(object);
	}
	inline static Long decode(const String& value) { return Long::decode(value); }
	inline String toString() const { return THIS->toString(); }
	inline static Long valueOf(int64_t value) { return Long(value); }
	
	inline bool operator ==(const Long& src) const { return compareTo(src) == 0; }
};

class Float;

class RefFloat : public RefNumber {
public:
	static const int MAX_EXPONENT = 127;
	constexpr static const float MAX_VALUE = 3.4028235E+38f;
	static const int MIN_EXPONENT = -126;
	constexpr static const float MIN_NORMAL = 1.17549435E-38f;
	constexpr static const float MIN_VALUE = 1.4E-45f;
	constexpr static const float NEGATIVE_INFINITY = -INFINITY;
	constexpr static const float NaN = NAN;
	constexpr static const float POSITIVE_INFINITY = INFINITY;
	static const int SIZE = 32;
	
	inline RefFloat() { TYPE = TypeFloat; }
	inline RefFloat(float value) { TYPE = TypeFloat; dValue = value; format = nfFloat; }
	inline RefFloat(const String& value) { TYPE = TypeFloat; dValue = (float)value; format = nfFloat; }
	inline static int compare(float lhs, float rhs) { return (lhs < rhs) ? -1 : ((lhs == rhs) ? 0 : 1); }
	int compareTo(const Float& object) const;
	static Float decode(const String& value);
	inline String toString() const { return String((float)dValue); }
};

class Float : public Number {
public:
	RefClass(Float, Float)
	
	inline Float(float value) { setRef(new RefFloat(value)); }
	inline Float(const String& value) { setRef(new RefFloat(value)); }
	
	inline static String toString(float value) { return String(value); }
	
	inline static int compare(float lhs, float rhs) { return (lhs < rhs) ? -1 : ((lhs == rhs) ? 0 : 1); }
	inline int compareTo(const Float& object) const {
		if (_object == NULL) {
			if (object._object == NULL) {
				return 0;
			} else {
				return -1;
			}
		} else if (object._object == NULL) {
			return 1;
		}
		return THIS->compareTo(object);
	}
	inline static Float decode(const String& value) { return Float::decode(value); }
	inline String toString() const { return THIS->toString(); }
	inline static Float valueOf(float value) { return Float(value); }
	
	inline bool operator ==(const Float& src) const { return compareTo(src) == 0; }
};

class Double;

class RefDouble : public RefNumber {
public:
	static const int MAX_EXPONENT = 1023;
	constexpr static const double MAX_VALUE = 1.7976931348623157E+308;
	static const int MIN_EXPONENT = -1022;
	constexpr static const double MIN_NORMAL = 2.2250738585072014E-308;
	constexpr static const double MIN_VALUE = 4.9E-324;
	constexpr static const double NEGATIVE_INFINITY = -INFINITY;
	constexpr static const double NaN = NAN;
	constexpr static const double POSITIVE_INFINITY = INFINITY;
	static const int SIZE = 64;
	
	inline RefDouble() { TYPE = TypeDouble; }
	inline RefDouble(double value) { TYPE = TypeDouble; dValue = value; format = nfFloat; }
	inline RefDouble(const String& value) { TYPE = TypeDouble; dValue = (double)value; format = nfFloat; }
	inline static int compare(double lhs, double rhs) { return (lhs < rhs) ? -1 : ((lhs == rhs) ? 0 : 1); }
	int compareTo(const Double& object) const;
	inline static Double decode(const String& value);
	inline String toString() const { return String(dValue); }
};

class Double : public Number {
public:
	RefClass(Double, Double)
	
	inline Double(double value) { setRef(new RefDouble(value)); }
	inline Double(const String& value) { setRef(new RefDouble(value)); }
	
	inline static String toString(double value) { return String(value); }
	
	inline static int compare(double lhs, double rhs) { return (lhs < rhs) ? -1 : ((lhs == rhs) ? 0 : 1); }
	inline int compareTo(const Double& object) const {
		if (_object == NULL) {
			if (object._object == NULL) {
				return 0;
			} else {
				return -1;
			}
		} else if (object._object == NULL) {
			return 1;
		}
		return THIS->compareTo(object);
	}
	inline static Double decode(const String& value) { return Double::decode(value); }
	inline String toString() const { return THIS->toString(); }
	inline static Double valueOf(double value) { return Double(value); }
	
	inline bool operator ==(const Double& src) const { return compareTo(src) == 0; }
};

#endif
