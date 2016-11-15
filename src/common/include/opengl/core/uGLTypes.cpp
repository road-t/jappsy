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

#include "uGLTypes.h"

/*
GLRect& GLRect::operator =(GLRect rhs) {
	left = rhs.left; right = rhs.right; top = rhs.top; bottom = rhs.bottom;
	
	return *this;
}
 */

GLRect& GLRect::operator =(const GLRect& rhs) {
	if (this != &rhs) {
		left = rhs.left; right = rhs.right; top = rhs.top; bottom = rhs.bottom;
	}
	
	return *this;
}

GLRect& GLRect::operator |=(const GLRect& rhs) {
	if (this != &rhs) {
		if (rhs.left < left) left = rhs.left;
		if (rhs.top < top) top = rhs.top;
		if (rhs.right > right) right = rhs.right;
		if (rhs.bottom > bottom) bottom = rhs.bottom;
	}
	
	return *this;
}

GLRect operator |(GLRect lhs, const GLRect& rhs) {
	lhs |= rhs;
	return lhs;
}

GLRect& GLRect::operator &=(const GLRect& rhs) {
	if (this != &rhs) {
		if (left < rhs.left) left = rhs.left;
		if (top < rhs.top) top = rhs.top;
		if (right > rhs.right) right = rhs.right;
		if (bottom > rhs.bottom) bottom = rhs.bottom;
	}
	
	return *this;
}

GLRect operator &(GLRect lhs, const GLRect& rhs) {
	lhs &= rhs;
	return lhs;
}
