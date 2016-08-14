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


#if defined(__IOS__)
namespace Java {
#endif
	
	int RefBoolean::compareTo(const Boolean& object) const { return compare(siValue, object->shortValue()); }
	Boolean RefBoolean::decode(const String& value) { return Boolean((bool)value); }
	
	int RefByte::compareTo(const Byte& object) const { return compare(bValue, object->byteValue()); }
	Byte RefByte::decode(const String& value) { return Byte((int8_t)value); }

#if defined(__IOS__)
}
#endif

int RefShort::compareTo(const Short& object) const { return compare(siValue, object->shortValue()); }
Short RefShort::decode(const String& value) { return Short((int16_t)value); }

int RefInteger::compareTo(const Integer& object) const { return compare(iValue, object->intValue()); }
Integer RefInteger::decode(const String& value) { return Integer((int32_t)value); }

int RefLong::compareTo(const Long& object) const { return compare(iValue64, object->longValue()); }
Long RefLong::decode(const String& value) { return Long((int64_t)value); }

int RefFloat::compareTo(const Float& object) const { return compare(dValue, object->floatValue()); }
Float RefFloat::decode(const String& value) { return Float((float)value); }

int RefDouble::compareTo(const Double& object) const { return compare(dValue, object->doubleValue()); }
Double RefDouble::decode(const String& value) { return Double((double)value); }
