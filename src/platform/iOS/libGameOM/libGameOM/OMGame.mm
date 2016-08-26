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

#include "OMGame.h"
#include "OMView.h"

#include <opengl/uGLRender.h>
#include <core/uSystem.h>
#include <opengl/uGLLight.h>

void OMGame::startBars(bool freespin) {
    bars.start();
    updateStage(STAGE_BARS_WAIT);
    rollTimeout = currentTimeMillis() + 3000;
    
    if (freespin) {
        LOG("Фриспин - Ускорение всех барабанов");
    } else if (conf.autospin) {
        LOG("Автоспин - Ускорение всех барабанов");
        conf.points -= conf.bet;
    } else {
        LOG("Ускорение всех барабанов");
        conf.points -= conf.bet;
    }
}

void OMGame::updateStage(OMGameStage stage, onNextCallback callback, uint64_t timeout, bool fade) {
    uint64_t nextTimeout = currentTimeMillis() + timeout;
    nextTime = fade ? nextTimeout : 0;
    
    if (timeout == 0) {
        this->stage = stage;
        this->nextTimeout = 0;
        nextStage = stage;
        nextCallback = NULL;
        
        updateStageVisibility();
        
        if (callback)
            callback(this);
    } else {
        this->nextTimeout = nextTimeout;
        nextStage = stage;
        nextCallback = callback;
    }
}

void OMGame::updateStageVisibility() {
    if (stage <= STAGE_BARS) {
        scene->visibleGroup(groups.barlist, true);
        scene->visibleGroup(groups.bareffects, true);
        scene->visibleGroup(groups.dbleffects, false);
    } else {
        scene->visibleGroup(groups.barlist, false);
        scene->visibleGroup(groups.bareffects, false);
        scene->visibleGroup(groups.dbleffects, true);
    }
    
    scene->visibleGroup(groups.btnlist, false);
    
    Vector<GLObject*> buttons;
    buttons.push( scene->objects->get(L"btn_rewards") );
    buttons.push( scene->objects->get(L"btn_grace") );
    buttons.push( scene->objects->get(L"btn_donate") );
    if (stage <= STAGE_BARS) {
        if (conf.autospin) {
            buttons.push( scene->objects->get(L"btn_double") );
            buttons.push( scene->objects->get(L"btn_stop") );
        } else {
            if (stage == STAGE_BARS) {
                buttons.push( scene->objects->get(L"btn_auto") );
                buttons.push( scene->objects->get(L"btn_info") );
                buttons.push( scene->objects->get(L"btn_start") );
            } else {
                buttons.push( scene->objects->get(L"btn_double") );
                buttons.push( scene->objects->get(L"btn_take") );
            }
        }
    } else if (stage <= STAGE_DOUBLE) {
        buttons.push( scene->objects->get(L"btn_sun") );
        buttons.push( scene->objects->get(L"btn_moon") );
        buttons.push( scene->objects->get(L"btn_take") );
    }
    scene->visibleGroup(buttons, true);
}

//static int color = 0;

static const Vec3 barsRotateVector = Vec3({0.0, -1, 0.0}).normalize();

