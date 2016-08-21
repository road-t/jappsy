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
#include <opengl/uGLObject.h>

class GLRender;

enum GLTextureMapping {
	UVMAP_DEFAULT = 0,
	UVMAP_PLANAR = 1,
	UVMAP_BOX = 2,
	UVMAP_CYLINDER = 3,
	UVMAP_SPHERE = 4
};

class RefGLMaterialTexture : public JRefObject {
public:
	GLRender* context = NULL;
	GLObjectData* texture = NULL;
	GLuint texture1iv = 0;
	
	GLfloat x = 0.0;
	GLfloat y = 0.0;
	GLfloat w = 1.0;
	GLfloat h = 1.0;
	
	GLTextureMapping uvmap = GLTextureMapping::UVMAP_DEFAULT;
	
	inline RefGLMaterialTexture() { throw eInvalidParams; }
	RefGLMaterialTexture(GLRender* context, const JString& textureReference) throw(const char*);
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

class GLMaterialTexture : public JObject {
public:
	JRefClass(GLMaterialTexture, RefGLMaterialTexture);
	
	inline GLMaterialTexture(GLRender* context, const JString& textureReference) throw(const char*) {
		RefGLMaterialTexture* o = new RefGLMaterialTexture(context, textureReference);
		if (o == NULL) throw eOutOfMemory;
		THIS.setRef(o);
	}
	
	inline bool checkReady() throw(const char*) { return THIS.ref().checkReady(); }
};

//===============================

class RefGLMaterial : public JRefObject {
public:
	JString name;
	
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
		GLfloat blend = 0.0;
	} diffuse;
	
	// Specular color is the color of the light of a specular reflection.
	// Specular reflection is the type of reflection that is characteristic of light reflected from a shiny surface.
	// 3DSMax - Specular
	struct {
		Vec3 color = {1.0, 1.0, 1.0};
		GLMaterialTexture texture;
		GLfloat blend = 0.0;
	} specular;
	
	// Emissive color is the self-illumination color an object has.
	// 3DSMax - Self-Illumination
	struct {
		Vec3 color = {0.0, 0.0, 0.0};
		GLMaterialTexture texture;
		GLfloat blend = 0.0;
	} emissive;
	
	// 3DSMax - (value) Glossiness, (percent) Specular Level, (texture alpha channel) Specular Level Blend
	struct {
		GLfloat value = 50;
		GLfloat percent = 1;
		GLMaterialTexture texture;
		GLfloat blend = 0.0;
	} shininess;
	
	// 3DSMax - Opacity
	struct {
		GLfloat value = 1;
		GLMaterialTexture texture;
		GLfloat blend = 0.0;
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
	RefGLMaterial(const JString& name);
	RefGLMaterial(const JString& name, const GLMaterialTexture& texture);
	~RefGLMaterial();

	void update();
};

class GLMaterial : public JObject {
public:
	JRefClass(GLMaterial, RefGLMaterial);
	
	inline GLMaterial(const JString& name) throw(const char*) {
		RefGLMaterial* o = new RefGLMaterial(name);
		if (o == NULL) throw eOutOfMemory;
		THIS.setRef(o);
	}
	
	inline GLMaterial(const JString& name, const GLMaterialTexture& texture) throw(const char*) {
		RefGLMaterial* o = new RefGLMaterial(name, texture);
		if (o == NULL) throw eOutOfMemory;
		THIS.setRef(o);
	}
	
	inline void update() throw(const char*) { THIS.ref().update(); }
};

//===============================

class GLModels;

class ModelJsonParserNodeData {
public:
	JString name;
	Vector<GLfloat> transformation;
	Vector<ModelJsonParserNodeData*> children;
	Vector<GLuint> meshes;
	
	inline void releaseChildren() {
		int32_t count = children.count();
		if (count > 0) {
			ModelJsonParserNodeData** items = children.items();
			for (int i = 0; i < count; i++) {
				memDelete(items[i]);
			}
			children.clear();
		}
	}
	
	inline void release() {
		transformation.clear();
		releaseChildren();
		meshes.clear();
	}
	
	inline ModelJsonParserNodeData() {
		transformation.growstep(16);
	}
	
	inline ~ModelJsonParserNodeData() {
		release();
	}
};

class ModelJsonParserMeshData {
public:
	JString name;
	GLuint materialindex;
	Vector<GLfloat> vertices;
	Vector<GLfloat> normals;
	Vector<Vector<GLfloat>*> texturecoords;
	Vector<GLshort> faces;
	
