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
	items[7] = (diffuse.texture == null) ? 0 : diffuse.texture.ref().blend;
	
	Vec3SetV(items + 8, specular.color.v);
	items[11] = (specular.texture == null) ? 0 : specular.texture.ref().blend;
	
	Vec3SetV(items + 12, emissive.color.v);
	items[15] = (emissive.texture == null) ? 0 : emissive.texture.ref().blend;
	
	items[16] = shininess.value;
	items[17] = shininess.percent;
	items[18] = 0.0; // unused
	items[19] = (opacity.texture == null) ? 0 : opacity.texture.ref().blend;
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

class ModelJsonParserMaterialData {
public:
	String name;
	
};

class ModelJsonParser {
public:
	GLModels* models;
	
	ModelJsonParserNodeData* rootnode = NULL;
	Vector<ModelJsonParserMeshData*> meshes;
	Vector<ModelJsonParserMaterialData*> materials;
	
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
			ModelJsonParserMaterialData** items = materials.items();
			for (int i = 0; i < count; i++) {
				memDelete(items[i]);
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
	
	ctx->callbacks->target = data;
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
	
	ctx->callbacks->target = data;
	ctx->callbacks->onobject.onstring = onjson_mesh_string;
	ctx->callbacks->onobject.onnumber = onjson_mesh_number;
	ctx->callbacks->onobject.onarraystart = onjson_node_array_start;
	ctx->callbacks->onarray.onobjectend = onjson_mesh_end;
}

void GLModels::onjson_mesh_end(struct json_context* ctx, const int index, void* parenttarget, void* target, bool noerror) {
	
}

void GLModels::onjson_mesh_string(struct json_context* ctx, const char* key, char* value, void* target) {
	
}

void GLModels::onjson_mesh_number(struct json_context* ctx, const char* key, const struct json_number& number, void* target) {

}

void GLModels::onjson_mesh_array_start(struct json_context* ctx, const char* key, void* target) {
	// vertices | normals | texturecoords | faces
}

void GLModels::onjson_materials_start(struct json_context* ctx, const char* key, void* target) {
	ModelJsonParser* parser = (ModelJsonParser*)target;
	
	ModelLog(L"model.materials");
	
	//ctx->callbacks->onarray.onobjectstart; // materialobject
	
	ctx->callbacks->onobject.onarrayend = onjson_materials_end;
}

void GLModels::onjson_materials_end(struct json_context* ctx, const char* key, void* parenttarget, void* target, bool noerror) {
	ModelJsonParser* parser = (ModelJsonParser*)target;
	
	if (!noerror) {
		parser->releaseMaterials();
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
