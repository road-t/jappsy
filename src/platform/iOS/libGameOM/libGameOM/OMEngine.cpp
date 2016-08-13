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

#include "OMEngine.h"

#include <opengl/uOpenGL.h>
#include <opengl/uGLRender.h>
#include <net/uLoader.h>

static int color = 0;

void onFrame(GLRender* context) {
    float c = (float)color / 255.0f;
    glClearColor(0.0f, c, 0.0f, 1.0f);
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    
    color++;
    if (color >= 256) color = 0;
}

void onTouch(const wchar_t* event) {
    
}

#include <net/uURI.h>
#include <data/uAtomicObject.h>
#include <data/uStack.h>
#include <data/uIterator.h>
#include <data/uListIterator.h>
#include <data/uCollection.h>

OMEngine::OMEngine() {
    context = memNew(context, GLRender(this, 1920, 1080, ::onFrame, ::onTouch));
    
    URI::basePath = L"https://localhost/";
    
    AtomicInteger i = new AtomicInteger(0);
    i.set(10);

    Stack<Object>::_test();
    Iterator<Object>::_test();
    ListIterator<Object>::_test();
    Collection<Object>::_test();
    SynchronizedCollection<Object>::_test();
    
    //URI* uri = new URI(L"http://user:password@host:12345/path/path2/path3/file.ext?query=query#anchor");
    URI* uri = new URI(L"path/path2/path3/file.ext?query=query#anchor");
    String url = uri->uri();
    delete uri;

    NamedArray<NamedStrings>* query = memNew(query, NamedArray<NamedStrings>());
    
    NamedStrings* shaders = memNew(shaders, NamedStrings());
    shaders->insert(L"vsh_main",            L"shaders/vsh_main.jsh");
    shaders->insert(L"vsh_texture",         L"shaders/vsh_texture.jsh");
    shaders->insert(L"square_fill",         L"shaders/square_fill.jsh");
    shaders->insert(L"square_stroke",       L"shaders/square_stroke.jsh");
    shaders->insert(L"square_texture",      L"shaders/square_texture.jsh");
    shaders->insert(L"sprite",              L"shaders/sprite.jsh");
    shaders->insert(L"particle",            L"shaders/particle.jsh");
    shaders->insert(L"model",               L"shaders/model.jsh");
    
    shaders->insert(L"vsh_effect",          L"shaders/vsh_effect.jsh");
    shaders->insert(L"effect_torsion",      L"shaders/effect_torsion.jsh");
    shaders->insert(L"effect_sun1",         L"shaders/effect_sun1.jsh");
    shaders->insert(L"effect_moon1",        L"shaders/effect_moon1.jsh");
    shaders->insert(L"effect_unknown",      L"shaders/effect_unknown.jsh");
    shaders->insert(L"effect_unknown_star", L"shaders/effect_unknown_star.jsh");
    shaders->insert(L"effect_sun2",         L"shaders/effect_sun2.jsh");
    shaders->insert(L"effect_moon2",        L"shaders/effect_moon2.jsh");
    shaders->insert(L"effect_star1",        L"shaders/effect_star1.jsh");
    shaders->insert(L"effect_star2",        L"shaders/effect_star2.jsh");
				
    shaders->insert(L"fsh_effect_test",     L"shaders/fsh_effect_test.fsh");
				
    shaders->insert(L"mobile_effect_torsion",       L"shaders/mobile_effect_torsion.jsh");
    shaders->insert(L"mobile_effect_sun1",          L"shaders/mobile_effect_sun1.jsh");
    shaders->insert(L"mobile_effect_moon1",         L"shaders/mobile_effect_moon1.jsh");
    shaders->insert(L"mobile_effect_unknown",       L"shaders/mobile_effect_unknown.jsh");
    shaders->insert(L"mobile_effect_unknown_star",  L"shaders/mobile_effect_unknown_star.jsh");
    shaders->insert(L"mobile_effect_sun2",          L"shaders/mobile_effect_sun2.jsh");
    shaders->insert(L"mobile_effect_moon2",         L"shaders/mobile_effect_moon2.jsh");
    shaders->insert(L"mobile_effect_star1",         L"shaders/mobile_effect_star1.jsh");
    shaders->insert(L"mobile_effect_star2",         L"shaders/mobile_effect_star2.jsh");
    
    query->insert(L"shaders", shaders);
    
    NamedStrings* textures = memNew(textures, NamedStrings());
    textures->insert(L"null",               L"models/null.jimg");
    textures->insert(L"mramor.png",         L"models/mramor.jimg");
    textures->insert(L"metal.png",          L"models/metal.jimg");
    textures->insert(L"gold.png",           L"models/gold.jimg");
    textures->insert(L"cox",                L"models/cox.jimg");
    
    textures->insert(L"background1",        L"images/background1.jimg");
    textures->insert(L"background2",        L"images/background2.jimg");
    textures->insert(L"background3",        L"images/background3.jimg");
    textures->insert(L"background4",        L"images/background4.jimg");
    textures->insert(L"buttons",            L"images/buttons.jimg");
    textures->insert(L"sprites",            L"images/sprites.jimg");
				
    textures->insert(L"mobile_tex_effect_star",     L"shaders/mobile_tex_effect_star.jimg");
    textures->insert(L"mobile_tex_random",          L"shaders/mobile_tex_random.jimg");
    
    query->insert(L"textures", textures);
    
    NamedStrings* models = memNew(models, NamedStrings());
    models->insert(L"ball",                 L"models/ball.json");
    models->insert(L"cap",                  L"models/cap.json");
    models->insert(L"separator",            L"models/separator.json");
    models->insert(L"bar",                  L"models/bar.json");
    
    query->insert(L"models", models);
    
    memDelete(query);
}

OMEngine::~OMEngine() {
    memDelete(context);
}