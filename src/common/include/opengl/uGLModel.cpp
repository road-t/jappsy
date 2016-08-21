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

#include "uGLModel.h"
#include <opengl/uGLRender.h>
#include <core/uMemory.h>

RefGLMaterialTexture::~RefGLMaterialTexture() {
	if (texture != NULL) {
		memDelete(texture);
		texture = NULL;
	}
	
	texture1iv = 0;
	context = NULL;
}

RefGLMaterialTexture::RefGLMaterialTexture(GLRender* context, const String& textureReference) throw(const char*) {
	texture1iv = 0;

	if (textureReference == null) {
		if (texture != NULL) {
			memDelete(texture);
			texture = NULL;
		}
	} else {
		try {
			texture = memNew(texture, GLObjectData(context));
			if (texture == NULL)
				throw eOutOfMemory;
			texture->setTarget(textureReference);
		} catch (...) {
			if (texture != NULL) {
				memDelete(texture);
				texture = NULL;
			}
			throw;
		}
	}
}

bool RefGLMaterialTexture::checkReady() {
	if ((THIS.texture != NULL) && (THIS.texture->isReference())) {
		GLTexture* texture;
		try {
			texture = &(context->textures->get(THIS.texture->getTarget()));
		} catch (...) {
			return false;
		}
		
		Vector<GLuint>* textures = &(texture->ref().handles);
		THIS.texture->setTextures(*textures, true);
		texture1iv = textures->get(0);
	}

	return true;
}

//===============================

RefGLMaterial::RefGLMaterial() {
	name = L"NONAME";
}

RefGLMaterial::RefGLMaterial(const String& name) {
	THIS.name = name;
}

RefGLMaterial::RefGLMaterial(const String& name, const GLMaterialTexture& texture) {
	THIS.name = name;
	
	diffuse.texture = texture;
}

void RefGLMaterial::release() {
	name = null;
	diffuse.texture = null;
	specular.texture = null;
	emissive.texture = null;
	shininess.texture = null;
	opacity.texture = null;
	bump.texture = null;
	colors4fv.clear();
}

void RefGLMaterial::update() {
	colors4fv.resize(20);
	GLfloat *items = colors4fv.items();
	
	Vec3SetV(items + 0, ambient.color.v);
	items[3] = opacity.value;
	
	Vec3SetV(items + 4, diffuse.color.v);
	items[7] = (diffuse.texture == null) ? 0 : diffuse.blend;
	
	Vec3SetV(items + 8, specular.color.v);
	items[11] = (specular.texture == null) ? 0 : specular.blend;
	
	Vec3SetV(items + 12, emissive.color.v);
	items[15] = (emissive.texture == null) ? 0 : emissive.blend;
	
	items[16] = shininess.value;
	items[17] = shininess.percent;
	items[18] = 0.0; // unused
	items[19] = (opacity.texture == null) ? 0 : opacity.blend;
}

//===============================

void RefGLModelNode::release() {
	name = null;
	
	if (nodes != null) {
		Iterator<GLModelNode> it = nodes.iterator();
		while (it.hasNext()) {
			(*(GLModelNode*)&(it.next())).release();
		}
		nodes.clear();
		nodes = null;
	}
	
	if (meshes != null) {
		Iterator<GLModelMesh> it = meshes.iterator();
		while (it.hasNext()) {
			(*(GLModelMesh*)&(it.next())).release();
		}
		meshes.clear();
		meshes = null;
	}
}

GLModelNode& RefGLModelNode::insertNode(const GLModelNode& node) throw(const char*) {
	if (meshes == null) {
		if (nodes == null) {
			nodes = new List<GLModelNode>();
		}
		return nodes.push(node);
	}
	throw eInvalidParams;
}

GLModelMesh& RefGLModelNode::insertMesh(const GLModelMesh& mesh) throw(const char*) {
	if (nodes == null) {
		if (meshes == null) {
			meshes = new List<GLModelMesh>();
		}
		return meshes.push(mesh);
	}
	throw eInvalidParams;
}

