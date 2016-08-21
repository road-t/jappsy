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

#include "uGLObjectData.h"
#include <opengl/uGLRender.h>

GLObjectData::~GLObjectData() {
	if (!m_reference) {
		if (m_type == GLObjectDataType::TEXTURES) {
			uint32_t count = m_handles.count();
			if (count > 0) {
				GLuint* items = m_handles.items();
				for (int i = 0; i < count; i++) {
					m_context->textures->releaseTextureHandle(items[i]);
				}
			}
		} else if (m_type == GLObjectDataType::SHADER) {
			m_context->shaders->releaseShader(m_handles[0]);
		}
	}
	m_target = null;
	m_handles.clear();
	m_reference = false;
	m_type = GLObjectDataType::NONE;
}

GLObjectData& GLObjectData::setTarget(const JString& target) {
	m_target = target;
	m_handles.clear();
	m_reference = false;
	m_type = GLObjectDataType::STRING;
	return THIS;
}

GLObjectData& GLObjectData::setTextures(const Vector<GLuint>& handles, bool reference) {
	m_target = null;
	m_handles.clear();
	uint32_t count = handles.count();
	if (count > 0) {
		GLuint* items = handles.items();
		for (int i = 0; i < count; i++) {
			m_handles.push(items[i]);
		}
	}
	m_reference = reference;
	m_type = GLObjectDataType::TEXTURES;
	return THIS;
}

GLObjectData& GLObjectData::setShader(const GLuint handle, bool reference) {
	m_target = null;
	m_handles.clear();
	m_handles.push(handle);
	m_reference = reference;
	m_type = GLObjectDataType::SHADER;
	return THIS;
}
