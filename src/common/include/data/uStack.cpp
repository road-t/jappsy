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

#include "uStack.h"

template <>
bool CStack<const wchar_t*>::contains(const wchar_t *const & value) const {
	if (m_count > 0) {
		for (int i = m_count-1; i >= 0; i--) {
			if (wcscmp(m_stack[i], value) == 0) return true;
		}
	}
	return false;
}

template <>
int32_t CStack<const wchar_t*>::search(const wchar_t *const & value) const {
	if (m_count > 0) {
		for (int i = m_count-1; i >= 0; i--) {
			if (wcscmp(m_stack[i], value) == 0) return i;
		}
	}
	return -1;
}

template <>
int32_t CStack<const wchar_t*>::indexOf(const wchar_t *const & value) const {
	if (m_count > 0) {
		for (int i = m_count-1; i >= 0; i--) {
			if (wcscmp(m_stack[i], value) == 0) return i;
		}
	}
	return -1;
}

template <>
bool CStack<wchar_t*>::contains(wchar_t *const & value) const {
	if (m_count > 0) {
		for (int i = m_count-1; i >= 0; i--) {
			if (wcscmp(m_stack[i], value) == 0) return true;
		}
	}
	return false;
}

template <>
int32_t CStack<wchar_t*>::search(wchar_t *const & value) const {
	if (m_count > 0) {
		for (int i = m_count-1; i >= 0; i--) {
			if (wcscmp(m_stack[i], value) == 0) return i;
		}
	}
	return -1;
}

template <>
int32_t CStack<wchar_t*>::indexOf(wchar_t *const & value) const {
	if (m_count > 0) {
		for (int i = m_count-1; i >= 0; i--) {
			if (wcscmp(m_stack[i], value) == 0) return i;
		}
	}
	return -1;
}

template <>
bool CStack<const char*>::contains(const char *const & value) const {
	if (m_count > 0) {
		for (int i = m_count-1; i >= 0; i--) {
			if (strcmp(m_stack[i], value) == 0) return true;
		}
	}
	return false;
}

template <>
int32_t CStack<const char*>::search(const char *const & value) const {
	if (m_count > 0) {
		for (int i = m_count-1; i >= 0; i--) {
			if (strcmp(m_stack[i], value) == 0) return i;
		}
	}
	return -1;
}

template <>
int32_t CStack<const char*>::indexOf(const char *const & value) const {
	if (m_count > 0) {
		for (int i = m_count-1; i >= 0; i--) {
			if (strcmp(m_stack[i], value) == 0) return i;
		}
	}
	return -1;
}

template <>
bool CStack<char*>::contains(char *const & value) const {
	if (m_count > 0) {
		for (int i = m_count-1; i >= 0; i--) {
			if (strcmp(m_stack[i], value) == 0) return true;
		}
	}
	return false;
}

template <>
int32_t CStack<char*>::search(char *const & value) const {
	if (m_count > 0) {
		for (int i = m_count-1; i >= 0; i--) {
			if (strcmp(m_stack[i], value) == 0) return i;
		}
	}
	return -1;
}

template <>
int32_t CStack<char*>::indexOf(char *const & value) const {
	if (m_count > 0) {
		for (int i = m_count-1; i >= 0; i--) {
			if (strcmp(m_stack[i], value) == 0) return i;
		}
	}
	return -1;
}
