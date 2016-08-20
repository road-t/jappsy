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
#include <data/uList.h>
#include <opengl/uGLObjectData.h>

class GLRender;

enum GLTextureMapping {
	UVMAP_DEFAULT = 0,
	UVMAP_PLANAR = 1,
	UVMAP_BOX = 2,
	UVMAP_CYLINDER = 3,
	UVMAP_SPHERE = 4
};

class RefGLMaterialTexture : public RefObject {
public:
	GLRender* context = NULL;
	GLObjectData* texture = NULL;
	GLfloat blend = 0.0;
	GLuint texture1iv = 0;
	
	GLfloat x = 0.0;
	GLfloat y = 0.0;
	GLfloat w = 1.0;
	GLfloat h = 1.0;
	
	GLTextureMapping uvmap = GLTextureMapping::UVMAP_DEFAULT;
	
	inline RefGLMaterialTexture() { throw eInvalidParams; }
	RefGLMaterialTexture(GLRender* context, const String& textureReference) throw(const char*);
	~RefGLMaterialTexture();
	
	inline RefGLMaterialTexture& setUVMap(GLfloat x, GLfloat y, GLfloat w, GLfloat h, GLTextureMapping uvmap) {
		THIS.x = x;
		THIS.y = y;
		THIS.w = w;
		THIS.h = h;
		THIS.uvmap = uvmap;
		return *this;
	}
	
	bool checkReady();
};

class GLMaterialTexture : public Object {
public:
	RefClass(GLMaterialTexture, RefGLMaterialTexture);
	
	inline GLMaterialTexture(GLRender* context, const String& textureReference) throw(const char*) {
		RefGLMaterialTexture* o = new RefGLMaterialTexture(context, textureReference);
		if (o == NULL) throw eOutOfMemory;
		THIS.setRef(o);
	}
	
	inline bool checkReady() throw(const char*) { return THIS.ref().checkReady(); }
};

//===============================

class RefGLMaterial : public RefObject {
private:
	String name;
	
	// Ambient color is the color of an object where it is in shadow.
	// This color is what the object reflects when illuminated by ambient light rather than direct light.
	// 3DSMax - Ambient
	struct {
		Vec3 color = {0.0, 0.0, 0.0};
	} ambient;
	
	// Diffuse color is the most instinctive meaning of the color of an object.
	// It is that essential color that the object reveals under pure white light.
	// It is perceived as the color of the object itself rather than a reflection of the light.
	// 3DSMax - Diffuse
	struct {
		Vec3 color = {1.0, 1.0, 1.0};
		GLMaterialTexture texture;
	} diffuse;
	
	// Specular color is the color of the light of a specular reflection.
	// Specular reflection is the type of reflection that is characteristic of light reflected from a shiny surface.
	// 3DSMax - Specular
	struct {
		Vec3 color = {1.0, 1.0, 1.0};
		GLMaterialTexture texture;
	} specular;
	
	// Emissive color is the self-illumination color an object has.
	// 3DSMax - Self-Illumination
	struct {
		Vec3 color = {0.0, 0.0, 0.0};
		GLMaterialTexture texture;
	} emissive;
	
	// 3DSMax - (value) Glossiness, (percent) Specular Level, (texture alpha channel) Specular Level Blend
	struct {
		GLfloat value = 50;
		GLfloat percent = 1;
		GLMaterialTexture texture;
	} shininess;
	
	// 3DSMax - Opacity
	struct {
		GLfloat value = 1;
		GLMaterialTexture texture;
	} opacity;
	
	// 3DSMax - Bump
	// Not supported
	struct {
		GLfloat scale = 1;
		GLMaterialTexture texture;
	} bump;
	
	// 3DSMax - Reflection
	// Not supported
	struct {
		GLMaterialTexture texture;
	} reflection;
	
public:
	Vector<GLfloat> colors4fv;
	
	RefGLMaterial();
	RefGLMaterial(const String& name);
	RefGLMaterial(const String& name, const GLMaterialTexture& texture);
	
	void release();
	void update();
};

class GLMaterial : public Object {
public:
	RefClass(GLMaterial, RefGLMaterial);
	
	inline void release() throw(const char*) { THIS.ref().release(); }
	inline void update() throw(const char*) { THIS.ref().update(); }
};

//===============================

class GLModelNode;
class GLModelMesh;

class RefGLModelNode : public RefObject {
public:
	String name;
	Mat4 transformation;
	
	List<GLModelNode> nodes = null;
	List<GLModelMesh> meshes = null;
	
	inline RefGLModelNode() { Mat4Identity(transformation.v); }
	inline RefGLModelNode(const String& name, const Mat4& transformation) { this->name = name; Mat4SetV(this->transformation.v, transformation.v); }
	
	GLModelNode& insertNode(const GLModelNode& node) throw(const char*);
	GLModelMesh& insertMesh(const GLModelMesh& mesh) throw(const char*);
	
	void release();
};

class GLModelNode : public Object {
public:
	RefClass(GLModelNode, RefGLModelNode);