void RefGLModelMesh::release() {
	name = null;
	material = 0;
	
	uint8_t nullData = 0;
	
	glBindBuffer(GL_ARRAY_BUFFER, vertices);
	glBufferData(GL_ARRAY_BUFFER, 1, &nullData, GL_STATIC_DRAW);
	glDeleteBuffers(1, &vertices);
	vertices = 0;
	
	glBindBuffer(GL_ARRAY_BUFFER, normals);
	glBufferData(GL_ARRAY_BUFFER, 1, &nullData, GL_STATIC_DRAW);
	glDeleteBuffers(1, &normals);
	normals = 0;
	
	glBindBuffer(GL_ARRAY_BUFFER, texturecoords);
	glBufferData(GL_ARRAY_BUFFER, 1, &nullData, GL_STATIC_DRAW);
	glDeleteBuffers(1, &texturecoords);
	texturecoords = 0;
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexes);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 1, &nullData, GL_STATIC_DRAW);
	glDeleteBuffers(1, &indexes);
	indexes = 0;
}

//===============================

RefGLModel::RefGLModel(GLRender* context) {
	THIS.context = context;
}

RefGLModel::~RefGLModel() {
	THIS.context = NULL;
}

GLModels::GLModels(GLRender* context) throw(const char*) {
	THIS.context = context;
	list = new HashMap<String, GLModel>();
}

GLModels::~GLModels() {
	list = null;
	context = NULL;
}

GLModel& GLModels::get(const String& key) throw(const char*) {
	return (GLModel&)list.get(key);
}

GLModel& GLModels::createModel(const String& key) throw(const char*) {
	try {
		list.remove(key);
		GLModel* shader = &(list.put(key, new RefGLModel(context)));
		//		if (wcscmp(key, L"null") == 0) {
		//		}
		return *shader;
	} catch (...) {
		throw;
	}
}

//===============================

class ModelJsonParserNodeData {
public:
	String name;
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
	String name;
	GLuint materialindex;
	Vector<GLfloat> vertices;
	Vector<GLfloat> normals;
	Vector<Vector<GLfloat>*> texturecoords;
	Vector<GLfloat> faces;
	
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

class ModelJsonParser {
public:
	GLModels* models;
	
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

class ModelJsonParserMaterialData {
public:
	GLModels* models;
	RefGLMaterial* material;
	
