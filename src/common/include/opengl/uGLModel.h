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

#ifndef JAPPSY_UGLMODEL_H
#define JAPPSY_UGLMODEL_H

#include <opengl/uOpenGL.h>
#include <data/uObject.h>
#include <data/uHashMap.h>
#include <data/uString.h>
#include <data/uVector.h>

class GLRender;

enum GLTextureMapping {
	UVMAP_DEFAULT = 0,
	UVMAP_PLANAR = 1,
	UVMAP_BOX = 2,
	UVMAP_CYLINDER = 3,
	UVMAP_SPHERE = 4
};

class GLMaterialTexture {
public:
	String file;
	GLfloat blend = 0.0;
	GLuint texture1iv;
	
	GLfloat x = 0.0;
	GLfloat y = 0.0;
	GLfloat w = 1.0;
	GLfloat h = 1.0;
	
	GLTextureMapping uvmap = GLTextureMapping::UVMAP_DEFAULT;
	
	inline ~GLMaterialTexture() {
		file = null;
		texture1iv = 0;
	}
	
	GLMaterialTexture& texture(GLRender* context, const String& key);
	
	inline GLMaterialTexture& map(GLfloat x, GLfloat y, GLfloat w, GLfloat h, GLTextureMapping uvmap) {
		THIS.x = x;
		THIS.y = y;
		THIS.w = w;
		THIS.h = h;
		THIS.uvmap = uvmap;
		return *this;
	}
};

class GLMaterialSimpleColor {
public:
	Vec3 color;
	
	inline GLMaterialSimpleColor(GLfloat color[3]) { memcpy(THIS.color.v, color, 3 * sizeof(GLfloat)); }
};

class GLMaterialColor : public GLMaterialSimpleColor {
public:
	GLMaterialTexture* texture = NULL;
	
	inline GLMaterialColor(GLfloat color[3], GLMaterialTexture* texture) : GLMaterialSimpleColor(color) { THIS.texture = texture; }
	inline ~GLMaterialColor() { if (texture != NULL) { memDelete(texture); texture = NULL; } }
};

class GLMaterial {
private:
	String name;
	
	// Ambient color is the color of an object where it is in shadow.
	// This color is what the object reflects when illuminated by ambient light rather than direct light.
	// 3DSMax - Ambient
	GLMaterialSimpleColor* ambient = NULL;
	
	// Diffuse color is the most instinctive meaning of the color of an object.
	// It is that essential color that the object reveals under pure white light.
	// It is perceived as the color of the object itself rather than a reflection of the light.
	// 3DSMax - Diffuse
	GLMaterialColor* diffuse = NULL;
	
	// Specular color is the color of the light of a specular reflection.
	// Specular reflection is the type of reflection that is characteristic of light reflected from a shiny surface.
	// 3DSMax - Specular
	GLMaterialColor* specular = NULL;
	
	// Emissive color is the self-illumination color an object has.
	// 3DSMax - Self-Illumination
	GLMaterialColor* emissive;
	
public:
	GLMaterial(const String& name, GLMaterialTexture* texture);
};

class RefGLModel : public RefObject {
public:
	GLRender* context = NULL;
	
	inline RefGLModel() { throw eInvalidParams; }
	RefGLModel(GLRender* context);
	~RefGLModel();
};

class GLModel : public Object {
public:
	RefClass(GLModel, RefGLModel)
};

class GLModels {
private:
	GLRender* context;
	HashMap<String, GLModel> list;
	
public:
	GLModels(GLRender* context) throw(const char*);
	~GLModels();
	
	GLModel& get(const String& key) throw(const char*);
	GLModel& createModel(const String& key, const char* json) throw(const char*);
};

#endif //JAPPSY_UGLMODEL_H