void OMGame::onFrame(GLRender* context) {
    glClearColor(0.2f, 0.4f, 0.8f, 1.0f);
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
/*
    float c = (float)color / 255.0f;
    glClearColor(0.0f, c, 0.0f, 1.0f);
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    
    color++;
    if (color >= 256) color = 0;
*/
    if (ready) {
        uint64_t currentTime = currentTimeMillis();
        
        // Переключение сцены по таймауту
        if ((nextTimeout != 0) && (currentTime >= nextTimeout)) {
            nextTimeout = 0;
            stage = nextStage;
            
            updateStageVisibility();
            
            if (nextCallback)
                nextCallback(this);
        }
        else if (nextTimeout == 0) {
            if (stage <= STAGE_BARS) {
                // FIXIT: Временное торможение без ответа от сервера (не забыть про фриспины)
                if ((rollTimeout != 0) && (currentTime >= rollTimeout)) {
                    rollTimeout = 0;
                    LOG("Торможение всех барабанов");
                    int check = bars.stopRandom();
                    //LOG("Grace: " + check);
                    updateStage(STAGE_BARS_STOP);
                }
                
                if (bars.update()) {
                    scene->rotateGroup(groups.barlist1, barsRotateVector, bars.rotation[0].rotate);
                    scene->rotateGroup(groups.barlist2, barsRotateVector, bars.rotation[1].rotate);
                    scene->rotateGroup(groups.barlist3, barsRotateVector, bars.rotation[2].rotate);
                }
                
                if (bars.stopped()) {
                    int sum = bars.calculate();
                    //LOG("Check: " + sum);
                    LOG("Барабаны остановились");
                    if (sum == 0) {
                        if (conf.free != 0) {
                            prizeTimeout = currentTime - 1;
                        } else {
                            updateStage(STAGE_BARS);
                        }
                    } else if (sum < 0) {
                        LOG("Фриспин +" + (-sum));
                        prizeTimeout = currentTime + 8000; // Время отображения бонус фриспин
                        updateStage(STAGE_BARS_FREE);
                        
                        conf.prizefree = -sum;
                        conf.free -= sum;
                        freespinTime = currentTime;
                        
                        LOG("Запуск феерверков");
#warning Particles
//                        scene->startParticlesGroup(rocketslist, 1);
                    } else if (sum > 0) {
                        CString::format(L"Приз +%d", sum).log();
                        prizeTimeout = currentTime + 4000; // Время отображения приза
                        updateStage(STAGE_BARS_PRIZE);
                        
                        conf.prize = sum * conf.bet;
                        conf.total += conf.prize;
                    }
                }
                
                if ((prizeTimeout != 0) && (prizeTimeout < currentTime)) {
                    prizeTimeout = 0;
                    
                    if ((conf.free > 0) && (conf.spin < conf.free)) {
                        conf.spin++;
                        
                        startBars(true);
                    } else if ((conf.autospin) || ((conf.free != 0) && (conf.total == 0))) {
                        if (conf.total != 0) {
                            LOG("Автокручение - Забрать приз");
                        }
                        updateStage(STAGE_BARS_TAKE);
                    } else {
                        LOG("Войти в удвоение или забрать приз?");
                        updateStage(STAGE_BARS_USER);
                        
                        takeTime = currentTime;
                    }
                }
            }
            
            if (stage <= STAGE_DOUBLE) {
                // FIXIT: Временный показ дабла без ответа от сервера
                if ((selectTimeout != 0) && (currentTime >= selectTimeout)) {
                    LOG("Удвоение - Открытие затмения");
                    selectTimeout = 0;
                    localTime = currentTime;
                    conf.dblindex = Rotation::generateDouble();
                    doubleShowTimeout = currentTime + 3000; // Время открытия затмения
                    updateStage(STAGE_DOUBLE_SHOW);
                    
                    GLDrawing* drawing;
                    if (conf.dblindex == 0) {
                        drawing = bkg_sun;
                    } else {
                        drawing = bkg_moon;
                    }
                    GLAnimation::createFadeIn(drawing, 0.0, 1.0, 2000);
                }
                
                if ((doubleShowTimeout != 0) && (doubleShowTimeout < currentTime)) {
                    doubleShowTimeout = 0;
                    
                    (void)conf.sequence.shift();
                    conf.sequence.push(conf.dblindex);
                    
                    if (conf.dblselect != conf.dblindex) {
                        LOG("Удвоение - Проиграно");
                        conf.doubleout = 0;
                        
                        updateStage(STAGE_DOUBLE_TAKE);
                    } else {
                        LOG("Удвоение - Угадано");
                        conf.doublein = conf.doubleout;
                        conf.doubleout = conf.doublein * 2;
                        
                        LOG("Удвоение - Закрытие затмения");
                        
                        localTime = currentTime + 3000;
                        doubleHideTimeout = currentTime + 3000; // Время закрытия затмения
                        updateStage(STAGE_DOUBLE_HIDE);
                        
                        GLDrawing* drawing;
                        if (conf.dblindex == 0) {
                            drawing = bkg_sun;
                        } else {
                            drawing = bkg_moon;
                        }
                        GLAnimation::createFadeOut(drawing, 0.0, 1.0, 2000);
                        
                        GLAnimation::createLightOut(dbl_sun, 0.3, 1.0, 500);
                        GLAnimation::createLightOut(dbl_moon, 0.3, 1.0, 500);
                    }
                }
                
                if ((doubleHideTimeout != 0) && (doubleHideTimeout < currentTime)) {
                    doubleHideTimeout = 0;
                    if (conf.dblselect == conf.dblindex) {
                        LOG("Удвоение - Удвоить или забрать приз?");
                        updateStage(STAGE_DOUBLE);
                    }
                    
                    GLAnimation::createLightOut(dbl_sun, 0.3, 1.0, 500);
                    GLAnimation::createLightOut(dbl_moon, 0.3, 1.0, 500);
                    GLAnimation::createPingPong(question, 0.75, 1.0, 500);
                }
            }
            
            if (stage == STAGE_BARS_TAKE) {
                if (conf.total == 0) {
                    updateStage(STAGE_BARS);
                    
                    conf.prize = 0;
                    conf.free = 0;
                    conf.spin = 0;
                } else if (takeTimeout < currentTime) {
                    static int values[9] = { 1, 3, 5, 8, 13, 33, 108, 144, 369 };
                    int count = 0;
                    if (conf.total < 3) {
                        count = 1;
                    } else if (conf.total < 5) {
                        count = 2;
                    } else if (conf.total < 8) {
                        count = 3;
                    } else if (conf.total < 13) {
                        count = 4;
                    } else if (conf.total < 33) {
                        count = 5;
                    } else if (conf.total < 108) {
                        count = 6;
                    } else if (conf.total < 144) {
                        count = 7;
                    } else if (conf.total < 369) {
                        count = 8;
                    } else {
                        count = 9;
                    }
                    int idx = randomi(count);
                    conf.total -= values[idx];
                    conf.points += values[idx];
                    
                    GLfloat speed = (GLfloat)(conf.total) / 5000.0 + 1.0;
                    takeTimeout = currentTime + (int)(50.0 / speed);
                }
            }
            
            else if (stage == STAGE_DOUBLE_TAKE) {
                if ((conf.doublein == 0) || (conf.doubleout == 0)) {
                    LOG("Удвоение - Выход");
                    updateStage(STAGE_BARS, doubleReset, (conf.doubleout == 0) ? 3000 : 500, true);
                    
                    conf.total = 0;
                    conf.prize = 0;
                    conf.free = 0;
                    conf.spin = 0;
                } else if (takeTimeout < currentTime) {
                    static int values[9] = { 1, 3, 5, 8, 13, 33, 108, 144, 369 };
                    int count = 0;
                    if (conf.total < 3) {
                        count = 1;
                    } else if (conf.total < 5) {
                        count = 2;
                    } else if (conf.total < 8) {
                        count = 3;
                    } else if (conf.total < 13) {
                        count = 4;
                    } else if (conf.total < 33) {
                        count = 5;
                    } else if (conf.total < 108) {
                        count = 6;
                    } else if (conf.total < 144) {
                        count = 7;
                    } else if (conf.total < 369) {
                        count = 8;
                    } else {
                        count = 9;
                    }
                    int idx = randomi(count);
                    conf.doublein -= values[idx];
                    conf.points += values[idx];
                    
                    GLfloat speed = (GLfloat)(conf.total) / 5000.0 + 1.0;
                    takeTimeout = currentTime + (int)(50.0 / speed);
                }
            }
            
            if ((conf.autospin) && (stage == STAGE_BARS)) {
                startBars(false);
            }
        }
        
        scene->render();
        
        static int fpscounter = 0;
        fpscounter++;
        if (fpscounter >= 10) {
            fpscounter = 0;
            CString::format(L"FPS: %d", context->frame->fps).log();
        }
    }
}

