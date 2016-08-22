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

#ifndef JAPPSY_UGLOBJECTDATA_H
#define JAPPSY_UGLOBJECTDATA_H

#include <opengl/uOpenGL.h>
#include <data/uObject.h>
#include <data/uString.h>
#include <data/uVector.h>

class GLRender;

enum GLObjectDataType { NONE, STRING, TEXTURES, SHADER };

class GLObjectData : public CObject {
private:
	GLRender* m_context = NULL;
	
	GLObjectDataType m_type = GLObjectDataType::NONE;
	bool m_reference = false;
	
	CString m_target;
	Vector<GLuint> m_handles;
	
public:
	inline GLObjectData(GLRender* context) { this->m_context = context; }
	~GLObjectData();
	
	GLObjectData* setTarget(const CString& target);
	GLObjectData* setTextures(const Vector<GLuint>& handles, bool reference);
	GLObjectData* setShader(const GLuint handle, bool reference);
	
	inline bool isReference() { return m_type == GLObjectDataType::STRING; }
	inline const CString& getTarget() { return m_target; }
	inline GLuint getShader() { return m_handles[0]; }
	inline Vector<GLuint>& getTextures() { return m_handles; }
};

#endif //JAPPSY_UGLOBJECTDATA_H