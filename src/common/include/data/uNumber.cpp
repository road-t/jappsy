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

#include "uNumber.h"

namespace Java {

	int RefBoolean::compareTo(const Boolean& object) const throw(const char*) { return compare(siValue, object.shortValue()); }
	Boolean RefBoolean::decode(const JString& value) throw(const char*) { return Boolean((bool)value); }
	
	int RefByte::compareTo(const Byte& object) const throw(const char*) { return compare(bValue, object.byteValue()); }
	Byte RefByte::decode(const JString& value) throw(const char*) { return Byte((int8_t)value); }

}

int RefShort::compareTo(const Short& object) const throw(const char*) { return compare(siValue, object.shortValue()); }
Short RefShort::decode(const JString& value) throw(const char*) { return Short((int16_t)value); }

int RefInteger::compareTo(const Integer& object) const throw(const char*) { return compare(iValue, object.intValue()); }
Integer RefInteger::decode(const JString& value) throw(const char*) { return Integer((int32_t)value); }

int RefLong::compareTo(const Long& object) const throw(const char*) { return compare(iValue64, object.longValue()); }
Long RefLong::decode(const JString& value) throw(const char*) { return Long((int64_t)value); }

int RefFloat::compareTo(const Float& object) const throw(const char*) { return compare(dValue, object.floatValue()); }
Float RefFloat::decode(const JString& value) throw(const char*) { return Float((float)value); }

int RefDouble::compareTo(const Double& object) const throw(const char*) { return compare(dValue, object.doubleValue()); }
Double RefDouble::decode(const JString& value) throw(const char*) { return Double((double)value); }
