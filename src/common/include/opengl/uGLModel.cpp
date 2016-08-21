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
#include <core/uSystem.h>

RefGLMaterialTexture::~RefGLMaterialTexture() {
	if (texture != NULL) {
		memDelete(texture);
		texture = NULL;
	}
	
	texture1iv = 0;
	context = NULL;
}

RefGLMaterialTexture::RefGLMaterialTexture(GLRender* context, const JString& textureReference) throw(const char*) {
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

RefGLMaterial::RefGLMaterial(const JString& name) {
	THIS.name = name;
}

RefGLMaterial::RefGLMaterial(const JString& name, const GLMaterialTexture& texture) {
	THIS.name = name;
	
	diffuse.texture = texture;
}

RefGLMaterial::~RefGLMaterial() {
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

RefGLModelNode::~RefGLModelNode() {
	name = null;
	nodes = null;
	meshes = null;
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

void RefGLModelNode::render(GLRender* context, GLModel& model) const {
	if (nodes.size() > 0) {
		Iterator<GLModelNode> it = nodes.iterator();
		while (it.hasNext()) {
			it.next().render(context, model);
		}
	} else if (meshes.size() > 0) {
		Iterator<GLModelMesh> it = meshes.iterator();
		while (it.hasNext()) {
			it.next().render(context, model);
		}
	}
}

RefGLModelMesh::~RefGLModelMesh() {
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

void RefGLModelMesh::render(GLRender* context, GLModel& model) const {
	context->shaderModel;
	
	const RefGLMaterial* mat = &(model.ref().materials.get(material).ref());
	glUniform4fv(context->shaderModel.uColors, mat->colors4fv.count() / 4, mat->colors4fv.items());
	
	glBindBuffer(GL_ARRAY_BUFFER, vertices);
	glVertexAttribPointer(context->shaderModel.aVertexPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, texturecoords);
	glVertexAttribPointer(context->shaderModel.aTextureCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, normals);
	glVertexAttribPointer(context->shaderModel.aVertexNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glActiveTexture(GL_TEXTURE0);
	if (mat->diffuse.texture != null) {
		glBindTexture(GL_TEXTURE_2D, mat->diffuse.texture.ref().texture1iv);
	} else {
		glBindTexture(GL_TEXTURE_2D, context->textures->defaultTextureHandle);
	}
	glActiveTexture(GL_TEXTURE1);
	if (mat->specular.texture != null) {
		glBindTexture(GL_TEXTURE_2D, mat->specular.texture.ref().texture1iv);
	} else {
		glBindTexture(GL_TEXTURE_2D, context->textures->defaultTextureHandle);
	}
	glActiveTexture(GL_TEXTURE2);
	if (mat->emissive.texture != null) {
		glBindTexture(GL_TEXTURE_2D, mat->emissive.texture.ref().texture1iv);
	} else {
		glBindTexture(GL_TEXTURE_2D, context->textures->defaultTextureHandle);
	}
	
	static GLint modelTextureIndexes[3] = {0, 1, 2};
	glUniform1iv(context->shaderModel.uTexture, 3, modelTextureIndexes);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexes);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0);
}

//===============================

RefGLModel::RefGLModel(GLRender* context) {
	THIS.context = context;
}

RefGLModel::~RefGLModel() {
	context = NULL;
	materials = null;
	rootnode = null;
}

GLuint RefGLModel::createMaterial(const JString& key, const GLfloat x, const GLfloat y, const GLfloat w, const GLfloat h, GLTextureMapping uvmap) {
	GLMaterialTexture texture = new GLMaterialTexture(context, key);
	texture.ref().setUVMap(x, y, w, h, uvmap);
	GLMaterial material = new GLMaterial(key, texture);
	material.update();
	return materials.indexOf(materials.push(material));
}

GLuint RefGLModel::insertMaterial(GLMaterial& material) {
	material.update();
	return materials.indexOf(materials.push(material));
}

//===============================

GLModels::GLModels(GLRender* context) throw(const char*) {
	THIS.context = context;
	list = new HashMap<JString, GLModel>();
}

GLModels::~GLModels() {
	list = null;
	context = NULL;
}

GLModel& GLModels::get(const JString& key) throw(const char*) {
	return (GLModel&)list.get(key);
}

GLModel& GLModels::createModel(const JString& key) throw(const char*) {
	try {
		list.remove(key);
		GLModel* model = &(list.put(key, new RefGLModel(context)));
		return *model;
	} catch (...) {
		throw;
	}
}

//===============================

#define ModelLog(fmt, ...) JString::format(fmt, ## __VA_ARGS__).log()

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
			JString file = value; file.toLowerCase();
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

GLModel& GLModels::createModel(const JString& key, const char* json) throw(const char*) {
	if (json == NULL)
		throw eNullPointer;
	
	// Парсим JSON во временный обьект
	ModelJsonParser target;
	target.models = this;
	target.key = key;
	
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
	
	// Проверка данных
	int32_t meshes = target.meshes.count();
	int32_t materials = target.materials.count();
	if ((target.rootnode == NULL) || (meshes == 0) || (materials == 0)) {
		target. release();
		throw eInvalidFormat;
	}

	// Проверка наличия обьектов с координатами
	Vector<ModelJsonParserNodeData*> nodestack;
	ModelJsonParserNodeData* node = target.rootnode;
	int32_t used = 0;
	while (node != NULL) {
		int32_t count = node->meshes.count();
		if (count > 0) {
			GLuint* items = node->meshes.items();
			for (int i = 0; i < count; i++) {
				if (items[i] < meshes) {
					ModelJsonParserMeshData* mesh = target.meshes.get(items[i]);
					if ((mesh->vertices.count() > 0) && (mesh->faces.count() > 0)) {
						used++;
					}
				} else {
					target.release();
					throw eInvalidFormat;
				}
			}
		}
		count = node->children.count();
		if (count > 0) {
			ModelJsonParserNodeData** items = node->children.items();
			for (int i = 0; i < count; i++) {
				nodestack.push(items[i]);
			}
		}
		try {
			node = nodestack.pop();
		} catch (...) {
			node = NULL;
		}
	}
	
	if (used == 0) {
		target.release();
		throw eInvalidFormat;
	}
	
	// Проверка используемых материалов
	ModelJsonParserMeshData** items = target.meshes.items();
	for (int i = 0; i < meshes; i++) {
		if (items[i]->materialindex >= materials) {
			target.release();
			throw eInvalidFormat;
		}
	}
	
	// Все проверки пройдены - создаем GLModel
	GLModel* model = NULL;
	try {
		model = (GLModel*)MainThreadSync(CreateJsonModelCallback, &target);
	} catch (...) {
		target.release();
		throw;
	}
	
	target.release();
	return *model;
}

void* GLModels::CreateJsonModelCallback(void* threadData) {
	ModelJsonParser* parser = (ModelJsonParser*)threadData;
	
	GLModel* model = NULL;
	try {
		model = &(parser->models->createModel(parser->key));
		model->ref().parseJson(parser);
	} catch (...) {
		throw;
	}
	
	return model;
}

void RefGLModel::parseJson(ModelJsonParser* parser) throw(const char*) {
	Mat4 rootmat; rootmat.identity();
	try {
		rootnode = new GLModelNode();
		rootnode.ref().parseJson(parser->rootnode, parser->meshes, rootmat);
	} catch (...) {
		rootnode = null;
	}
}

void RefGLModelNode::parseJson(ModelJsonParserNodeData* node, Vector<ModelJsonParserMeshData*>& meshes, Mat4& rootmat) {
	name = node->name;
	Mat4Transpose(transformation.v, node->transformation.items());
	
	Mat4 matV; matV.multiply(rootmat, transformation);
	
	int32_t childrenCount = node->children.count();
	if (childrenCount > 0) {
		ModelJsonParserNodeData** items = node->children.items();
		for (int i = 0; i < childrenCount; i++) {
			GLModelNode* childNode = new GLModelNode();
			if (childNode == NULL)
				throw eOutOfMemory;
			
			try {
				childNode->ref().parseJson(items[i], meshes, matV);
				insertNode(*childNode);
			} catch (...) {
				delete childNode;
			}
		}
		return;
	}
	
	int32_t meshesCount = node->meshes.count();
	if (meshesCount > 0) {
		Mat4 matN; matN.inverse(matV).transpose();
		
		GLuint* items = node->meshes.items();
		for (int i = 0; i < meshesCount; i++) {
			GLModelMesh* modelMesh = new GLModelMesh();
			if (modelMesh == NULL)
				throw eOutOfMemory;
			
			try {
				modelMesh->ref().parseJson(meshes.get(items[i]), matN, matV);
				insertMesh(*modelMesh);
			} catch (...) {
				delete modelMesh;
			}
		}
	}
}

void RefGLModelMesh::parseJson(ModelJsonParserMeshData* mesh, Mat4& matN, Mat4& matV) {
	GLuint vertexBuffer = 0;
	GLuint normalsBuffer = 0;
	GLuint textureBuffer = 0;
	GLuint indexBuffer = 0;
	
	try {
		glGenBuffers(1, &vertexBuffer);
		CheckGLError();
		{
			int32_t vsrcCount = mesh->vertices.count();
			GLfloat* vsrc = mesh->vertices.items();
			GLfloat* vdst = memAlloc(GLfloat, vdst, vsrcCount * sizeof(GLfloat));
			if (vdst == NULL)
				throw eOutOfMemory;
			
			for (int i = 0; i < vsrcCount; i += 3) {
				Vec3Transform(vdst + i, vsrc + i, matV.v);
				Vec3Multiply(vdst + i, vdst + i, 2.54); // convert inches into mm
			}
			
			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
			glBufferData(GL_ARRAY_BUFFER, vsrcCount * sizeof(GLfloat), vdst, GL_STATIC_DRAW);
			try {
				CheckGLError();
			} catch (...) {
				memFree(vdst);
				throw;
			}
		}
		
		glGenBuffers(1, &normalsBuffer);
		CheckGLError();
		{
			int32_t vsrcCount = mesh->normals.count();
			GLfloat* vsrc = mesh->normals.items();
			GLfloat* vdst = memAlloc(GLfloat, vdst, vsrcCount * sizeof(GLfloat));
			if (vdst == NULL)
				throw eOutOfMemory;
			
			for (int i = 0; i < vsrcCount; i += 3) {
				Vec3Transform(vdst + i, vsrc + i, matN.v);
			}
			
			glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
			glBufferData(GL_ARRAY_BUFFER, vsrcCount * sizeof(GLfloat), vdst, GL_STATIC_DRAW);
			try {
				CheckGLError();
			} catch (...) {
				memFree(vdst);
				throw;
			}
		}
		
		glGenBuffers(1, &textureBuffer);
		CheckGLError();
		{
			Vector<GLfloat>* coords = mesh->texturecoords.get(0);
			
			glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
			glBufferData(GL_ARRAY_BUFFER, coords->count() * sizeof(GLfloat), coords->items(), GL_STATIC_DRAW);
			CheckGLError();
		}
		
		glGenBuffers(1, &indexBuffer);
		CheckGLError();
		{
			indexCount = mesh->faces.count();
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLshort), mesh->faces.items(), GL_STATIC_DRAW);
		}
	} catch (...) {
		if (vertexBuffer != 0) {
			glDeleteBuffers(1, &vertexBuffer);
		}
		if (normalsBuffer != 0) {
			glDeleteBuffers(1, &normalsBuffer);
		}
		if (textureBuffer != 0) {
			glDeleteBuffers(1, &textureBuffer);
		}
		if (indexBuffer != 0) {
			glDeleteBuffers(1, &indexBuffer);
		}
		throw;
	}
	
	name = mesh->name;
	material = mesh->materialindex;
	vertices = vertexBuffer;
	normals = normalsBuffer;
	texturecoords = textureBuffer;
	indexes = indexBuffer;
}