	String key;
	GLuint semantic = 0;
	Vector<GLfloat> value;
};

#define ModelLog(fmt, ...) String::format(fmt, ## __VA_ARGS__).log()

void GLModels::onjson_root_start(struct json_context* ctx, void* target) {
	//ModelJsonParser* parser = (ModelJsonParser*)target;
	ModelLog(L"Model Parser");
	
	ctx->callbacks->onobject.onobjectstart = onjson_root_object_start;
	ctx->callbacks->onobject.onarraystart = onjson_root_array_start;
}

void GLModels::onjson_root_end(struct json_context* ctx, void* target, bool noerror) {
	ModelJsonParser* parser = (ModelJsonParser*)target;

	if (!noerror) {
		parser->release();
	}
}

void GLModels::onjson_root_object_start(struct json_context* ctx, const char* key, void* target) {
	//ModelJsonParser* parser = (ModelJsonParser*)target;

	json_clear_callbacks(ctx->callbacks, target);
	if (strcmp(key, "rootnode") == 0) {
		onjson_rootnode_start(ctx, key, target);
	}
}

void GLModels::onjson_rootnode_start(struct json_context* ctx, const char* key, void* target) {
	//ModelJsonParser* parser = (ModelJsonParser*)target;
	
	ModelLog(L"model.rootnode");
	ModelJsonParserNodeData* data = memNew(data, ModelJsonParserNodeData());
	if (data == NULL)
		throw eOutOfMemory;
	
	ctx->callbacks->target = data;
	ctx->callbacks->onobject.onstring = onjson_node_string;
	ctx->callbacks->onobject.onarraystart = onjson_node_array_start;
	ctx->callbacks->onobject.onobjectend = onjson_rootnode_end;
}

void GLModels::onjson_rootnode_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror) {
	ModelJsonParserNodeData* data = (ModelJsonParserNodeData*)target;
	if ((noerror) && ((data->meshes.count() > 0) || (data->children.count() > 0))) {
		ModelJsonParser* parser = (ModelJsonParser*)parenttarget;
		parser->rootnode = data;
	} else {
		memDelete(data);
	}
}

void GLModels::onjson_node_string(struct json_context* ctx, const char* key, char* value, void* target) {
	ModelJsonParserNodeData* data = (ModelJsonParserNodeData*)target;
	
	if (strcmp(key, "name") == 0) {
		ModelLog(L"node.name: %s", value);
		
		data->name = value;
	}
}

void GLModels::onjson_node_array_start(struct json_context* ctx, const char* key, void* target) {
	//ModelJsonParserNodeData* data = (ModelJsonParserNodeData*)target;
	
	json_clear_callbacks(ctx->callbacks, target);
	if (strcmp(key, "transformation") == 0) {
		onjson_node_tranformation_start(ctx, key, target);
	} else if (strcmp(key, "children") == 0) {
		onjson_node_children_start(ctx, key, target);
	} else if (strcmp(key, "meshes") == 0) {
		onjson_node_meshes_start(ctx, key, target);
	}
}

void GLModels::onjson_node_tranformation_start(struct json_context* ctx, const char* key, void* target) {
	//ModelJsonParserNodeData* data = (ModelJsonParserNodeData*)target;
	
	ModelLog(L"node.transformation");
	
	ctx->callbacks->onarray.onnumber = onjson_node_transformation_number;
	ctx->callbacks->onobject.onarrayend = onjson_node_transformation_end;
}

void GLModels::onjson_node_transformation_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror) {
	ModelJsonParserNodeData* data = (ModelJsonParserNodeData*)target;
	
	if ((!noerror) || (data->transformation.count() != 16)) {
		data->transformation.clear();
	}
}

void GLModels::onjson_node_transformation_number(struct json_context* ctx, const int index, const struct json_number& number, void* target) {
	ModelJsonParserNodeData* data = (ModelJsonParserNodeData*)target;

	if (number.is_float) {
		data->transformation.push(number.v.f);
	} else {
		data->transformation.push(number.v.i);
	}
}

void GLModels::onjson_node_children_start(struct json_context* ctx, const char* key, void* target) {
	//ModelJsonParserNodeData* data = (ModelJsonParserNodeData*)target;
	
	ModelLog(L"node.children");
	
	ctx->callbacks->onarray.onobjectstart = onjson_node_start;
	ctx->callbacks->onobject.onarrayend = onjson_node_children_end;
}

void GLModels::onjson_node_children_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror) {
	ModelJsonParserNodeData* data = (ModelJsonParserNodeData*)target;
	
	if (!noerror) {
		data->releaseChildren();
	}
}

void GLModels::onjson_node_start(struct json_context* ctx, const int index, void* target) {
	//ModelJsonParserNodeData* data = (ModelJsonParserNodeData*)target;
	
	ModelLog(L"node.children[%d]", index);
	ModelJsonParserNodeData* data = memNew(data, ModelJsonParserNodeData());
	if (data == NULL)
		throw eOutOfMemory;
	
	json_clear_callbacks(ctx->callbacks, data);
	ctx->callbacks->onobject.onstring = onjson_node_string; // name
	ctx->callbacks->onobject.onarraystart = onjson_node_array_start; // transformation | children
	ctx->callbacks->onarray.onobjectend = onjson_node_end;
}