void OMGame::onResize(int width, int height) {
    context->touchScreen->update(1920, 1080); // independent size
}

void OMGame::onTouch(const CString& event) {
    if (nextTimeout == 0) {
        if (event.indexOf("click ") < 0) {
            event.log();
        }
        
        if (event == L"swipe long left") {
            if (stage == STAGE_BARS) {
                startBars(false);
            }
        }
    }
}

bool OMGame::onTrackBar(const CString& event, const GLTouchPoint* cur, const GLTouchPoint* delta, const GLTouchPoint* speed, void* userData) {
    TrackBarData* data = (TrackBarData*)userData;
    OMGame* game = data->game;
    int index = data->index;
    
    if (game->nextTimeout == 0) {
        if (game->stage == STAGE_BARS) {
            GLScene* scene = game->scene;
            
            game->bars.clear();
            if (event.startsWith(L"move")) {
                game->bars.rotation[index].userMove(-delta->x / 4.0);
            } else if (event.startsWith(L"leave")) {
                game->bars.rotation[index].userRotate(-delta->x / 4.0, speed->x * 250.0);
            }
            return true;
        }
    }
    
    return false;
}

bool OMGame::onButtonEvent(GLEngine* engine, const CString& event, GLDrawing* drawing) {
    OMGame* game = (OMGame*)engine;
    if (game->nextTimeout == 0) {
        if (event.startsWith("enter ")) {
            GLAnimation::createBlink(drawing, 1.0, 1.5, 500);
            CString::format(L"Нажата кнопка %ls", (wchar_t*)event).log();
        } else if (event.startsWith("move ")) {
        } else if (event.startsWith("leave ")) {
        } else {
            CString::format(L"Действие по кнопке %ls", (wchar_t*)event).log();
            if (event == "click btn_auto") {
                if (game->stage == STAGE_BARS) {
                    LOG("Включен автоспин");
                    game->conf.autospin = true;
                }
            } else if (event == "click btn_start") {
                if (game->stage == STAGE_BARS) {
                    game->startBars(false);
                }
            } else if (event == "click btn_take") {
                if (game->stage == STAGE_BARS_USER) {
                    LOG("Забрть приз");
                    game->updateStage(STAGE_BARS_TAKE);
                } else if (game->stage == STAGE_BARS_TAKE) {
                    LOG("Забран весь приз без ожидания перетекания");
                    game->conf.points += game->conf.total;
                    game->conf.total = 0;
                } else if (game->stage == STAGE_DOUBLE) {
                    LOG("Удвоение - Забрать приз");
                    game->updateStage(STAGE_DOUBLE_TAKE);
                    
                    GLAnimation::createFadeOut(game->question, 0.0, 1.0, 500);
                } else if (game->stage == STAGE_DOUBLE_TAKE) {
                    LOG("Забран весь приз без ожидания перетекания");
                    if (game->conf.doubleout != 0) {
                        game->conf.points += game->conf.doublein;
                        game->conf.doublein = 0;
                    }
                    game->conf.total = 0;
                }
            } else if (event == "click btn_stop") {
                if (game->conf.autospin) {
                    LOG("Отключен автоспин");
                    game->conf.autospin = false;
                    game->updateStage(game->stage);
                }
            } else if (event == "click btn_double") {
                if (game->stage == STAGE_BARS_USER) {
                    LOG("Вход в удвоение");
                    game->conf.doublein = game->conf.total;
                    game->conf.doubleout = game->conf.total * 2;
                    game->updateStage(STAGE_DOUBLE, NULL, 500, true);
                }
            } else if (event == "click btn_sun") {
                if (game->stage == STAGE_DOUBLE) {
                    game->doubleSelect(0);
                }
            } else if (event == "click btn_moon") {
                if (game->stage == STAGE_DOUBLE) {
                    game->doubleSelect(1);
                }
            } else {
                event.log();
            }
        }
        return true;
    }
    return false;
}

void OMGame::doubleSelect(int index) {
    GLDrawing* drawing1 = (index == 0) ? dbl_sun : dbl_moon;
    GLDrawing* drawing2 = (index != 0) ? dbl_sun : dbl_moon;
    
    GLAnimation::createLightIn(drawing1, 0.3, 1.0, 500);
    GLAnimation::createLightOut(drawing2, 0.3, 1.0, 500);
    GLAnimation::createFadeOut(question, 0.0, 1.0, 500);
    
    updateStage(STAGE_DOUBLE_WAIT);
    selectTimeout = currentTimeMillis() + 1000;
    
    doubleShowTimeout = 0;
    doubleHideTimeout = 0;
    conf.dblselect = index;
    
    LOG("Удвоение - Выбор сделан");
}

void OMGame::doubleReset(OMGame* engine) {
    engine->dbl_sun->paint->setLight(0.3, 1.0);
    engine->dbl_moon->paint->setLight(0.3, 1.0);
    engine->bkg_sun->paint->setLight(0.0, 0.0);
    engine->bkg_moon->paint->setLight(0.0, 0.0);
    
    GLAnimation::createLightOut(engine->dbl_sun, 0.3, 1.0, 500);
    GLAnimation::createLightOut(engine->dbl_moon, 0.3, 1.0, 500);
    GLAnimation::createPingPong(engine->question, 0.75, 1.0, 500);
}