	inline void releaseTexturecoords() {
		int32_t count = texturecoords.count();
		if (count > 0) {
			Vector<GLfloat>** items = texturecoords.items();
			for (int i = 0; i < count; i++) {
				memDelete(items[i]);
			}
			texturecoords.clear();
		}
	}
	
	inline void release() {
		vertices.clear();
		normals.clear();
		releaseTexturecoords();
		faces.clear();
	}
	
	inline ~ModelJsonParserMeshData() {
		releaseTexturecoords();
	}
};

class ModelJsonParserMaterialData {
public:
	GLModels* models;
	RefGLMaterial* material;
	
	JString key;
	GLuint semantic = 0;
	Vector<GLfloat> value;
};

class ModelJsonParser {
public:
	GLModels* models;
	JString key;
	
	ModelJsonParserNodeData* rootnode = NULL;
	Vector<ModelJsonParserMeshData*> meshes;
	Vector<GLMaterial*> materials;
	
	inline void releaseRootNode() {
		if (rootnode != NULL) {
			memDelete(rootnode);
			rootnode = NULL;
		}
	}
	
	inline void releaseMeshes() {
		int32_t count = meshes.count();
		if (count > 0) {
			ModelJsonParserMeshData** items = meshes.items();
			for (int i = 0; i < count; i++) {
				memDelete(items[i]);
			}
			meshes.clear();
		}
	}
	
	inline void releaseMaterials() {
		int32_t count = materials.count();
		if (count > 0) {
			GLMaterial** items = materials.items();
			for (int i = 0; i < count; i++) {
				delete items[i];
			}
			meshes.clear();
		}
	}
	
	inline void release() {
		releaseRootNode();
		releaseMeshes();
		releaseMaterials();
	}
	
	inline ~ModelJsonParser() {
		release();
	}
};

//===============================

class GLModelNode;
class GLModelMesh;
class GLModel;

class RefGLModelNode : public JRefObject {
public:
	JString name;
	Mat4 transformation;
	
	List<GLModelNode> nodes = null;
	List<GLModelMesh> meshes = null;
	
	inline RefGLModelNode() { Mat4Identity(transformation.v); }
	inline RefGLModelNode(const JString& name, const Mat4& transformation) { this->name = name; Mat4SetV(this->transformation.v, transformation.v); }
	~RefGLModelNode();
	
	GLModelNode& insertNode(const GLModelNode& node) throw(const char*);
	GLModelMesh& insertMesh(const GLModelMesh& mesh) throw(const char*);
	
	void parseJson(ModelJsonParserNodeData* node, Vector<ModelJsonParserMeshData*>& meshes, Mat4& rootmat);
	
	void render(GLRender* context, GLModel& model) const;
};

class GLModelNode : public JObject {
public:
	JRefClass(GLModelNode, RefGLModelNode);

	inline GLModelNode& insertNode(const GLModelNode& node) throw(const char*) { return THIS.ref().insertNode(node); }
	inline GLModelMesh& insertMesh(const GLModelMesh& mesh) throw(const char*) { return THIS.ref().insertMesh(mesh); }
	
	inline void render(GLRender* context, GLModel& model) const throw(const char*) { THIS.ref().render(context, model); }
};

class RefGLModelMesh : public JRefObject {
public:
	JString name;
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
	
	inline RefGLModelMesh(const JString& name, GLuint materialIndex, GLuint vertexBuffer, GLuint normalsBuffer, GLuint textureBuffer, GLuint indexBuffer, GLuint indexCount) {
		THIS.name = name;
		THIS.material = materialIndex;
		THIS.vertices = vertexBuffer;
		THIS.normals = normalsBuffer;
		THIS.texturecoords = textureBuffer;
		THIS.indexes = indexBuffer;
		THIS.indexCount = indexCount;
	}
	
	~RefGLModelMesh();
	
	void parseJson(ModelJsonParserMeshData* mesh, Mat4& matN, Mat4& matV);
	
	void render(GLRender* context, GLModel& model) const;
};

class GLModelMesh : public JObject {
public:
	JRefClass(GLModelMesh, RefGLModelMesh);

	inline void render(GLRender* context, GLModel& model) const throw(const char*) { THIS.ref().render(context, model); }
};

//===============================

class RefGLModel : public JRefObject {
public:
	GLRender* context = NULL;
	List<GLMaterial> materials;
	GLModelNode rootnode;
	