void GLModels::onjson_node_end(struct json_context* ctx, const int index, void* parenttarget, void* target, bool noerror) {
	ModelJsonParserNodeData* data = (ModelJsonParserNodeData*)target;
	if ((noerror) && ((data->meshes.count() > 0) || (data->children.count() > 0))) {
		ModelJsonParserNodeData* parentdata = (ModelJsonParserNodeData*)parenttarget;
		parentdata->children.push(data);
	} else {
		memDelete(data);
	}
}

void GLModels::onjson_node_meshes_start(struct json_context* ctx, const char* key, void* target) {
	//ModelJsonParserNodeData* data = (ModelJsonParserNodeData*)target;
	
	ModelLog(L"node.meshes");
	
	ctx->callbacks->onarray.onnumber = onjson_node_meshes_number;
	ctx->callbacks->onobject.onarrayend = onjson_node_meshes_end;
}

void GLModels::onjson_node_meshes_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror) {
	ModelJsonParserNodeData* data = (ModelJsonParserNodeData*)target;
	
	if (!noerror) {
		data->meshes.clear();
	}
}

void GLModels::onjson_node_meshes_number(struct json_context* ctx, const int index, const struct json_number& number, void* target) {
	ModelJsonParserNodeData* data = (ModelJsonParserNodeData*)target;
	
	if (number.is_float) {
		data->meshes.push(number.v.f);
	} else {
		data->meshes.push(number.v.i);
	}
}

void GLModels::onjson_root_array_start(struct json_context* ctx, const char* key, void* target) {
	ModelJsonParser* parser = (ModelJsonParser*)target;
	
	json_clear_callbacks(ctx->callbacks, target);
	if (strcmp(key, "meshes") == 0) {
		onjson_meshes_start(ctx, key, target);
	} else if (strcmp(key, "materials") == 0) {
		onjson_materials_start(ctx, key, target);
	}
}

void GLModels::onjson_meshes_start(struct json_context* ctx, const char* key, void* target) {
	ModelJsonParser* parser = (ModelJsonParser*)target;

	ModelLog(L"model.meshes");
	
	ctx->callbacks->onarray.onobjectstart = onjson_mesh_start;
	ctx->callbacks->onobject.onarrayend = onjson_meshes_end;
}

void GLModels::onjson_meshes_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror) {
	ModelJsonParser* parser = (ModelJsonParser*)target;
	
	if (!noerror) {
		parser->releaseMeshes();
	}
}

void GLModels::onjson_mesh_start(struct json_context* ctx, const int index, void* target) {
	//ModelJsonParserNodeData* data = (ModelJsonParserNodeData*)target;
	
	ModelLog(L"model.meshes[%d]", index);
	ModelJsonParserMeshData* data = memNew(data, ModelJsonParserMeshData());
	if (data == NULL)
		throw eOutOfMemory;
	
	json_clear_callbacks(ctx->callbacks, data);
	ctx->callbacks->onobject.onstring = onjson_mesh_string;
	ctx->callbacks->onobject.onnumber = onjson_mesh_number;
	ctx->callbacks->onobject.onarraystart = onjson_mesh_array_start;
	ctx->callbacks->onarray.onobjectend = onjson_mesh_end;
}

void GLModels::onjson_mesh_end(struct json_context* ctx, const int index, void* parenttarget, void* target, bool noerror) {
	ModelJsonParserMeshData* data = (ModelJsonParserMeshData*)target;
	if ((noerror) && (data->vertices.count() > 0) && (data->normals.count() > 0) && (data->faces.count() > 0)) {
		ModelJsonParser* parser = (ModelJsonParser*)parenttarget;
		parser->meshes.push(data);
	} else {
		memDelete(data);
	}
}

void GLModels::onjson_mesh_string(struct json_context* ctx, const char* key, char* value, void* target) {
	ModelJsonParserMeshData* data = (ModelJsonParserMeshData*)target;
	
	if (strcmp(key, "name") == 0) {
		data->name = value;
	}
}