bool OMGame::onDoubleEvent(GLEngine* engine, const CString& event, GLDrawing* drawing) {
    OMGame* game = (OMGame*)engine;
    if (event.indexOf("enter ") == 0) {
        if (game->stage == STAGE_DOUBLE) {
            GLAnimation::createPingPong(drawing, 0.3, 1.0, 500);
            CString::format(L"Нажата кнопка %ls", (wchar_t*)event).log();
        }
    } else if (event.indexOf("move ") == 0) {
    } else if (event.indexOf("leave ") == 0) {
        if (game->stage == STAGE_DOUBLE) {
            GLAnimation::createLightOut(drawing, 0.3, 1.0, 500);
        }
    } else {
        CString::format(L"Действие по кнопке %ls", (wchar_t*)event).log();
        if (event == "click dbl_sun") {
            if (game->stage == STAGE_DOUBLE) {
                game->doubleSelect(0);
            }
        } else if (event == "click dbl_moon") {
            if (game->stage == STAGE_DOUBLE) {
                game->doubleSelect(1);
            }
        } else {
            event.log();
        }
    }
    return true;
}

void OMGame::renderButtonGrace(GLEngine* engine, GLDrawing* drawing) {
    OMGame* game = (OMGame*)engine;
    game->num_sm->renderNumber( {344+7, 928+66}, 30, game->conf.points );
}

void OMGame::renderButtonDonate(GLEngine* engine, GLDrawing* drawing) {
    OMGame* game = (OMGame*)engine;
    game->num_sm->renderNumber( {760+7, 928+66}, 30, game->conf.bet);
}

void OMGame::renderButtonDouble(GLEngine* engine, GLDrawing* drawing) {
    OMGame* game = (OMGame*)engine;
    GLRender* context = game->context;
    
    int state = 0;
    if (game->conf.autospin) {
        if ((game->stage == STAGE_BARS) || (game->stage <= STAGE_BARS_STOP)) {
            state = 2;
        } else if (game->stage == STAGE_BARS_PRIZE) {
            state = 1;
        } else if (game->stage == STAGE_BARS_FREE) {
            state = 2;
        } else if (game->stage == STAGE_BARS_USER) {
            state = 0;
        } else if (game->stage == STAGE_BARS_TAKE) {
            state = 2;
        }
    } else {
        if (game->stage == STAGE_BARS) {
        } else if (game->stage <= STAGE_BARS_STOP) {
            state = 2;
        } else if (game->stage == STAGE_BARS_PRIZE) {
            state = 1;
        } else if (game->stage == STAGE_BARS_FREE) {
            state = 2;
        } else if (game->stage == STAGE_BARS_USER) {
            uint64_t currentTime = currentTimeMillis();
            int doubleBlink = (currentTime - game->takeTime) / 500;
            if ((doubleBlink & 1) == 0) {
                state = 0;
            } else {
                state = 2;
            }
        } else if (game->stage == STAGE_BARS_TAKE) {
            state = 2;
        }
    }
    
    if (state == 1) {
        drawing->foreground = 5;
        if (game->conf.spin != 0) {
            game->num_sm->renderNumber( {1576-7, 928+7}, -30, CString::format(L"%d;%d", game->conf.spin, game->conf.free));
        }
        if (game->conf.prize > 0) {
            game->num_sm->renderNumber( {1576-7, 928+66}, -30, CString::format(L":%d", game->conf.prize));
        }
    } else if (state == 2) {
        drawing->foreground = 5;
        if (game->conf.free != 0) {
            uint64_t currentTime = currentTimeMillis();
            int freeBlink = (currentTime - game->freespinTime) / 500;
            if ((freeBlink >= 8) || ((freeBlink & 1) == 0)) {
                if (game->conf.spin == 0) {
                    game->num_sm->renderNumber( {1576-7, 928+7}, -30, game->conf.free);
                } else {
                    game->num_sm->renderNumber( {1576-7, 928+7}, -30, CString::format(L"%d;%d", game->conf.spin, game->conf.free));
                }
            } else {
                if (game->conf.spin != 0) {
                    CString sFree = game->conf.free;
                    GLfloat len = sFree.m_length;
                    game->num_sm->renderNumber( {1576-7-30*len, 928+7}, -30, CString::format(L"%d;", game->conf.spin));
                }
            }
        }
        game->num_sm->renderNumber( {1176+7, 928+66}, 30, game->conf.total);
    } else {
        drawing->foreground = 6;
        game->num_sm->renderNumber( {1176+7, 928+66}, 30, game->conf.total);
    }
}

void OMGame::onFile(const CString& url, void* object) {
    CString::format(L"FILE: %ls", (wchar_t*)url).log();
}

void OMGame::onStatus(const LoaderStatus& status) {
    CString::format(L"STATUS: %d / %d", status.count, status.total).log();
}

void OMGame::drawBackground(GLEngine* engine){
    OMGame* game = (OMGame*)engine;
    GLRender* context = game->scene->context;

    if (game->stage <= STAGE_BARS) {
        game->bkg_bars->render();
    } else if (game->stage <= STAGE_DOUBLE) {
        game->bkg_double->render();
        if ((game->stage == STAGE_DOUBLE_SHOW) || (game->stage == STAGE_DOUBLE_HIDE) || (game->stage == STAGE_DOUBLE_TAKE)) {
            if (game->conf.dblindex == 0) {
                game->bkg_sun->render();
            } else {
                game->bkg_moon->render();
            }
        }
    }
}