	inline void release() throw(const char*) { THIS.ref().release(); }
	inline GLModelNode& insertNode(const GLModelNode& node) throw(const char*) { return THIS.ref().insertNode(node); }
	inline GLModelMesh& insertMesh(const GLModelMesh& mesh) throw(const char*) { return THIS.ref().insertMesh(mesh); }
};

class RefGLModelMesh : public RefObject {
public:
	String name;
	GLuint material;
	GLuint vertices;
	GLuint normals;
	GLuint texturecoords;
	GLuint indexes;
	GLuint indexCount;
	
	inline RefGLModelMesh() {
		name = null;
		material = 0;
		vertices = normals = texturecoords = indexes = indexCount = 0;
	}
	
	inline RefGLModelMesh(const String& name, GLuint materialIndex, GLuint vertexBuffer, GLuint normalsBuffer, GLuint textureBuffer, GLuint indexBuffer, GLuint indexCount) {
		THIS.name = name;
		THIS.material = materialIndex;
		THIS.vertices = vertexBuffer;
		THIS.normals = normalsBuffer;
		THIS.texturecoords = textureBuffer;
		THIS.indexes = indexBuffer;
		THIS.indexCount = indexCount;
	}
	
	void release();
};

class GLModelMesh : public Object {
public:
	RefClass(GLModelMesh, RefGLModelMesh);

	inline void release() throw(const char*) { THIS.ref().release(); }
};

//===============================

class RefGLModel : public RefObject {
public:
	GLRender* context = NULL;
	List<GLMaterial> materials;
	GLModelNode rootnode;
	
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
	GLModel& createModel(const String& key) throw(const char*);

private:
	// json parser callbacks
	// root
	static void onjson_root_start(struct json_context* ctx, void* target);
	static void onjson_root_end(struct json_context* ctx, void* target, bool noerror);
	static void onjson_root_object_start(struct json_context* ctx, const char* key, void* target);
		// root.rootnode
		static void onjson_rootnode_start(struct json_context* ctx, const char* key, void* target);
		static void onjson_rootnode_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror);
		static void onjson_node_string(struct json_context* ctx, const char* key, char* value, void* target);
		static void onjson_node_array_start(struct json_context* ctx, const char* key, void* target);
			// node.transformation
			static void onjson_node_tranformation_start(struct json_context* ctx, const char* key, void* target);
			static void onjson_node_transformation_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror);
				// node.transformation[index]
				static void onjson_node_transformation_number(struct json_context* ctx, const int index, const struct json_number& number, void* target);
			// node.children
			static void onjson_node_children_start(struct json_context* ctx, const char* key, void* target);
			static void onjson_node_children_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror);
				// node.children[index]
				static void onjson_node_start(struct json_context* ctx, const int index, void* target);
				static void onjson_node_end(struct json_context* ctx, const int index, void* parenttarget, void* target, bool noerror);
			// node.meshes
			static void onjson_node_meshes_start(struct json_context* ctx, const char* key, void* target);
			static void onjson_node_meshes_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror);
				// node.meshes[index]
				static void onjson_node_meshes_number(struct json_context* ctx, const int index, const struct json_number& number, void* target);
	static void onjson_root_array_start(struct json_context* ctx, const char* key, void* target);
		// root.meshes
		static void onjson_meshes_start(struct json_context* ctx, const char* key, void* target);
		static void onjson_meshes_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror);
	
			// root.meshes[index]
			static void onjson_mesh_start(struct json_context* ctx, const int index, void* target);
			static void onjson_mesh_end(struct json_context* ctx, const int index, void* parenttarget, void* target, bool noerror);
			static void onjson_mesh_string(struct json_context* ctx, const char* key, char* value, void* target);
			static void onjson_mesh_number(struct json_context* ctx, const char* key, const struct json_number& number, void* target);
			static void onjson_mesh_array_start(struct json_context* ctx, const char* key, void* target); // vertices | normals | texturecoords | faces
	
	
		// root.materials
		static void onjson_materials_start(struct json_context* ctx, const char* key, void* target);
		static void onjson_materials_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror);

	/*
	// root.rootnode | node parser

	// node.transformation
	static void onjson_object_node_array_transformation_number(struct json_context* ctx, const int index, const struct json_number& number, void* target); // node.transformation[index]
	static void onjson_object_node_array_transformation_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror);

	// node.children
	static void onjson_object_node_array_children_object_start(struct json_context* ctx, const int index, void* target); // childnode
	static void onjson_object_node_array_children_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror);

	// node.meshes
	static void onjson_object_node_array_meshes_number(struct json_context* ctx, const int index, const struct json_number& number, void* target); // node.meshes[index]
	static void onjson_object_node_array_meshes_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror);
	
	// root.meshes parser
	static void onjson_array_meshes_object_start(struct json_context* ctx, const int index, void* target); // mesh
	static void onjson_array_meshes_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror);
	
	// root.materials parser
	static void onjson_array_materials_object_start(struct json_context* ctx, const int index, void* target); // material
	static void onjson_array_materials_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror);
	 */
	
public: // Thread Safe
	
	GLModel& createModel(const String& key, const char* jsonModelSource) throw(const char*);
};

#endif //JAPPSY_UGLMODEL_H