void GLModels::onjson_mesh_number(struct json_context* ctx, const char* key, const struct json_number& number, void* target) {
	ModelJsonParserMeshData* data = (ModelJsonParserMeshData*)target;

	if (strcmp(key, "materialindex") == 0) {
		if (number.is_float) {
			data->materialindex = number.v.f;
		} else {
			data->materialindex = number.v.i;
		}
	}
}

void GLModels::onjson_mesh_array_start(struct json_context* ctx, const char* key, void* target) {
	ModelJsonParserMeshData* data = (ModelJsonParserMeshData*)target;

	json_clear_callbacks(ctx->callbacks, target);
	if (strcmp(key, "vertices") == 0) {
		onjson_vertices_start(ctx, key, target);
	} else if (strcmp(key, "normals") == 0) {
		onjson_normals_start(ctx, key, target);
	} else if (strcmp(key, "texturecoords") == 0) {
		onjson_texturecoords_start(ctx, key, target);
	} else if (strcmp(key, "faces") == 0) {
		onjson_faces_start(ctx, key, target);
	}
}

void GLModels::onjson_vertices_start(struct json_context* ctx, const char* key, void* target) {
	ModelJsonParserMeshData* data = (ModelJsonParserMeshData*)target;

	ModelLog(L"mesh.vertices");
	
	data->vertices.growstep(128);
	ctx->callbacks->onarray.onnumber = onjson_vertices_number;
	ctx->callbacks->onobject.onarrayend = onjson_vertices_end;
}

void GLModels::onjson_vertices_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror) {
	ModelJsonParserMeshData* data = (ModelJsonParserMeshData*)target;

	if (!noerror) {
		data->vertices.clear();
	}
}

void GLModels::onjson_vertices_number(struct json_context* ctx, const int index, const struct json_number& number, void* target) {
	ModelJsonParserMeshData* data = (ModelJsonParserMeshData*)target;
	
	if (number.is_float) {
		data->vertices.push(number.v.f);
	} else {
		data->vertices.push(number.v.i);
	}
}

void GLModels::onjson_normals_start(struct json_context* ctx, const char* key, void* target) {
	ModelJsonParserMeshData* data = (ModelJsonParserMeshData*)target;

	ModelLog(L"mesh.normals");
	
	data->normals.growstep(128);
	ctx->callbacks->onarray.onnumber = onjson_normals_number;
	ctx->callbacks->onobject.onarrayend = onjson_normals_end;
}

void GLModels::onjson_normals_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror) {
	ModelJsonParserMeshData* data = (ModelJsonParserMeshData*)target;
	
	if (!noerror) {
		data->normals.clear();
	}
}

void GLModels::onjson_normals_number(struct json_context* ctx, const int index, const struct json_number& number, void* target) {
	ModelJsonParserMeshData* data = (ModelJsonParserMeshData*)target;
	
	if (number.is_float) {
		data->normals.push(number.v.f);
	} else {
		data->normals.push(number.v.i);
	}
}

void GLModels::onjson_texturecoords_start(struct json_context* ctx, const char* key, void* target) {
	ModelLog(L"mesh.texturecoords");
	
	ctx->callbacks->onarray.onarraystart = onjson_coords_start;
	ctx->callbacks->onobject.onarrayend = onjson_texturecoords_end;
}

void GLModels::onjson_texturecoords_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror) {
	ModelJsonParserMeshData* data = (ModelJsonParserMeshData*)target;

	if (!noerror) {
		data->releaseTexturecoords();
	}
}