void OMGame::drawSplash(GLEngine* engine) {
    OMGame* game = (OMGame*)engine;
    GLRender* context = game->scene->context;
    
    if (game->stage <= STAGE_BARS) {
        game->text_om->render();
        game->text_mani->render();
        game->text_padme->render();
        game->text_hum->render();
    }
    
    if (game->stage == STAGE_BARS_FREE) {
        if (game->conf.prizefree == 4)
            game->text_free4->render();
        else
            game->text_free8->render();
    }
    
    if (game->stage <= STAGE_BARS) {
    } else if (game->stage <= STAGE_DOUBLE) {
        game->text_doublein->render();
        if ((game->stage != STAGE_DOUBLE_TAKE) || (game->conf.doubleout == 0)) {
            game->text_doubleout->render();
        }
        
        uint64_t currentTime = currentTimeMillis();
        int state = 0;
        if (game->stage == STAGE_DOUBLE_SHOW) {
            int doubleBlink = (currentTime - game->doubleShowTimeout) / 500;
            if ((doubleBlink & 1) != 0) {
                state = 1;
            }
        }
        if (state == 0) {
            game->num->renderNumber( {216-4, 100}, 42, game->conf.doublein );
            if ((game->stage != STAGE_DOUBLE_TAKE) || (game->conf.doubleout == 0)) {
                game->num->renderNumber( {970-4, 100}, 42, game->conf.doubleout );
            }
        }
        
        context->drawRect(0, 178, 1920, 178+152, game->paintShadow);
        game->text_sequence->render();
        GLfloat x = 900;
        if (game->stage == STAGE_DOUBLE_SHOW) {
            uint64_t ofs = (currentTime - game->doubleShowTimeout + 3000) / 2000;
            if (ofs > 1) ofs = 1;
            game->paintSpriteLeftCenter.setLight(ofs, ofs);
            game->sequence->render( {x - 176 + ofs * 176, 178+152/2}, game->conf.dblindex, &game->paintSpriteLeftCenter );
            game->paintSpriteLeftCenter.setLight(1.0 - ofs, 1.0 - ofs);
            game->sequence->render( {x + 176 * 4 + ofs * 176, 178+152/2}, game->conf.sequence[0], &game->paintSpriteLeftCenter );
            game->paintSpriteLeftCenter.setLight(1.0, 1.0);
            for (int i = 0; i < 4; i++) {
                game->sequence->render( {x + 176 * i + ofs * 176, 178+152/2}, game->conf.sequence[4-i], &game->paintSpriteLeftCenter );
            }
        } else {
            for (int i = 0; i < 5; i++) {
                game->sequence->render( {x + 176 * i, 178+152/2}, game->conf.sequence[4-i], &game->paintSpriteLeftCenter );
            }
        }
    }
}

void OMGame::drawEffectsBars(GLEngine* engine) {
    OMGame* game = (OMGame*)engine;
    GLRender* context = game->scene->context;
    
    game->bars.renderEffects(context);
}

void OMGame::drawEffectsTorsion(GLEngine* engine) {
    OMGame* game = (OMGame*)engine;
    GLRender* context = game->scene->context;
    
    bool mobile = true;
    GLfloat worldTime = (GLfloat)(currentTimeMillis() % 86400000) / 86400000.0;
    
    if (mobile) {
        context->drawEffectMobile(960-700, 606-700, 960+700, 606+700, "mobile_effect_torsion", worldTime, worldTime);
    } else {
        context->drawEffect(960-700, 606-700, 960+700, 606+700, "effect_torsion", worldTime, worldTime);
    }
}

void OMGame::drawEffectsDouble(GLEngine* engine) {
    OMGame* game = (OMGame*)engine;
    GLRender* context = game->scene->context;
    
    bool mobile = false;
    int index = game->conf.dblindex;
    
    uint64_t currentTime = currentTimeMillis();
    GLfloat worldTime = (GLfloat)(currentTime % 86400000) / 86400000.0;
    
    GLfloat localTime;
    if (game->stage != STAGE_DOUBLE_HIDE) {
        localTime = (GLfloat)((currentTime - game->localTime) % 86400000) / 86400000.0;
    } else {
        localTime = (GLfloat)((game->localTime - currentTime) % 86400000) / 86400000.0;
    }
    
    if ((game->stage == STAGE_DOUBLE) || (game->stage == STAGE_DOUBLE_WAIT) || ((game->stage == STAGE_DOUBLE_TAKE) && (game->conf.doubleout != 0))) {
        if (mobile) {
            context->drawEffectMobile(960-320, 606-320, 960+320, 606+320, L"mobile_effect_unknown", 0.0, worldTime);
            context->drawEffectMobile(1050-120, 490-120, 1050+120, 490+120, L"mobile_effect_unknown_star", 0.0, worldTime);
        } else {
            context->drawEffect(960-320, 606-320, 960+320, 606+320, L"effect_unknown", 0.0, worldTime);
            context->drawEffect(1050-120, 490-120, 1050+120, 490+120, L"effect_unknown_star", 0.0, worldTime);
        }
        
        game->question->render();
    } else if ((game->stage == STAGE_DOUBLE_SHOW) || (game->stage == STAGE_DOUBLE_HIDE) || (game->stage == STAGE_DOUBLE_TAKE)) {
        if (mobile) {
            if (index == 0) {
                context->drawEffectMobile(960-320, 606-320, 960+320, 606+320, L"mobile_effect_sun2", localTime, worldTime);
            } else {
                context->drawEffectMobile(960-320, 606-320, 960+320, 606+320, L"mobile_effect_moon2", localTime, worldTime);
                context->drawEffectMobile(220-200, 450-200, 220+200, 450+200, L"mobile_effect_star1", localTime, worldTime);
                context->drawEffectMobile(450-150, 820-150, 450+150, 820+150, L"mobile_effect_star2", localTime, worldTime + 0.3/86400.0);
                context->drawEffectMobile(1700-200, 750-200, 1700+200, 750+200, L"mobile_effect_star1", localTime, worldTime + 0.6/86400.0);
            }
            
            context->drawEffectMobile(1050-120, 490-120, 1050+120, 490+120, L"mobile_effect_unknown_star", localTime, worldTime);
        } else {
            if (index == 0) {
                context->drawEffect(960-320, 606-320, 960+320, 606+320, L"effect_sun2", localTime, worldTime);
            } else {
                context->drawEffect(960-320, 606-320, 960+320, 606+320, L"effect_moon2", localTime, worldTime);
                context->drawEffect(220-200, 450-200, 220+200, 450+200, L"effect_star1", localTime, worldTime);
                context->drawEffect(450-150, 820-150, 450+150, 820+150, L"effect_star2", localTime, worldTime + 0.3/86400.0);
                context->drawEffect(1700-200, 750-200, 1700+200, 750+200, L"effect_star1", localTime, worldTime + 0.6/86400.0);
            }
            
            context->drawEffect(1050-120, 490-120, 1050+120, 490+120, L"effect_unknown_star", localTime, worldTime);
        }
    }
}

