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

struct ModelJsonParser {
	GLModels* models;
};

struct ModelJsonParserNodeData {
	
};

void GLModels::onjson_root(struct json_context* ctx, void* target) {
	ctx->callbacks->onobject.onobjectstart = onjson_object_root_object;
	ctx->callbacks->onobject.onarraystart = onjson_object_root_array;
}

void GLModels::onjson_object_root_object(struct json_context* ctx, const char* key, void* target) {
	struct ModelJsonParser* parser = (struct ModelJsonParser*)target;

	String::format(L"ModelParser: Object %s", key).log();
	
	json_clear_callbacks(ctx->callbacks, target);
	if (strcmp(key, "rootnode") == 0) {
		ctx->callbacks->onobject.onstring; // name
		ctx->callbacks->onobject.onarraystart; // transformation | children
	}
}

void GLModels::onjson_object_root_array(struct json_context* ctx, const char* key, void* target) {
	struct ModelJsonParser* parser = (struct ModelJsonParser*)target;

	String::format(L"ModelParser: Array %s", key).log();

	json_clear_callbacks(ctx->callbacks, target);
	if (strcmp(key, "meshes") == 0) {
		ctx->callbacks->onarray.onobjectstart; // meshobject
	} else if (strcmp(key, "materials") == 0) {
		ctx->callbacks->onarray.onobjectstart; // materialobject
	}
}

void GLModels::onjson_object_node_string_name(struct json_context* ctx, const char* key, char* value, void* target) {
	// node.name
}

void GLModels::onjson_object_node_array(struct json_context* ctx, const char* key, void* target) {
	// node.transformation | node.children | node.meshes
}

void GLModels::onjson_array_tranformation_number(struct json_context* ctx, const int index, const struct json_number& number, void* target) {
	// node.transformation[index]
}

void GLModels::onjson_array_children_object(struct json_context* ctx, const int index, void* target) {
	// childnode
}

void GLModels::onjson_array_meshes_number(struct json_context* ctx, const int index, const struct json_number& number, void* target) {
	// node.meshes[index]
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
	callbacks.onroot = onjson_root;
	if (!json_call(&ctx, json)) {
#ifdef DEBUG
		json_debug_error(ctx, json);
#endif
		throw eConvert;
	}
	
	throw eOK;
}