void GLModels::onjson_coords_start(struct json_context* ctx, const int index, void* target) {
	//ModelJsonParserMeshData* data = (ModelJsonParserMeshData*)target;
	
	ModelLog(L"mesh.texturecoords[%d]", index);
	Vector<GLfloat>* data = memNew(data, Vector<GLfloat>(0, 128));
	if (data == NULL)
		throw eOutOfMemory;
	
	json_clear_callbacks(ctx->callbacks, data);
	ctx->callbacks->onarray.onnumber = onjson_coords_number;
	ctx->callbacks->onarray.onarrayend = onjson_coords_end;
}

void GLModels::onjson_coords_end(struct json_context* ctx, const int index, void* parenttarget, void* target, bool noerror) {
	Vector<GLfloat>* data = (Vector<GLfloat>*)target;
	if (noerror) {
		ModelJsonParserMeshData* parentdata = (ModelJsonParserMeshData*)parenttarget;
		parentdata->texturecoords.push(data);
	} else {
		memDelete(data);
	}
}

void GLModels::onjson_coords_number(struct json_context* ctx, const int index, const struct json_number& number, void* target) {
	Vector<GLfloat>* data = (Vector<GLfloat>*)target;
	
	if (number.is_float) {
		data->push(number.v.f);
	} else {
		data->push(number.v.i);
	}
}

void GLModels::onjson_faces_start(struct json_context* ctx, const char* key, void* target) {
	ModelJsonParserMeshData* data = (ModelJsonParserMeshData*)target;

	ModelLog(L"mesh.faces");
	
	data->faces.growstep(128);
	ctx->callbacks->onarray.onarraystart = onjson_triangle_start;
	ctx->callbacks->onobject.onarrayend = onjson_faces_end;
}

void GLModels::onjson_faces_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror) {
	ModelJsonParserMeshData* data = (ModelJsonParserMeshData*)target;
	
	if (!noerror) {
		data->faces.clear();
	}
}

void GLModels::onjson_triangle_start(struct json_context* ctx, const int index, void* target) {
	ModelJsonParserMeshData* data = (ModelJsonParserMeshData*)target;
	
	json_clear_callbacks(ctx->callbacks, target);
	ctx->callbacks->onarray.onnumber = onjson_triangle_number;
	ctx->callbacks->onarray.onarrayend = onjson_triangle_end;
}

void GLModels::onjson_triangle_end(struct json_context* ctx, const int index, void* parenttarget, void* target, bool noerror) {
	ModelJsonParserMeshData* data = (ModelJsonParserMeshData*)target;
	
	if (!noerror) {
		data->faces.clear();
	}
}

void GLModels::onjson_triangle_number(struct json_context* ctx, const int index, const struct json_number& number, void* target) {
	ModelJsonParserMeshData* data = (ModelJsonParserMeshData*)target;
	
	if (number.is_float) {
		data->faces.push(number.v.f);
	} else {
		data->faces.push(number.v.i);
	}
}

void GLModels::onjson_materials_start(struct json_context* ctx, const char* key, void* target) {
	ModelJsonParser* parser = (ModelJsonParser*)target;
	
	ModelLog(L"model.materials");
	
	ctx->callbacks->onarray.onobjectstart = onjson_material_start;
	ctx->callbacks->onobject.onarrayend = onjson_materials_end;
}

void GLModels::onjson_materials_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror) {
	ModelJsonParser* parser = (ModelJsonParser*)target;
	
	if (!noerror) {
		parser->releaseMaterials();
	}
}

void GLModels::onjson_material_start(struct json_context* ctx, const int index, void* target) {
	ModelJsonParser* parser = (ModelJsonParser*)target;
	
	ModelLog(L"model.materials[%d]", index);

	ModelJsonParserMaterialData* data = memNew(data, ModelJsonParserMaterialData());
	if (data == NULL)
		throw eOutOfMemory;
	
	data->models = parser->models;
	data->material = new RefGLMaterial();
	
	if (data->material == NULL)
		throw eOutOfMemory;
	
	json_clear_callbacks(ctx->callbacks, data);
	ctx->callbacks->onobject.onarraystart = onjson_material_array_start;
	ctx->callbacks->onarray.onobjectend = onjson_material_end;
}