void OMGame::drawEffectSwitch(GLEngine* engine) {
    OMGame* game = (OMGame*)engine;
    GLRender* context = game->scene->context;
    
    if (game->nextTime != 0) {
        uint64_t currentTime = currentTimeMillis();
        GLfloat ofs = (GLfloat)(currentTime - game->nextTime) / 500.0;
        if (ofs >= 1) {
            game->nextTime = 0;
        } else {
            game->paintSwitch.setLight(0, 1.0 - clamp(fabsf(ofs), 0.0, 1.0));
            context->drawRect(0, 0, 1920, 1080, game->paintSwitch);
        }
    }
}

void OMGame::onReady() {
    CString::format(L"READY!").log();
    
    //NSBundle* bundle = [NSBundle bundleWithIdentifier:@"com.jappsy.libGameOM"];
    NSBundle* bundle = [NSBundle mainBundle];
    void* library = (__bridge void*)bundle;
    
    // Подготавливаем шейдеры
    
    const char *sOMShadersRes =
        #include "OMShaders.res"
    ;
    
    if (!context->createShaders(sOMShadersRes, library)) {
        shutdown();
    }
    
    // Подготавливаем модели

    if (!context->createModels("{}", library)) {
        shutdown();
    }
    
    // Подготавливаем спрайты из текстур
    
    const char *sOMSpritesRes =
        #include "OMSprites.res"
    ;
    
    if (!context->createSprites(sOMSpritesRes)) {
        shutdown();
    }
    
    // Подготавливаем картинки из спрайтов
    
    const char *sOMDrawingsRes =
        #include "OMDrawings.res"
    ;
    
    if (!context->createDrawings(sOMDrawingsRes)) {
        shutdown();
    }
    
    context->drawings->get(L"text_sequence")->setPaint(paintSpriteLeftCenter);
    context->drawings->get(L"dbl_sun")->setPaint(paintSpriteCenter);
    context->drawings->get(L"dbl_moon")->setPaint(paintSpriteCenter);
    context->drawings->get(L"question")->setPaint(paintSpriteCenter);
    
    // Добавляем наложения на кнопки
    context->drawings->get(L"btn_grace")->onrender = renderButtonGrace;
    context->drawings->get(L"btn_donate")->onrender = renderButtonDonate;
    context->drawings->get(L"btn_double")->onrender = renderButtonDouble;
    
    // Задаем свет для спрайтов по умолчанию
    context->light.set( {1.0, 1.0, 1.0, 1.1 } );
    
    // Создаем и описываем сцену с освещением и камерами
    scene = context->scenes->createScene(L"main");
    scene->ambient.set( { 0.0, 0.0, 0.0 } );
    
    GLCamera* cam = context->cameras->createCamera(L"cam");
    cam->size(1920, 1080)->perspective(45, 0.1, 100)->lookAt( {317.207, 347.124, 1351.136}, {317.207, 347.124, -249.692}, {0,1,0} );
    scene->setCamera(cam);
    
    { // Освещение сцены
        scene->lights->createLight(L"Omni001")->omni( {-554.5, 701.5, -246}, {1, 1, 1}, 0.54, 0.0, 0.0, false );
        scene->lights->createLight(L"Omni002")->omni( {1183.5, 378.5, 109.5}, {1, 1, 1}, 0.3, 0.0, 0.0, false );
        scene->lights->createLight(L"Spot001")->spot( {783.9, 359.2, 619.6}, {47.2, 256.3, -536.5}, {1, 1, 1}, 1.0, 111.7, 2.0, false );
        scene->lights->createLight(L"Spot002")->spot( {-182, 578.2, 488}, {256.4, 352.5, -830.5}, {1, 1, 1}, 0.63, 77.2, 2.0, false );
    }
    
    Vector<GLObject*>* backLayer = scene->createLayer();
    Vector<GLObject*>* modelLayer = scene->createLayer();
    Vector<GLObject*>* effectLayer = scene->createLayer();
    Vector<GLObject*>* rocketLayer = scene->createLayer();
    Vector<GLObject*>* splashLayer = scene->createLayer();
    Vector<GLObject*>* buttonLayer = scene->createLayer();
    Vector<GLObject*>* blackLayer = scene->createLayer();
    
    /*
    { // Феерверки
        this.rocketslist = [];
        
        for (var i = 0; i < 10; i++) {
            var name = "rocket" + i;
            var s = this.context.particles.createParticleSystem(name);
            s.createRocket(0, new Vec3([0,0,0]));
            s.generate();
            this.rocketslist.push(name);
            
            var p = new Vec3().subtract(scene.camera.target, [0,250,0]);
            scene.createLayerObject(rocketLayer, name).setParticleSystem(name)->objectMatrix.translate(p);
        }
        
        //scene.startParticlesGroup(this.rocketslist, 2);
        scene.visibleGroup(this.rocketslist, true);
    }
    */
    
    { // Модели
        scene->createLayerObject(modelLayer, L"ball_0")->setModel(L"ball")->objectMatrix.translate( {1.947, 620.468, -252.468} );
        scene->createLayerObject(modelLayer, L"cap_0")->setModel(L"cap")->objectMatrix.translate( {4.12, 572.537, -249.301} );
        scene->createLayerObject(modelLayer, L"bar_00")->setModel(L"bar")->objectMatrix.translate( {3.397, 477.718, -249.692} ).multiply(Mat4().rotate({0,-1,0}, 180));
        scene->createLayerObject(modelLayer, L"separator_00")->setModel(L"separator")->objectMatrix.translate( {3.049, 401.208, -248.473} );
        scene->createLayerObject(modelLayer, L"bar_01")->setModel(L"bar")->objectMatrix.translate( {3.397, 327.124, -249.692} ).multiply(Mat4().rotate({0,-1,0}, 90));
        scene->createLayerObject(modelLayer, L"separator_01")->setModel(L"separator")->objectMatrix.translate( {3.049, 252.739, -248.473} );
        scene->createLayerObject(modelLayer, L"bar_02")->setModel(L"bar")->objectMatrix.translate( {3.397, 179.488, -249.692} );
        scene->createLayerObject(modelLayer, L"separator_02")->setModel(L"separator")->objectMatrix.translate( {3.049, 106.063, -248.473} );
        
        scene->createLayerObject(modelLayer, L"ball_1")->setModel(L"ball")->objectMatrix.translate( {315.811, 620.468, -252.468} );
        scene->createLayerObject(modelLayer, L"cap_1")->setModel(L"cap")->objectMatrix.translate( {317.984, 572.537, -249.301} );
        scene->createLayerObject(modelLayer, L"bar_10")->setModel(L"bar")->objectMatrix.translate( {317.207, 477.718, -249.692} ).multiply(Mat4().rotate({0,-1,0}, 180));
        scene->createLayerObject(modelLayer, L"separator_10")->setModel(L"separator")->objectMatrix.translate( {316.914, 401.208, -248.473} );
        scene->createLayerObject(modelLayer, L"bar_11")->setModel(L"bar")->objectMatrix.translate( {317.207, 327.124, -249.692} ).multiply(Mat4().rotate({0,-1,0}, 90));
        scene->createLayerObject(modelLayer, L"separator_11")->setModel(L"separator")->objectMatrix.translate( {316.914, 252.739, -248.473} );
        scene->createLayerObject(modelLayer, L"bar_12")->setModel(L"bar")->objectMatrix.translate( {317.207, 179.488, -249.692} );
        scene->createLayerObject(modelLayer, L"separator_12")->setModel(L"separator")->objectMatrix.translate( {316.914, 106.063, -248.473} );
        
        scene->createLayerObject(modelLayer, L"ball_2")->setModel(L"ball")->objectMatrix.translate( {626.263, 620.468, -252.468} );
        scene->createLayerObject(modelLayer, L"cap_2")->setModel(L"cap")->objectMatrix.translate( {628.436, 572.537, -249.301} );
        scene->createLayerObject(modelLayer, L"bar_20")->setModel(L"bar")->objectMatrix.translate( {632.114, 477.718, -249.692} ).multiply(Mat4().rotate({0,-1,0}, 180));
        scene->createLayerObject(modelLayer, L"separator_20")->setModel(L"separator")->objectMatrix.translate( {631.676, 401.208, -248.473} );
        scene->createLayerObject(modelLayer, L"bar_21")->setModel(L"bar")->objectMatrix.translate( {632.114, 327.124, -249.692} ).multiply(Mat4().rotate({0,-1,0}, 90));
        scene->createLayerObject(modelLayer, L"separator_21")->setModel(L"separator")->objectMatrix.translate( {631.676, 252.739, -248.473} );
        scene->createLayerObject(modelLayer, L"bar_22")->setModel(L"bar")->objectMatrix.translate( {632.114, 179.488, -249.692} );
        scene->createLayerObject(modelLayer, L"separator_22")->setModel(L"separator")->objectMatrix.translate( {631.676, 106.063, -248.473} );
        
        // Список 3д объектов на первом барабане
        groups.barlist1.push( scene->objects->get(L"ball_0") );
        groups.barlist1.push( scene->objects->get(L"cap_0") );
        groups.barlist1.push( scene->objects->get(L"bar_00") );
        groups.barlist1.push( scene->objects->get(L"bar_01") );
        groups.barlist1.push( scene->objects->get(L"bar_02") );
        groups.barlist1.push( scene->objects->get(L"separator_00") );
        groups.barlist1.push( scene->objects->get(L"separator_01") );
        groups.barlist1.push( scene->objects->get(L"separator_02") );
        
        // Список 3д объектов на втором барабане
        groups.barlist2.push( scene->objects->get(L"ball_1") );
        groups.barlist2.push( scene->objects->get(L"cap_1") );
        groups.barlist2.push( scene->objects->get(L"bar_10") );
        groups.barlist2.push( scene->objects->get(L"bar_11") );
        groups.barlist2.push( scene->objects->get(L"bar_12") );
        groups.barlist2.push( scene->objects->get(L"separator_10") );
        groups.barlist2.push( scene->objects->get(L"separator_11") );
        groups.barlist2.push( scene->objects->get(L"separator_12") );
        
        // Список 3д объектов на третьем барабане
        groups.barlist3.push( scene->objects->get(L"ball_2") );
        groups.barlist3.push( scene->objects->get(L"cap_2") );
        groups.barlist3.push( scene->objects->get(L"bar_20") );
        groups.barlist3.push( scene->objects->get(L"bar_21") );
        groups.barlist3.push( scene->objects->get(L"bar_22") );
        groups.barlist3.push( scene->objects->get(L"separator_20") );
        groups.barlist3.push( scene->objects->get(L"separator_21") );
        groups.barlist3.push( scene->objects->get(L"separator_22") );
        
        groups.barlist.push(groups.barlist1);
        groups.barlist.push(groups.barlist2);
        groups.barlist.push(groups.barlist3);
        
        scene->rotateGroup(groups.barlist1, Vec3( {0,-1,0} ).normalize(), -11, true);
        scene->rotateGroup(groups.barlist3, Vec3( {0,-1,0} ).normalize(), 11, true);
        //scene->rotateGroup(groups.barlist, Vec3( {0,-1,0} ).normalize(), 45*3);
    }
    
    { // Фон
        bkg_bars = context->drawings->get(L"bkg_bars");
        bkg_double = context->drawings->get(L"bkg_double");
        bkg_sun = context->drawings->get(L"bkg_sun");
        bkg_moon = context->drawings->get(L"bkg_moon");

        scene->createLayerObject(backLayer, L"background")->setFunc(drawBackground);
    }
    
    { // Тексты
        num = context->sprites->get(L"num");

        text_om = context->drawings->get(L"text_om");
        text_mani = context->drawings->get(L"text_mani");
        text_padme = context->drawings->get(L"text_padme");
        text_hum = context->drawings->get(L"text_hum");
        text_free4 = context->drawings->get(L"text_free4");
        text_free8 = context->drawings->get(L"text_free8");
        text_doublein = context->drawings->get(L"text_doublein");
        text_doubleout = context->drawings->get(L"text_doubleout");
        text_sequence = context->drawings->get(L"text_sequence");
        sequence = context->sprites->get(L"sequence");
        
        scene->createLayerObject(splashLayer, L"splash")->setFunc(drawSplash);
    }
    
    { // Анимация на барабанах
        scene->createLayerObject(effectLayer, L"effects_bar")->setFunc(drawEffectsBars);

        groups.bareffects.push( scene->objects->get(L"effects_bar") );
    }
    
    { // Торсион
        scene->createLayerObject(effectLayer, L"effects_torsion")->setFunc(drawEffectsTorsion);

        groups.dbleffects.push( scene->objects->get(L"effects_torsion") );
    }
    
    { // Картинки и Кнопки
        scene->createLayerDrawing(splashLayer, "anchor1");
        scene->createLayerDrawing(splashLayer, "anchor2");
        
        groups.btnlist.push( scene->createLayerDrawing(buttonLayer, L"btn_rewards") );
        groups.btnlist.push( scene->createLayerDrawing(buttonLayer, L"btn_grace") );
        groups.btnlist.push( scene->createLayerDrawing(buttonLayer, L"btn_donate") );
        groups.btnlist.push( scene->createLayerDrawing(buttonLayer, L"btn_auto") );
        groups.btnlist.push( scene->createLayerDrawing(buttonLayer, L"btn_info") );
        groups.btnlist.push( scene->createLayerDrawing(buttonLayer, L"btn_double") );
        groups.btnlist.push( scene->createLayerDrawing(buttonLayer, L"btn_start") );
        groups.btnlist.push( scene->createLayerDrawing(buttonLayer, L"btn_take") );
        groups.btnlist.push( scene->createLayerDrawing(buttonLayer, L"btn_stop") );
        groups.btnlist.push( scene->createLayerDrawing(buttonLayer, L"btn_sun") );
        groups.btnlist.push( scene->createLayerDrawing(buttonLayer, L"btn_moon") );
        scene->objects->trackEvents(groups.btnlist, onButtonEvent);
        
        groups.dbllist.push( scene->createLayerDrawing(effectLayer, L"dbl_sun") );
        groups.dbllist.push( scene->createLayerDrawing(effectLayer, L"dbl_moon") );
        scene->objects->trackEvents(groups.dbllist, onDoubleEvent);
        
        groups.dbleffects.push( scene->objects->get(L"dbl_sun") );
        groups.dbleffects.push( scene->objects->get(L"dbl_moon") );
    }
    
    { // Анимация в удвоении
        question = context->drawings->get(L"question");
        
        scene->createLayerObject(effectLayer, L"effects_double")->setFunc(drawEffectsDouble);

        groups.dbleffects.push( scene->objects->get(L"effects_double") );
    }

    { // Затемнение экрана при переключении сцены
        scene->createLayerObject(blackLayer, L"effects_switch")->setFunc(drawEffectSwitch);
    }
    
    num_sm = context->sprites->get(L"num_sm");
    
    dbl_sun = context->drawings->get(L"dbl_sun");
    dbl_moon = context->drawings->get(L"dbl_moon");

    updateStage(STAGE_BARS, NULL, 0, true);
    doubleReset(this);
    
    trackBarData1.game = this;
    trackBarData2.game = this;
    trackBarData3.game = this;
    context->touchScreen->trackEvent("bar1", 276, 192, 456, 720, onTrackBar, &trackBarData1);
    context->touchScreen->trackEvent("bar2", 732, 192, 456, 720, onTrackBar, &trackBarData2);
    context->touchScreen->trackEvent("bar3", 1188, 192, 456, 720, onTrackBar, &trackBarData3);
    
    ready = true;
}

void OMGame::onError(const CString& error) {
    CString::format(L"ERROR: %ls", (wchar_t*)error).log();
}

OMGame::OMGame() {
    cache = new Cache(L"om");
    
    conf.sequence.growstep(5);
    conf.sequence.push(1);
    conf.sequence.push(1);
    conf.sequence.push(0);
    conf.sequence.push(0);
    conf.sequence.push(0);
    
    const char *sOMLoadRes =
        #include "OMLoad.res"
    ;
    
    paint.setColor(0x80FF0000).setStroke(3, 0x800000FF);
    paintShadow.setColor(0x80000000);
    paintSwitch.setColor(0x00000000);
    paintSpriteCenter.setColor(0xFFFFFFFF).setAlignX(GLAlignX::CENTER).setAlignY(GLAlignY::MIDDLE);
    paintSpriteLeftCenter.setColor(0xFFFFFFFF).setAlignX(GLAlignX::LEFT).setAlignY(GLAlignY::MIDDLE);
    
    this->setBasePath(L"https://www.cox.ru/res/om/");
    this->load(sOMLoadRes);
}