	inline RefGLModel() { throw eInvalidParams; }
	RefGLModel(GLRender* context);
	~RefGLModel();
	
	GLuint createMaterial(const JString& key, const GLfloat x, const GLfloat y, const GLfloat w, const GLfloat h, GLTextureMapping uvmap);
	GLuint insertMaterial(GLMaterial& material);
	
	void parseJson(ModelJsonParser* parser) throw(const char*);
	
	void render(const GLObject& object);
};

class GLModel : public JObject {
public:
	JRefClass(GLModel, RefGLModel)

	inline GLuint createMaterial(const JString& key, const GLfloat x, const GLfloat y, const GLfloat w, const GLfloat h, GLTextureMapping uvmap) throw(const char*) { return THIS.ref().createMaterial(key, x, y, w, h, uvmap); }
	inline GLuint insertMaterial(GLMaterial& material) throw(const char*) { return THIS.ref().insertMaterial(material); }
};

//===============================

class GLModels {
private:
	GLRender* context;
	HashMap<JString, GLModel> list;
	
public:
	GLModels(GLRender* context) throw(const char*);
	~GLModels();
	
	GLModel& get(const JString& key) throw(const char*);
	GLModel& createModel(const JString& key) throw(const char*);

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
				// mesh.vertices
				static void onjson_vertices_start(struct json_context* ctx, const char* key, void* target);
				static void onjson_vertices_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror);
					// mesh.vertices[index]
					static void onjson_vertices_number(struct json_context* ctx, const int index, const struct json_number& number, void* target);
				// mesh.normals
				static void onjson_normals_start(struct json_context* ctx, const char* key, void* target);
				static void onjson_normals_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror);
					// mesh.normals[index]
					static void onjson_normals_number(struct json_context* ctx, const int index, const struct json_number& number, void* target);
				// mesh.texturecoords
				static void onjson_texturecoords_start(struct json_context* ctx, const char* key, void* target);
				static void onjson_texturecoords_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror);
					// mesh.texturecoords[index]
					static void onjson_coords_start(struct json_context* ctx, const int index, void* target);
					static void onjson_coords_end(struct json_context* ctx, const int index, void* parenttarget, void* target, bool noerror);
						// mesh.texturecoords[index][index]
						static void onjson_coords_number(struct json_context* ctx, const int index, const struct json_number& number, void* target);
				// mesh.faces
				static void onjson_faces_start(struct json_context* ctx, const char* key, void* target);
				static void onjson_faces_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror);
					// mesh.faces[index]
					static void onjson_triangle_start(struct json_context* ctx, const int index, void* target);
					static void onjson_triangle_end(struct json_context* ctx, const int index, void* parenttarget, void* target, bool noerror);
						// mesh.faces[index][index]
						static void onjson_triangle_number(struct json_context* ctx, const int index, const struct json_number& number, void* target);
		// root.materials
		static void onjson_materials_start(struct json_context* ctx, const char* key, void* target);
		static void onjson_materials_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror);
			// root.materials[index]
			static void onjson_material_start(struct json_context* ctx, const int index, void* target);
			static void onjson_material_end(struct json_context* ctx, const int index, void* parenttarget, void* target, bool noerror);
			static void onjson_material_array_start(struct json_context* ctx, const char* key, void* target);
				// material.properties
				static void onjson_material_properties_start(struct json_context* ctx, const char* key, void* target);
				static void onjson_material_properties_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror);
					// material.properties[index]
					static void onjson_material_property_start(struct json_context* ctx, const int index, void* target);
					static void onjson_material_property_end(struct json_context* ctx, const int index, void* parenttarget, void* target, bool noerror);
					static void onjson_material_property_string(struct json_context* ctx, const char* key, char* value, void* target);
					static void onjson_material_property_number(struct json_context* ctx, const char* key, const struct json_number& number, void* target);
					static void onjson_material_property_array_start(struct json_context* ctx, const char* key, void* target);
						// material.properties[index].value
						static void onjson_material_property_value_start(struct json_context* ctx, const char* key, void* target);
						static void onjson_material_property_value_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror);
						static void onjson_material_property_value_number(struct json_context* ctx, const int index, const struct json_number& number, void* target);
	
private: // Thread Safe
	static void* CreateJsonModelCallback(void* threadData);

public: // Thread Safe
	
	GLModel& createModel(const JString& key, const char* jsonModelSource) throw(const char*);
};

#endif //JAPPSY_UGLMODEL_H