void GLModels::onjson_material_end(struct json_context* ctx, const int index, void* parenttarget, void* target, bool noerror) {
	ModelJsonParserMaterialData* data = (ModelJsonParserMaterialData*)target;
	
	if (noerror) {
		ModelJsonParser* parentdata = (ModelJsonParser*)parenttarget;

		try {
			GLMaterial* material = new GLMaterial(data->material);
			parentdata->materials.push(material);
		} catch (...) {
			memDelete(data);
			throw;
		}
	}
	memDelete(data);
}

void GLModels::onjson_material_array_start(struct json_context* ctx, const char* key, void* target) {
	//ModelJsonParserNodeData* data = (ModelJsonParserMaterialData*)target;
	
	json_clear_callbacks(ctx->callbacks, target);
	if (strcmp(key, "properties") == 0) {
		onjson_material_properties_start(ctx, key, target);
	}
}

// material.properties
void GLModels::onjson_material_properties_start(struct json_context* ctx, const char* key, void* target) {
	//ModelJsonParserMaterialData* data = (ModelJsonParserMaterialData*)target;
	
	ModelLog(L"material.properties");
	
	ctx->callbacks->onarray.onobjectstart = onjson_material_property_start;
	ctx->callbacks->onobject.onarrayend = onjson_material_properties_end;
}

void GLModels::onjson_material_properties_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror) {
	//ModelJsonParserMaterialData* data = (ModelJsonParserMaterialData*)target;
}

// material.properties[index]
void GLModels::onjson_material_property_start(struct json_context* ctx, const int index, void* target) {
	//ModelJsonParserMaterialData* data = (ModelJsonParserMaterialData*)target;
	
	ModelLog(L"material.properties[%d]", index);
	
	json_clear_callbacks(ctx->callbacks, target);
	ctx->callbacks->onobject.onstring = onjson_material_property_string;
	ctx->callbacks->onobject.onnumber = onjson_material_property_number;
	ctx->callbacks->onobject.onarraystart = onjson_material_property_array_start;
	ctx->callbacks->onarray.onobjectend = onjson_material_property_end;
}

void GLModels::onjson_material_property_end(struct json_context* ctx, const int index, void* parenttarget, void* target, bool noerror) {
	//ModelJsonParserMaterialData* data = (ModelJsonParserMaterialData*)target;
}

void GLModels::onjson_material_property_string(struct json_context* ctx, const char* key, char* value, void* target) {
	ModelJsonParserMaterialData* data = (ModelJsonParserMaterialData*)target;
	
	if (strcmp(key, "key") == 0) {
		data->key = value;
		data->key.toLowerCase();
	} else if (strcmp(key, "value") == 0) {
		if ((data->semantic == 0) && (data->key.endsWith(L".name"))) {
			data->material->name = value;
		} else if (data->key.endsWith(L".file")) {
			String file = value; file.toLowerCase();
			switch (data->semantic) {
				case 1: data->material->diffuse.texture = new GLMaterialTexture(data->models->context, file); break;
				case 2: data->material->specular.texture = new GLMaterialTexture(data->models->context, file); break;
				case 4: data->material->emissive.texture = new GLMaterialTexture(data->models->context, file); break;
				case 5: data->material->bump.texture = new GLMaterialTexture(data->models->context, file); break;
				case 7: data->material->shininess.texture = new GLMaterialTexture(data->models->context, file); break;
				case 8: data->material->opacity.texture = new GLMaterialTexture(data->models->context, file); break;
				case 11: data->material->reflection.texture = new GLMaterialTexture(data->models->context, file); break;
			}
		}
	}
}

void GLModels::onjson_material_property_number(struct json_context* ctx, const char* key, const struct json_number& number, void* target) {
	ModelJsonParserMaterialData* data = (ModelJsonParserMaterialData*)target;

	if (strcmp(key, "semantic") == 0) {
		if (number.is_float) {
			data->semantic = number.v.f;
		} else {
			data->semantic = number.v.i;
		}
	} else if (strcmp(key, "value") == 0) {
		GLfloat value = (number.is_float ? number.v.f : number.v.i);
		if (data->semantic == 0) {
			if (data->key.endsWith(L"shininess")) {
				data->material->shininess.value = value;
			} else if (data->key.endsWith(L"shinpercent")) {
				data->material->shininess.percent = value;
			} else if (data->key.endsWith(L"opacity")) {
				data->material->opacity.value = value;
			} else if (data->key.endsWith(L"bumpscaling")) {
				data->material->bump.scale = value;
			}
		} else {
			if (data->key.endsWith(L"blend")) {
				switch (data->semantic) {
					case 1: data->material->diffuse.blend = value; break;
					case 2: data->material->specular.blend = value; break;
					case 4: data->material->emissive.blend = value; break;
					case 7: data->material->shininess.blend = value; break;
					case 8: data->material->opacity.blend = value; break;
				}
			} else if (data->key.endsWith(L"mapmodeu")) {
				// Default: 0
			} else if (data->key.endsWith(L"mapmodev")) {
				// Default: 0
			}
		}
	}
}
void GLModels::onjson_material_property_array_start(struct json_context* ctx, const char* key, void* target) {
	//ModelJsonParserMaterialData* data = (ModelJsonParserMaterialData*)target;

	json_clear_callbacks(ctx->callbacks, target);
	if (strcmp(key, "value") == 0) {
		onjson_material_property_value_start(ctx, key, target);
	}
}

void GLModels::onjson_material_property_value_start(struct json_context* ctx, const char* key, void* target) {
	//ModelJsonParserMaterialData* data = (ModelJsonParserMaterialData*)target;
	
	ctx->callbacks->onarray.onnumber = onjson_material_property_value_number;
	ctx->callbacks->onobject.onarrayend = onjson_material_property_value_end;
}

void GLModels::onjson_material_property_value_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror) {
	ModelJsonParserMaterialData* data = (ModelJsonParserMaterialData*)target;
	if (noerror) {
		if (data->semantic == 0) {
			if (data->value.count() >= 3) {
				if (data->key.endsWith(L"ambient")) {
					Vec3SetV(data->material->ambient.color.v, data->value.items());
				} else if (data->key.endsWith(L"diffuse")) {
					Vec3SetV(data->material->diffuse.color.v, data->value.items());
				} else if (data->key.endsWith(L"specular")) {
					Vec3SetV(data->material->specular.color.v, data->value.items());
				} else if (data->key.endsWith(L"emissive")) {
					Vec3SetV(data->material->emissive.color.v, data->value.items());
				}
			}
		} else {
			if (data->key.endsWith(L"uvtrafo")) {
				// Default: [0,0,1,1,0]
			}
		}
	}
}

void GLModels::onjson_material_property_value_number(struct json_context* ctx, const int index, const struct json_number& number, void* target) {
	ModelJsonParserMaterialData* data = (ModelJsonParserMaterialData*)target;
	
	if (number.is_float) {
		data->value.push(number.v.f);
	} else {
		data->value.push(number.v.i);
	}
}

GLModel& GLModels::createModel(const String& key, const char* json) throw(const char*) {
	if (json == NULL)
		throw eNullPointer;
	
	struct ModelJsonParser target;
	target.models = this;
	
	struct json_context ctx;
	struct json_callbacks callbacks;
	ctx.callbacks = &callbacks;
	json_clear_callbacks(&callbacks, &target);
	callbacks.onrootstart = onjson_root_start;
	if (!json_call(&ctx, json)) {
#ifdef DEBUG
		json_debug_error(ctx, json);
#endif
		throw eConvert;
	}
	
	throw eOK;
}
