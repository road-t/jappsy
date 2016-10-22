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

#include <opengl/uGLLight.h>

void OMGame::playMantra() {
    rollTimeout = currentTimeMillis() + ((mantra == 0) ? 3900 : 1000);
    activeMantra = context->mixer->get(CString::format(L"mantra_0%d", mantra));
    activeMantra->mixPlay(false, 1.0, true, false);
    mantra++;
    if (mantra > 4) mantra = 0;
}

void OMGame::updateGrace(GLfloat diff) {
    if (isnan(diff)) diff = 0.0;

    if (diff != 0.0) {
        if (diff >= 0.0) {
            conf.pointsin += diff;
        } else {
            GLfloat left = conf.pointsin - conf.points;
            if (left > diff) {
                conf.pointsin += (diff - left);
            }
        }
        conf.points += diff;
    }
    
    CString ss = CString::format(L"GAINED GRACE: %.2f", conf.pointsin);
    ss = ss.replace(L".", L"<small>.");
    ss.concat(L"</small>");
    
    webScript(OMVIEW_HELP, CString::format(L"updateGrace('%ls');", (wchar_t*)ss));
}

void OMGame::startBars(bool freespin, bool query) {
#ifndef OMDEMO
    if ((!freespin) && (query)) {
        queryGenerateBars();
    }
#endif
    
    bars.start();
    updateStage(STAGE_BARS_WAIT);
    rollTimeout = currentTimeMillis() + 3000;
    
    if (freespin) {
        LOG("Фриспин - Ускорение всех барабанов");
    } else if (conf.autospin) {
        LOG("Автоспин - Ускорение всех барабанов");
        if (query) {
            updateGrace(-conf.bet);
        }
    } else {
        LOG("Ускорение всех барабанов");
        if (query) {
            updateGrace(-conf.bet);
        }
    }
    
    context->mixer->get(L"stone_move_long")->mixPlayTimeout(true, 1.0, false, false, 500, 250);
    playMantra();
}

void OMGame::updateSounds(OMGameStage stage) {
    if (stage == STAGE_DOUBLE) {
        context->mixer->get(L"space")->mixPlay(true, 1.0, true, false);
    } else if ((stage <= STAGE_BARS) && (this->stage > STAGE_BARS)) {
        context->mixer->get(L"space")->mixPlayTimeout(true, 1.0, false, false, 0, 500);
    }
}

void OMGame::updateStage(OMGameStage stage, onNextCallback callback, uint64_t timeout, bool fade) {
    uint64_t nextTimeout = currentTimeMillis() + timeout;
    nextTime = fade ? nextTimeout : 0;

    this->nextTimeout = nextTimeout;
    nextStage = stage;
    nextCallback = callback;

    /* FIXED multithread update from loader
    if (timeout == 0) {
        updateSounds(this->nextStage);
        
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
     */
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
    
    if (!minimized) {
        Vector<GLObject*> buttons;
        buttons.push( scene->objects->get(L"btn_rewards") );
        buttons.push( scene->objects->get(L"btn_grace") );
        buttons.push( scene->objects->get(L"btn_donate") );
        if (stage <= STAGE_BARS) {
            if (conf.autospin) {
                buttons.push( scene->objects->get(L"btn_double") );
                buttons.push( scene->objects->get(L"btn_stop") );
            
                donlistSwitch = false;
            } else {
                if (stage == STAGE_BARS) {
                    buttons.push( scene->objects->get(L"btn_auto") );
                    buttons.push( scene->objects->get(L"btn_info") );
                    buttons.push( scene->objects->get(L"btn_start") );
                } else {
                    buttons.push( scene->objects->get(L"btn_double") );
                    buttons.push( scene->objects->get(L"btn_take") );
                
                    donlistSwitch = false;
                }
            }
        } else if (stage <= STAGE_DOUBLE) {
            buttons.push( scene->objects->get(L"btn_sun") );
            buttons.push( scene->objects->get(L"btn_moon") );
            buttons.push( scene->objects->get(L"btn_take") );
        
            donlistSwitch = false;
        }
        scene->visibleGroup(buttons, true);
        scene->visibleGroup(groups.donlist, donlistSwitch);
    } else {
        scene->visibleGroup(groups.donlist, false);
    }
}

//static int color = 0;

static const Vec3 barsRotateVector = Vec3({0.0, -1, 0.0}).normalize();

void OMGame::onFrame(GLRender* context) {
    glClearColor(0.2f, 0.3f, 0.5f, 1.0f);
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
/*
    float c = (float)color / 255.0f;
    glClearColor(0.0f, c, 0.0f, 1.0f);
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    
    color++;
    if (color >= 256) color = 0;
*/
    if (ready == 1) {
        if (status.total > 0) {
            paintLoading.setColor(0xFF204080);
            context->drawRect(0, 1000, 1920, 1080, paintLoading, context->cameras->background);
            paintLoading.setColor(0xFF4080FF);
            context->drawRect(0, 1000, 1920 * status.count / status.total, 1080, paintLoading, context->cameras->background);
        }
    } else if (ready == 2) {
        uint64_t currentTime = currentTimeMillis();
        
        // Переключение сцены по таймауту
        if ((nextTimeout != 0) && (currentTime >= nextTimeout)) {
            nextTimeout = 0;
            updateSounds(nextStage);
            stage = nextStage;
            
            updateStageVisibility();
            
            if (nextCallback)
                nextCallback(this);
        }
        
        if (stage <= STAGE_BARS) {
            if (bars.update()) {
                scene->rotateGroup(groups.barlist1, barsRotateVector, bars.rotation[0].rotate);
                scene->rotateGroup(groups.barlist2, barsRotateVector, bars.rotation[1].rotate);
                scene->rotateGroup(groups.barlist3, barsRotateVector, bars.rotation[2].rotate);
            }
        }
        
        if (stage == STAGE_UNKNOWN) {
            // Ожидаем конфиг с сервера
        }
        else if (nextTimeout == 0) {
            if (stage <= STAGE_BARS) {
                if ((rollTimeout != 0) && (currentTime >= rollTimeout)) {
                    bool stop = false;
#ifdef OMDEMO
                    // Демо ход
                    int check = bars.stopRandom();
                    //LOG("Grace: " + check);
                    stop = true;
#else
                    if ((status.query == 0) && (onUpdateConfig(STAGE_BARS_STOP))) {
                        // Формируем ход на барабанах на основе полученных данных от сервера
                        try {
                            CString ofs = serverConfig->get(L"state")->optString(L"last_bars_gen", L"").substring(conf.spin * 3, (conf.spin + 1) * 3);
                            if (ofs.m_length == 3) {
                                /*int check = */bars.stop((wchar_t*)ofs);
                                //LOG("Grace: " + check);
                                stop = true;
                            }
                        } catch (...) {
                        }
                    } else {
                        // TODO: Сколько ждать ответ сервера?
                        // bars.stopCancel();
                    }
#endif
                    
                    if (stop) {
                        rollTimeout = 0;
                        LOG("Торможение всех барабанов");
                        updateStage(STAGE_BARS_STOP);
                    } else {
                        rollTimeout = currentTime + 1000; // TODO: Как правильно рассчитать запуск следующей мантры синхронно?
                        // playMantra();
                    }
                }
                
                if ((activeMantra != NULL) && (activeMantra->mixPlaying())) {
                    // Пропустить проверку остановки барабанов
                }
                else
                if (bars.stopped()) {
                    int sum = bars.calculate();
                    //LOG("Check: " + sum);
                    LOG("Барабаны остановились");
                    
                    switch (sum) {
                        case -8: context->mixer->get(L"sun_free_8")->mixPlay(false, 1.0, true, false); break;
                        case -4: context->mixer->get(L"moon_free_4")->mixPlay(false, 1.0, true, false); break;
                        case 4: context->mixer->get(L"moon_4")->mixPlay(false, 1.0, true, false); break;
                        case 8: context->mixer->get(L"sun_8")->mixPlay(false, 1.0, true, false); break;
                        case 44: context->mixer->get(L"moon_44")->mixPlay(false, 1.0, true, false); break;
                        case 88: context->mixer->get(L"sun_88")->mixPlay(false, 1.0, true, false); break;
                        case 444: context->mixer->get(L"moon_444")->mixPlay(false, 1.0, true, false); break;
                        case 888: context->mixer->get(L"sun_888")->mixPlay(false, 1.0, true, false); break;
                        default:;
                    }
                    
                    if (sum == 0) {
                        if (conf.free != 0) {
                            prizeTimeout = currentTime - 1;
                        } else {
#ifndef OMDEMO
                            queryTake();
#endif
                            updateStage(STAGE_BARS);
                        }
                    } else if (sum < 0) {
                        LOG("Фриспин +%d", (-sum));
                        prizeTimeout = currentTime + 8000; // Время отображения бонус фриспин
                        updateStage(STAGE_BARS_FREE);
                        
                        conf.prizefree = -sum;
                        conf.free -= sum;
                        freespinTime = currentTime;
                        
                        LOG("Запуск феерверков");
                        scene->startParticlesGroup(groups.rocketslist, 1);
                    } else if (sum > 0) {
                        LOG("Приз +%d", sum);
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
            else if (stage <= STAGE_DOUBLE) {
                if ((selectTimeout != 0) && (currentTime >= selectTimeout)) {
                    bool stop = false;
#ifdef OMDEMO
                    conf.dblindex = Rotation::generateDouble();
                    stop = true;
#else
                    if ((status.query == 0) && (onUpdateConfig(STAGE_DOUBLE_SHOW))) {
                        conf.dblindex = (int32_t)serverConfig->get(L"state")->optInt(L"last_double_gen", 0);
                        stop = true;
                    } else {
                        // TODO: Сколько ждать ответа от сервера?
                    }
#endif
                    
                    if (stop) {
                        if (conf.dblindex == 0) {
                            context->mixer->get(L"sun_double")->mixPlay(false, 1.0, true, false);
                        } else {
                            context->mixer->get(L"moon_double")->mixPlay(false, 1.0, true, false);
                        }
                        
                        LOG("Удвоение - Открытие затмения");
                        selectTimeout = 0;
                        localTime = currentTime;
                        doubleShowTimeout = currentTime + 3000; // Время открытия затмения
                        updateStage(STAGE_DOUBLE_SHOW);
                    
                        GLDrawing* drawing;
                        if (conf.dblindex == 0) {
                            drawing = bkg_sun;
                        } else {
                            drawing = bkg_moon;
                        }
                        GLAnimation::createFadeIn(drawing, 0.0, 1.0, 2000);
                    } else {
                        selectTimeout = currentTime + 1000;
                    }
                }
                
                if ((doubleShowTimeout != 0) && (doubleShowTimeout < currentTime)) {
                    doubleShowTimeout = 0;
                    
                    (void)conf.sequence.shift();
                    conf.sequence.push((int8_t)conf.dblindex);
                    
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
#ifndef OMDEMO
                    queryTake();
#endif
                    
                    updateStage(STAGE_BARS);
                    
                    conf.prize = 0;
                    conf.free = 0;
                    conf.spin = 0;
                } else if (takeTimeout < currentTime) {
                    static int values[9] = { 1, 3, 5, 8, 13, 33, 108, 144, 369 };
                    int count = 0;
                    if (conf.total < 1.0) {
                    } else if (conf.total < 3.0) {
                        count = 1;
                    } else if (conf.total < 5.0) {
                        count = 2;
                    } else if (conf.total < 8.0) {
                        count = 3;
                    } else if (conf.total < 13.0) {
                        count = 4;
                    } else if (conf.total < 33.0) {
                        count = 5;
                    } else if (conf.total < 108.0) {
                        count = 6;
                    } else if (conf.total < 144.0) {
                        count = 7;
                    } else if (conf.total < 369.0) {
                        count = 8;
                    } else {
                        count = 9;
                    }
                    if (count != 0) {
                        int idx = randomi(count);
                        conf.total -= values[idx];
                        updateGrace(values[idx]);
                    } else {
                        updateGrace(conf.total);
                        conf.total = 0;
                    }
                    
                    GLfloat speed = (GLfloat)(conf.total) / 5000.0f + 1.0f;
                    takeTimeout = currentTime + (int)(50.0 / speed);
                }
            }
            
            else if (stage == STAGE_DOUBLE_TAKE) {
                if ((conf.doublein == 0) || (conf.doubleout == 0)) {
#ifndef OMDEMO
                    queryTake();
#endif

                    LOG("Удвоение - Выход");
                    updateStage(STAGE_BARS, doubleReset, (conf.doubleout == 0) ? 3000 : 500, true);
                    
                    conf.total = 0;
                    conf.prize = 0;
                    conf.free = 0;
                    conf.spin = 0;
                } else if (takeTimeout < currentTime) {
                    static int values[9] = { 1, 3, 5, 8, 13, 33, 108, 144, 369 };
                    int count = 0;
                    if (conf.doublein < 1.0) {
                    } else if (conf.doublein < 3.0) {
                        count = 1;
                    } else if (conf.doublein < 5.0) {
                        count = 2;
                    } else if (conf.doublein < 8.0) {
                        count = 3;
                    } else if (conf.doublein < 13.0) {
                        count = 4;
                    } else if (conf.doublein < 33.0) {
                        count = 5;
                    } else if (conf.doublein < 108.0) {
                        count = 6;
                    } else if (conf.doublein < 144.0) {
                        count = 7;
                    } else if (conf.doublein < 369.0) {
                        count = 8;
                    } else {
                        count = 9;
                    }
                    if (count != 0) {
                        int idx = randomi(count);
                        conf.doublein -= values[idx];
                        updateGrace(values[idx]);
                    } else {
                        updateGrace(conf.doublein);
                        conf.doublein = 0;
                    }
                    
                    GLfloat speed = (GLfloat)(conf.total) / 5000.0f + 1.0f;
                    takeTimeout = currentTime + (int)(50.0 / speed);
                }
            }
            
            if ((conf.autospin) && (stage == STAGE_BARS)) {
                if (conf.points >= conf.bet) {
                    startBars(false);
                } else {
                    conf.autospin = false;
                    updateStageVisibility();
                }
            }
        }
        
        scene->render();
        context->mixer->update();
        
#ifndef OMDEMO
        if ((nextConfigUpdate != 0) && (currentTime >= nextConfigUpdate)) {
            nextConfigUpdate = 0;
            updateConfig();
        }
#endif
        
        static int fpscounter = 0;
        fpscounter++;
        if (fpscounter >= 60) {
            fpscounter = 0;

            LOG("FPS: %d", context->frame->fps);
        }
    }
}

void OMGame::onResize(int width, int height) {
//    context->touchScreen->update(1920, 1080); // independent size
    context->touchScreen->update(width, height);
    if (ready == 2) {
        updateStageVisibility();
    }
}

void OMGame::onTouch(const CString& event) {
    if (nextTimeout == 0) {
        if (minimized) {
            updateState(OMVIEW_RESTORE | OMVIEW_ANIMATE);
            return;
        }
        
#ifdef DEBUG
        if (event.indexOf("click ") < 0) {
            event.log();
        }
#endif
        
        if (event == L"swipe long left") {
            if (stage == STAGE_BARS) {
                if (conf.points >= conf.bet) {
                    startBars(false);
                }
            }
        } else if ((event == L"swipe bottom") || (event == L"swipe long bottom")) {
            if (!minimized) {
                updateState(OMVIEW_MINIMIZE | OMVIEW_ANIMATE);
            }
        }
    }
}

bool OMGame::onTrackBar(const CString& event, const GLTouchPoint* cur, const GLTouchPoint* delta, const GLTouchPoint* speed, void* userData) {
    TrackBarData* data = (TrackBarData*)userData;
    OMGame* game = data->game;
    int index = data->index;
    
    if (game->minimized) {
        return false;
    }
    
    if (game->nextTimeout == 0) {
        if (game->stage == STAGE_BARS) {
            //GLScene* scene = game->scene;
            
            game->bars.clear();
            if (event.startsWith(L"move")) {
                game->bars.rotation[index].userMove(-delta->x / 4.0f);
                game->context->mixer->get(L"stone_move_long")->mixPlayTimeout(true, 1.0, false, false, 100, 100);
            } else if (event.startsWith(L"leave")) {
                game->bars.rotation[index].userRotate(-delta->x / 4.0f, speed->x * 250.0f);
                if (speed->x >= 1.0) {
                    game->context->mixer->get(L"stone_move_long")->mixPlayTimeout(true, 1.0, false, false, 500, 250);
                }
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
#ifdef DEBUG
            CString::format(L"Нажата кнопка %ls", (wchar_t*)event).log();
#endif
            game->context->mixer->get(L"button")->mixPlay(false, 1.0, true, false);
        } else if (event.startsWith("move ")) {
        } else if (event.startsWith("leave ")) {
        } else {
#ifdef DEBUG
            CString::format(L"Действие по кнопке %ls", (wchar_t*)event).log();
#endif
            if (event == L"click btn_auto") {
                if (game->stage == STAGE_BARS) {
                    if (game->conf.points >= game->conf.bet) {
                        LOG("Включен автоспин");
                        game->conf.autospin = true;
                        game->context->mixer->get(L"button")->mixStop();
                        game->context->mixer->get(L"auto")->mixPlay(false, 1.0, true, false);
                    }
                }
            } else if (event == L"click btn_start") {
                if (game->stage == STAGE_BARS) {
                    if (game->conf.points >= game->conf.bet) {
                        game->startBars(false);
                    }
                }
            } else if (event == L"click btn_take") {
                if (game->stage == STAGE_BARS_USER) {
                    LOG("Забрть приз");
                    game->updateStage(STAGE_BARS_TAKE);
                } else if (game->stage == STAGE_BARS_TAKE) {
                    LOG("Забран весь приз без ожидания перетекания");
                    game->updateGrace(game->conf.total);
                    game->conf.total = 0;
                } else if (game->stage == STAGE_DOUBLE) {
                    LOG("Удвоение - Забрать приз");
                    game->updateStage(STAGE_DOUBLE_TAKE);
                    
                    GLAnimation::createFadeOut(game->question, 0.0, 1.0, 500);
                } else if (game->stage == STAGE_DOUBLE_TAKE) {
                    LOG("Забран весь приз без ожидания перетекания");
                    if (game->conf.doubleout != 0) {
                        game->updateGrace(game->conf.doublein);
                        game->conf.doublein = 0;
                    }
                    game->conf.total = 0;
                }
            } else if (event == L"click btn_stop") {
                if (game->conf.autospin) {
                    LOG("Отключен автоспин");
                    game->conf.autospin = false;
                    game->updateStage(game->stage);
                }
            } else if (event == L"click btn_double") {
                if (game->stage == STAGE_BARS_USER) {
                    LOG("Вход в удвоение");
                    game->conf.doublein = game->conf.total;
                    game->conf.doubleout = game->conf.total * 2;
                    game->updateStage(STAGE_DOUBLE, NULL, 500, true);
                }
            } else if (event == L"click btn_sun") {
                if (game->stage == STAGE_DOUBLE) {
                    game->doubleSelect(0);
                }
            } else if (event == L"click btn_moon") {
                if (game->stage == STAGE_DOUBLE) {
                    game->doubleSelect(1);
                }
            } else if (event == L"click btn_info") {
                game->updateState(OMVIEW_HELP | OMVIEW_ANIMATE);
                game->webScript(OMVIEW_HELP, L"document.gameInfo.switchTo(0, true);");
            } else if (event == L"click btn_rewards") {
                game->updateState(OMVIEW_HELP | OMVIEW_ANIMATE);
                game->webScript(OMVIEW_HELP, L"document.gameInfo.switchTo(1, true);");
            } else if (event == L"click btn_donate") {
                if (game->stage == STAGE_BARS) {
                    game->donlistSwitch = true;
                    game->scene->visibleGroup(game->groups.donlist, game->donlistSwitch);
                }
            } else if (event.startsWith(L"click btn_donate")) {
                GLfloat v = 0.1;
                if (event.endsWith(L"donate0")) {
                    v = 0.1;
                } else if (event.endsWith(L"donate1")) {
                    v = 0.2;
                } else if (event.endsWith(L"donate2")) {
                    v = 0.5;
                } else if (event.endsWith(L"donate3")) {
                    v = 1.0;
                } else if (event.endsWith(L"donate4")) {
                    v = 5.0;
                } else if (event.endsWith(L"donate5")) {
                    v = 10.0;
                }
                game->conf.bet = v;
                game->donlistSwitch = false;
                game->scene->visibleGroup(game->groups.donlist, game->donlistSwitch);
#ifdef DEBUG
            } else {
                event.log();
#endif
            }
        }
        return true;
    }
    return false;
}

void OMGame::doubleSelect(int index, bool query) {
#ifndef OMDEMO
    if (query) {
        queryGenerateDouble(index);
    }
#endif
    
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
    
    context->mixer->get(L"space")->mixPlay(true, 1.0, false, false);
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
#ifdef DEBUG
            CString::format(L"Нажата кнопка %ls", (wchar_t*)event).log();
#endif
            game->context->mixer->get(L"button")->mixPlay(false, 1.0, true, false);
        }
    } else if (event.indexOf("move ") == 0) {
    } else if (event.indexOf("leave ") == 0) {
        if (game->stage == STAGE_DOUBLE) {
            GLAnimation::createLightOut(drawing, 0.3, 1.0, 500);
        }
    } else {
#ifdef DEBUG
        CString::format(L"Действие по кнопке %ls", (wchar_t*)event).log();
#endif
        if (event == "click dbl_sun") {
            if (game->stage == STAGE_DOUBLE) {
                game->doubleSelect(0);
            }
        } else if (event == "click dbl_moon") {
            if (game->stage == STAGE_DOUBLE) {
                game->doubleSelect(1);
            }
#ifdef DEBUG
        } else {
            event.log();
#endif
        }
    }
    return true;
}

void OMGame::renderNumber12pt(const Vec2& position, GLfloat value, int direction, const wchar_t* prefix) {
    GLfloat step1 = 30;
    GLfloat step2 = 26;
    CString ss;
    if (prefix != NULL) {
        ss = CString::format(L"%ls%.2f", prefix, value);
    } else {
        ss = CString::format(L"%.2f", value);
    }
    wchar_t* ptr1 = (wchar_t*)ss;
    int len1 = ss.m_length - 3;
    wchar_t* ptr2 = ptr1 + len1;
    int len2 = 3;
    ptr2[0] = L'<';
    
    Vec2 pos = position;
    if (direction == 0) {
        pos.x += (GLfloat)(step1 * len1 + step2 * len2) / 2.0 - 18.0;
        pos.y -= (GLfloat)(num_12pt->height) / 2.0;
    } else if (direction > 0) {
        pos.x += (GLfloat)(step1 * len1 + step2 * len2);
    } else {
        pos.x -= (GLfloat)(num_12pt_sub->width) - step2;
    }
    while (--len2 >= 0) {
        pos.x -= step2;
        GLuint v = (uint32_t)(ptr2[len2]) - 48;
        num_12pt_sub->render(pos, v);
    }
    while (--len1 >= 0) {
        pos.x -= step1;
        GLuint v = (uint32_t)(ptr1[len1]) - 48;
        num_12pt->render(pos, v);
    }
}

void OMGame::renderValue12pt(const Vec2& position, const CString& value, int direction) {
    GLfloat step = 30;
    wchar_t* ptr = (wchar_t*)value;
    int len = value.m_length;
    
    Vec2 pos = position;
    if (direction >= 0) {
        pos.x += step * len;
    } else {
        pos.x -= (GLfloat)(num_12pt->width) - step;
    }
    while (--len >= 0) {
        pos.x -= step;
        GLuint v = (uint32_t)(ptr[len]) - 48;
        num_12pt->render(pos, v);
    }
}

void OMGame::renderNumber18pt(const Vec2& position, GLfloat value, int direction, const wchar_t* prefix) {
    GLfloat step1 = 42;
    GLfloat step2 = 36;
    CString ss;
    if (prefix != NULL) {
        ss = CString::format(L"%ls%.2f", prefix, value);
    } else {
        ss = CString::format(L"%.2f", value);
    }
    wchar_t* ptr1 = (wchar_t*)ss;
    int len1 = ss.m_length - 3;
    wchar_t* ptr2 = ptr1 + len1;
    int len2 = 3;
    ptr2[0] = L'<';
    
    Vec2 pos = position;
    if (direction == 0) {
        pos.x += (GLfloat)(step1 * len1 + step2 * len2) / 2.0 - 18.0;
        pos.y -= (GLfloat)(num_18pt->height) / 2.0;
    } else if (direction > 0) {
        pos.x += (GLfloat)(step1 * len1 + step2 * len2);
    } else {
        pos.x -= (GLfloat)(num_18pt_sub->width) - step2;
    }
    while (--len2 >= 0) {
        pos.x -= step2;
        GLuint v = (uint32_t)(ptr2[len2]) - 48;
        num_18pt_sub->render(pos, v);
    }
    while (--len1 >= 0) {
        pos.x -= step1;
        GLuint v = (uint32_t)(ptr1[len1]) - 48;
        num_18pt->render(pos, v);
    }
}

void OMGame::renderButtonGrace(GLEngine* engine, GLDrawing* drawing) {
    OMGame* game = (OMGame*)engine;
    game->renderNumber12pt( {344+7, 928+66}, game->conf.points, 1 );
}

void OMGame::renderButtonDonate(GLEngine* engine, GLDrawing* drawing) {
    OMGame* game = (OMGame*)engine;
    game->renderNumber12pt( {760+7, 928+66}, game->conf.bet, 1 );
}

void OMGame::renderButtonSelectDonate(GLEngine* engine, GLDrawing* drawing) {
    OMGame* game = (OMGame*)engine;
    Vec2 pos;
    pos.x = drawing->position.x + (GLfloat)(drawing->sprite->width) / 2.0f;
    pos.y = drawing->position.y + (GLfloat)(drawing->sprite->height) / 2.0f;

    GLfloat v = 0.0;
    
    if (drawing->key.equals(L"btn_donate0")) {
        v = 0.1;
    } else if (drawing->key.equals(L"btn_donate1")) {
        v = 0.2;
    } else if (drawing->key.equals(L"btn_donate2")) {
        v = 0.5;
    } else if (drawing->key.equals(L"btn_donate3")) {
        v = 1.0;
    } else if (drawing->key.equals(L"btn_donate4")) {
        v = 5.0;
    } else if (drawing->key.equals(L"btn_donate5")) {
        v = 10.0;
    }
    
    game->renderNumber12pt(pos, v, 0);
}

void OMGame::renderButtonDouble(GLEngine* engine, GLDrawing* drawing) {
    OMGame* game = (OMGame*)engine;
    //GLRender* context = game->context;
    
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
            int doubleBlink = (int)((currentTime - game->takeTime) / 500);
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
            game->renderValue12pt( {1576-7, 928+7}, CString::format(L"%d;%d", game->conf.spin, game->conf.free), -1);
        }
        if (game->conf.prize > 0) {
            game->renderNumber12pt( {1576-7, 928+66}, game->conf.prize, -1, L":");
        }
    } else if (state == 2) {
        drawing->foreground = 5;
        if (game->conf.free != 0) {
            uint64_t currentTime = currentTimeMillis();
            int freeBlink = (int)((currentTime - game->freespinTime) / 500);
            if ((freeBlink >= 8) || ((freeBlink & 1) == 0)) {
                if (game->conf.spin == 0) {
                    game->renderValue12pt( {1576-7, 928+7}, CString::format(L"%d", game->conf.free), -1);
                } else {
                    game->renderValue12pt( {1576-7, 928+7}, CString::format(L"%d;%d", game->conf.spin, game->conf.free), -1);
                }
            } else {
                if (game->conf.spin != 0) {
                    CString sFree = game->conf.free;
                    GLfloat len = sFree.m_length;
                    game->renderValue12pt( {1576-7-30*len, 928+7}, CString::format(L"%d;", game->conf.spin), -1);
                }
            }
        }
        game->renderNumber12pt( {1176+7, 928+66}, game->conf.total, 1);
    } else {
        drawing->foreground = 6;
        game->renderNumber12pt( {1176+7, 928+66}, game->conf.total, 1);
    }
}

void OMGame::drawBackground(GLEngine* engine){
    OMGame* game = (OMGame*)engine;
    GLRender* context = game->scene->context;

    if (game->stage <= STAGE_BARS) {
        game->bkg_bars->render(NULL, NAN, context->cameras->background);
    } else if (game->stage <= STAGE_DOUBLE) {
        game->bkg_double->render(NULL, NAN, context->cameras->background);
        if ((game->stage == STAGE_DOUBLE_SHOW) || (game->stage == STAGE_DOUBLE_HIDE) || (game->stage == STAGE_DOUBLE_TAKE)) {
            if (game->conf.dblindex == 0) {
                game->bkg_sun->render(NULL, NAN, context->cameras->background);
            } else {
                game->bkg_moon->render(NULL, NAN, context->cameras->background);
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
            int doubleBlink = (int)((currentTime - game->doubleShowTimeout) / 500);
            if ((doubleBlink & 1) != 0) {
                state = 1;
            }
        }
        if (state == 0) {
            game->renderNumber18pt( {216-4, 100}, game->conf.doublein, 1 );
            if ((game->stage != STAGE_DOUBLE_TAKE) || (game->conf.doubleout == 0)) {
                game->renderNumber18pt( {970-4, 100}, game->conf.doubleout, 1 );
            }
        }
        
        context->drawRect(0, 178, 1920, 178+152, game->paintShadow);
        game->text_sequence->render();
        GLfloat x = 900;
        if (game->stage == STAGE_DOUBLE_SHOW) {
            GLfloat ofs = (GLfloat)((int64_t)currentTime - (int64_t)(game->doubleShowTimeout) + 3000) / 2000.0f;
            if (ofs > 1) ofs = 1;
            game->paintSpriteLeftCenter.setLight(ofs, ofs);
            game->sequence->render( {x - 176 + ofs * 176, 178+152/2}, (GLuint)game->conf.dblindex, &game->paintSpriteLeftCenter );
            game->paintSpriteLeftCenter.setLight(1.0f - ofs, 1.0f - ofs);
            game->sequence->render( {x + 176 * 4 + ofs * 176, 178+152/2}, (GLuint)game->conf.sequence[0], &game->paintSpriteLeftCenter );
            game->paintSpriteLeftCenter.setLight(1.0, 1.0);
            for (int i = 0; i < 4; i++) {
                game->sequence->render( {x + 176 * i + ofs * 176, 178+152/2}, (GLuint)game->conf.sequence[4-i], &game->paintSpriteLeftCenter );
            }
        } else {
            for (int i = 0; i < 5; i++) {
                game->sequence->render( {x + 176 * i, 178+152/2}, (GLuint)game->conf.sequence[4-i], &game->paintSpriteLeftCenter );
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
    GLfloat worldTime = (GLfloat)(currentTimeMillis() % 86400000) / 86400000.0f;
    
    if (mobile) {
        context->drawEffectMobile(960-700, 606-700, 960+700, 606+700, "q1_effect_torsion", worldTime, worldTime);
    } else {
        context->drawEffect(960-700, 606-700, 960+700, 606+700, "q0_effect_torsion", worldTime, worldTime);
    }
}

void OMGame::drawEffectsDouble(GLEngine* engine) {
    OMGame* game = (OMGame*)engine;
    GLRender* context = game->scene->context;
    
    bool mobile = true;
    int index = game->conf.dblindex;
    
    uint64_t currentTime = currentTimeMillis();
    GLfloat worldTime = (GLfloat)(currentTime % 86400000) / 86400000.0f;
    
    GLfloat localTime;
    if (game->stage != STAGE_DOUBLE_HIDE) {
        localTime = (GLfloat)(((int64_t)currentTime - (int64_t)(game->localTime)) % 86400000) / 86400000.0f;
    } else {
        localTime = (GLfloat)(((int64_t)(game->localTime) - (int64_t)currentTime) % 86400000) / 86400000.0f;
    }
    
    if ((game->stage == STAGE_DOUBLE) || (game->stage == STAGE_DOUBLE_WAIT) || ((game->stage == STAGE_DOUBLE_TAKE) && (game->conf.doubleout != 0))) {
        if (mobile) {
            context->drawEffectMobile(960-320, 606-320, 960+320, 606+320, L"q1_effect_unknown", 0.0, worldTime);
            context->drawEffectMobile(1050-120, 490-120, 1050+120, 490+120, L"q1_effect_unknown_star", 0.0, worldTime);
        } else {
            context->drawEffect(960-320, 606-320, 960+320, 606+320, L"q0_effect_unknown", 0.0, worldTime);
            context->drawEffect(1050-120, 490-120, 1050+120, 490+120, L"q0_effect_unknown_star", 0.0, worldTime);
        }
        
        game->question->render();
    } else if ((game->stage == STAGE_DOUBLE_SHOW) || (game->stage == STAGE_DOUBLE_HIDE) || (game->stage == STAGE_DOUBLE_TAKE)) {
        if (mobile) {
            if (index == 0) {
                context->drawEffectMobile(960-320, 606-320, 960+320, 606+320, L"q1_effect_sun2", localTime, worldTime);
            } else {
                context->drawEffectMobile(960-320, 606-320, 960+320, 606+320, L"q1_effect_moon2", localTime, worldTime);
                context->drawEffectMobile(220-200, 450-200, 220+200, 450+200, L"q1_effect_star1", localTime, worldTime);
                context->drawEffectMobile(450-150, 820-150, 450+150, 820+150, L"q1_effect_star2", localTime, worldTime + 0.3f/86400.0f);
                context->drawEffectMobile(1700-200, 750-200, 1700+200, 750+200, L"q1_effect_star1", localTime, worldTime + 0.6f/86400.0f);
            }
            
            context->drawEffectMobile(1050-120, 490-120, 1050+120, 490+120, L"q1_effect_unknown_star", localTime, worldTime);
        } else {
            if (index == 0) {
                context->drawEffect(960-320, 606-320, 960+320, 606+320, L"q0_effect_sun2", localTime, worldTime);
            } else {
                context->drawEffect(960-320, 606-320, 960+320, 606+320, L"q0_effect_moon2", localTime, worldTime);
                context->drawEffect(220-200, 450-200, 220+200, 450+200, L"q0_effect_star1", localTime, worldTime);
                context->drawEffect(450-150, 820-150, 450+150, 820+150, L"q0_effect_star2", localTime, worldTime + 0.3f/86400.0f);
                context->drawEffect(1700-200, 750-200, 1700+200, 750+200, L"q0_effect_star1", localTime, worldTime + 0.6f/86400.0f);
            }
            
            context->drawEffect(1050-120, 490-120, 1050+120, 490+120, L"q0_effect_unknown_star", localTime, worldTime);
        }
    }
}

void OMGame::drawEffectSwitch(GLEngine* engine) {
    OMGame* game = (OMGame*)engine;
    GLRender* context = game->scene->context;
    
    if (game->nextTime != 0) {
        uint64_t currentTime = currentTimeMillis();
        GLfloat ofs = (GLfloat)((int64_t)currentTime - (int64_t)(game->nextTime)) / 500.0f;
        if (ofs >= 1.0) {
            game->nextTime = 0;
        } else {
            game->paintSwitch.setLight(0, 1.0f - clamp(fabsf(ofs), 0.0f, 1.0f));
            context->drawRect(0, 0, 1920, 1080, game->paintSwitch, context->cameras->background);
        }
    }
}

#ifndef OMDEMO

static char* queryBuilder(const Loader* loader, const Vector<CString&>* query) {
    //document.token = "e994a237491a85ff72b9f737bbf47047cfbc6dbb0897ea1eea5e75338a4b13c3";
    //document.sessid = "8ea5f70b15263872760d7e14ce8e579a";
    //document.devid = "";
    
    CString querys;
    int32_t count = 0;
    if (query != NULL) {
        count = query->count();
        CString** items = query->items();
        for (int i = 0; i < count; i++) {
            if (i != 0) {
                querys.concat(L',');
            }
            querys.concat(*(items[i]));
        }
    }
    
    OMGame* game = (OMGame*)(loader->context->engine);
    
    CString post = CString::format(L"{\"token\":\"%ls\",\"sessid\":\"%ls\",\"devid\":\"%ls\",\"count\":%d,\"data\":[%ls]}",
        (wchar_t*)(game->token),
        (wchar_t*)(game->sessid),
        (wchar_t*)(game->devid),
        count, (wchar_t*)querys);

    return post.toChar();
}

void OMGame::updateConfig() {
    try {
        JsonNode* message = JsonNode::createObject();
        message->setString(L"action", L"update");
        message->setInt(L"ask", nextAsk);
    
        if (stage == STAGE_BARS) {
            if (nextServerConfig != NULL) {
                if (nextServerConfig != serverConfig) {
                    JsonDestroy(nextServerConfig);
                }
                nextServerConfig = NULL;
            }
        
            message->setString(L"last_bars", CString::format(L"%d,%d,%d", (int)(roundf(bars.rotation[0].rotate)), (int)(roundf(bars.rotation[1].rotate)), (int)(roundf(bars.rotation[2].rotate))));
        }
    
        context->loader->query(L"./api.json", JSON::encode(message), queryBuilder);
    } catch(...) {
        shutdown();
    }

    status.query++;
}

void OMGame::queryGenerateBars() {
    try {
        if (nextServerConfig != NULL) {
            if (nextServerConfig != serverConfig) {
                JsonDestroy(nextServerConfig);
            }
            nextServerConfig = NULL;
        }
        
        JsonNode* message = JsonNode::createObject();
        message->setString(L"action", L"generate_bars");
        message->setInt(L"ask", nextAsk);
        message->setDouble(L"last_donate", conf.bet);
        message->setString(L"last_bars", CString::format(L"%d,%d,%d", (int)(roundf(bars.rotation[0].rotate)), (int)(roundf(bars.rotation[1].rotate)), (int)(roundf(bars.rotation[2].rotate))));
        message->setInt(L"last_stage", stage);
        
        context->loader->query(L"./api.json", JSON::encode(message), queryBuilder);
    } catch (...) {
        shutdown();
    }
    
    nextAsk++;
    status.query++;
}

void OMGame::queryGenerateDouble(int index) {
    try {
        if (nextServerConfig != NULL) {
            if (nextServerConfig != serverConfig) {
                JsonDestroy(nextServerConfig);
            }
            nextServerConfig = NULL;
        }
        
        JsonNode* message = JsonNode::createObject();
        message->setString(L"action", L"generate_double");
        message->setInt(L"ask", nextAsk);
        message->setInt(L"last_double_index", index);
        message->setDouble(L"last_result", conf.doublein);
        message->setString(L"last_bars", CString::format(L"%d,%d,%d", (int)(roundf(bars.rotation[0].rotate)), (int)(roundf(bars.rotation[1].rotate)), (int)(roundf(bars.rotation[2].rotate))));
        message->setInt(L"last_stage", stage);
        
        context->loader->query(L"./api.json", JSON::encode(message), queryBuilder);
    } catch (...) {
        shutdown();
    }
    
    nextAsk++;
    status.query++;
}

void OMGame::queryTake() {
    try {
        if (nextServerConfig != NULL) {
            if (nextServerConfig != serverConfig) {
                JsonDestroy(nextServerConfig);
            }
            nextServerConfig = NULL;
        }
        
        JsonNode* message = JsonNode::createObject();
        message->setString(L"action", L"take");
        message->setInt(L"ask", nextAsk);
        message->setString(L"last_bars", CString::format(L"%d,%d,%d", (int)(roundf(bars.rotation[0].rotate)), (int)(roundf(bars.rotation[1].rotate)), (int)(roundf(bars.rotation[2].rotate))));
        message->setInt(L"last_stage", stage);
        
        context->loader->query(L"./api.json", JSON::encode(message), queryBuilder);
    } catch (...) {
        shutdown();
    }
    
    nextAsk++;
    status.query++;
}

void onUpdateStageConfig(OMGame* engine) {
    (void)engine->onUpdateConfig();
}

bool OMGame::onUpdateConfig(OMGameStage action) {
    try {
        // Обработка ответа в onFrame
        if (action != STAGE_UNKNOWN) {
            if (nextServerConfig != NULL) {
                const JsonNode* state = nextServerConfig->opt(L"state");
                if ((state == NULL) || (state->opt(L"sync") == NULL)) {
                    return false;
                }
                int ask = (int)nextServerConfig->optInt(L"ask", -1);
                int sync = (int)state->optInt(L"sync", -1);
                if (ask != sync) {
                    // Синхронизация нарушена и ответ будет обработан по таймауту при переключении сцены
                    return false;
                }
            }
        }
    
        // Восстановить игровую ситуацию?
        bool restore = false;
    
        if (serverConfig == NULL) {
            if (nextServerConfig == NULL) {
                // Сервер не отвечает?
                return false;
            }
        
            // Первый статус
            restore = true;
        } else if (nextServerConfig != NULL) {
            const JsonNode* state = nextServerConfig->opt(L"state");
            if (state == NULL) {
                // Неизвестный ответ
                return false;
            } else {
                int ask = (int)nextServerConfig->optInt(L"ask", -1);
                int sync = (int)state->optInt(L"sync", -1);
                
                if (ask != sync) {
                    // Синхронизация нарушена
                    restore = true;
                } else {
                    // Синхронизация не нарушена
                }
            }
        } else if (action == STAGE_UNKNOWN) {
            // Ответ уже обработан
            return false;
        }
    
        if (nextServerConfig != NULL) {
            if (nextServerConfig != serverConfig) {
                if (serverConfig != NULL) {
                    JsonDestroy(serverConfig);
                }
                serverConfig = nextServerConfig;
            }
            nextServerConfig = NULL;
        }
    
        const JsonNode* state = serverConfig->opt(L"state");
        const JsonNode* grace = serverConfig->opt(L"grace");
        if ((state == NULL) || (grace == NULL)) {
            // Неизвестный ответ
            return false;
        }
        int last_stage = (int)state->optInt(L"last_stage", STAGE_UNKNOWN);
    
        if (restore) {
            //JSON::encode(serverConfig).log();
            
            // Поправляем синхронизацию
            int sync = (int)state->optInt(L"sync", -1);
            serverConfig->setInt(L"ask", sync);
        
            // Сброс локального конфига
            conf.points = (GLfloat)grace->optDouble(L"current_deposit", 0.0);
            conf.pointsin = (GLfloat)grace->optDouble(L"current_in", 0.0);
            conf.bet = (GLfloat)state->optDouble(L"last_donate", 0.1);
            conf.prize = 0;
            conf.prizefree = 0;
            conf.total = 0;
            conf.doublein = (GLfloat)state->optDouble(L"last_double_in", 0.0);
            conf.doubleout = conf.doublein * 2;
            conf.spin = 0;
            conf.free = 0;
            conf.autospin = false;
            conf.dblselect = 0;
            conf.dblindex = 0;
        
            CString tmp_last_bars = state->optString(L"last_bars", L"");
            int idx;
            int count = 0;
            int last_bars[3] = {0,0,0};
            while ((idx = tmp_last_bars.indexOf(L',')) >= 0) {
                last_bars[count] = (int32_t)(tmp_last_bars.substring(0, idx));
                tmp_last_bars = tmp_last_bars.substring(idx + 1);
                count++;
            }
            if (tmp_last_bars.m_length > 0) {
                last_bars[count] = (int32_t)tmp_last_bars;
                count++;
            }
            if (last_stage == STAGE_DOUBLE_SHOW) {
                CString last_gen_bars = state->optString(L"last_bars_gen", L"");
                int ofs = last_gen_bars.m_length - 3;
                if (ofs >= 0) {
                    for (int i = 0; i < 3; i++) {
                        last_bars[i] = ((int)(last_gen_bars.charAt(i)) - 48) * 45;
                    }
                    count = 3;
                }
            }
            if (count == 3) {
                // Считываем положение барабанов
                for (int i = 0; i < 3; i++) {
                    bars.rotation[i].reset(last_bars[i]);
                }
            
                // Обновляем положение группы 3д моделей барабанов
                scene->rotateGroup(groups.barlist1, barsRotateVector, bars.rotation[0].rotate);
                scene->rotateGroup(groups.barlist2, barsRotateVector, bars.rotation[1].rotate);
                scene->rotateGroup(groups.barlist3, barsRotateVector, bars.rotation[2].rotate);
            }

            CString last_double = state->optString(L"last_double", L"");
            if (last_double.m_length == 5) {
                for (int i = 0; i < 5; i++) {
                    conf.sequence[i] = (int8_t)((int32_t)(last_double.charAt(i)) - 48);
                }
            }
        
            if (last_stage == STAGE_BARS_STOP) {
                // Имитируем приход ответа (который уже получен) и запускаем барабаны без запроса на сервер
                nextServerConfig = serverConfig;
                startBars(false, false);
            } else if (last_stage == STAGE_DOUBLE_SHOW) {
                bars.calculate();
                
                // Имитируем приход ответа (который уже получен) и запускаем дабл без запроса на сервер
                nextServerConfig = serverConfig;
                doubleSelect((int)state->optInt(L"last_double_index", 0), false);
            }
        } else if (action == STAGE_BARS_STOP) {
            conf.points = (GLfloat)grace->optDouble(L"current_deposit", 0.0);
            conf.pointsin = (GLfloat)grace->optDouble(L"current_in", 0.0);
            conf.bet = (GLfloat)state->optDouble(L"last_donate", 0.1);
        } else if (action == STAGE_DOUBLE_SHOW) {
            conf.points = (GLfloat)grace->optDouble(L"current_deposit", 0.0);
            conf.pointsin = (GLfloat)grace->optDouble(L"current_in", 0.0);
            conf.bet = (GLfloat)state->optDouble(L"last_donate", 0.1);
            conf.doublein = (GLfloat)state->optDouble(L"last_double_in", 0.0);
            conf.doubleout = conf.doublein * 2;
        }
    
        if ((action == STAGE_UNKNOWN) || (nextConfigUpdate == 0)) {
            nextConfigUpdate = currentTimeMillis() + 60000;
        }

        updateGrace(0);

        return true;
    } catch (...) {
        return false;
    }
}

struct OnQueryThread {
    OMGame* game;
    JSONObject* json;
};

void* OnQueryCallback(void* userData) {
    OnQueryThread* thread = (OnQueryThread*)userData;
    
    try {
        if (thread->json->root != NULL) {
            if (thread->json->root->opt(L"ask") != NULL) {
                const struct JsonNode* state = thread->json->root->opt(L"state");
                if (state != NULL) {
                    int sync = (int)state->optInt(L"sync", -1);
                    if (sync != -1) {
                        thread->game->nextAsk = sync;
                    }
                }
            }
        
            if (thread->game->queryConfig != NULL) {
                JsonDestroy(thread->game->queryConfig);
            }
            thread->game->queryConfig = JsonClone(thread->json->root);
            if (thread->game->queryConfig == NULL) {
                throw eOutOfMemory;
            }
        }
    } catch (...) {
        thread->game->shutdown();
    }

    return NULL;
}

#endif

void OMGame::onFile(const Loader::File& info, void* object) {
#ifdef DEBUG
    CString::format(L"FILE: %ls", (wchar_t*)(info.uri)).log();
#endif
    
#ifndef OMDEMO
    if ((info.group.equals(L"api")) && (info.key.equals(L"query"))) {
        if (object != NULL) {
            OnQueryThread thread;
            thread.game = this;
            thread.json = (JSONObject*)object;
            
            MainThreadSync(OnQueryCallback, &thread);
        }
    }
#endif
}

void OMGame::onStatus(const LoaderStatus& status) {
    this->status.count = status.count;
    this->status.total = status.total;
    
    LOG("STATUS: %d / %d", status.count, status.total);
    
    if (ready >= 1) {
        webScript(OMVIEW_LOAD, CString::format(L"setPercent(%d);", status.count * 100 / status.total));
    } else {
        webScript(OMVIEW_LOAD, L"setPercent(0);");
    }
}

void OMGame::onPreload() {
    LOG("PRELOADED!");
    
    if (!context->createShaders(NULL, NULL)) {
        shutdown();
        return;
    }

    LOG("OMGame > Json Load Config");
    const char *sOMLoadRes =
        #include "OMLoad.res"
    ;
    
    status.count = 0;
    status.total = 0;
    
    LOG("OMGame > Start Loading");
    load(sOMLoadRes);
    
#ifndef OMDEMO
    updateConfig();
#endif
    
    ready = 1;
}

void OMGame::onLoad() {
    if (ready == 1) {
        LOG("READY!");

#if defined(__IOS__)
        //NSBundle* bundle = [NSBundle bundleWithIdentifier:@"com.jappsy.libGameOM"];
        NSBundle* bundle = [NSBundle mainBundle];
        void* library = (__bridge void*)bundle;
#else
        void* library = NULL;
#endif
    
        // Подготавливаем шейдеры
    
        const char *sOMShadersRes =
            #include "OMShaders.res"
        ;
    
        JSONObject shaders = JSONObject(sOMShadersRes);
    
        if (!context->createShaders(&shaders, library)) {
            shutdown();
        }
    
        // Подготавливаем модели

        if (!context->createModels("{}", library)) {
            shutdown();
            return;
        }
    
        // Подготавливаем спрайты из текстур
    
        const char *sOMSpritesRes =
            #include "OMSprites.res"
        ;
    
        if (!context->createSprites(sOMSpritesRes)) {
            shutdown();
            return;
        }
    
        // Подготавливаем картинки из спрайтов
    
        const char *sOMDrawingsRes =
            #include "OMDrawings.res"
        ;
    
        if (!context->createDrawings(sOMDrawingsRes)) {
            shutdown();
            return;
        }
    
        context->drawings->get(L"text_sequence")->setPaint(paintSpriteLeftCenter);
        context->drawings->get(L"dbl_sun")->setPaint(paintSpriteCenter);
        context->drawings->get(L"dbl_moon")->setPaint(paintSpriteCenter);
        context->drawings->get(L"question")->setPaint(paintSpriteCenter);
    
        // Добавляем наложения на кнопки
        context->drawings->get(L"btn_grace")->onrender = renderButtonGrace;
        context->drawings->get(L"btn_donate")->onrender = renderButtonDonate;
        context->drawings->get(L"btn_double")->onrender = renderButtonDouble;
        for (int i = 0; i < 6; i++) {
            context->drawings->get(CString::format(L"btn_donate%d", i))->onrender = renderButtonSelectDonate;
        }
    
        // Задаем свет для спрайтов по умолчанию
        context->light.set( {1.0, 1.0, 1.0, 1.1 } );
    
        // Создаем и описываем сцену с освещением и камерами
        scene = context->scenes->createScene(L"main");
        scene->ambient.set( { 0.0, 0.0, 0.0 } );
    
        GLCamera* cam = context->cameras->createCamera(L"cam");
        cam->size(1920, 1080)->perspective(45, 0.1f, 100)->lookAt( {317.207f, 347.124f, 1351.136f}, {317.207f, 347.124f, -249.692f}, {0,1,0} );
        scene->setCamera(cam);
    
        { // Освещение сцены
            /*
             scene->lights->createLight(L"Omni001")->omni( {-554.5, 701.5, -246}, {1, 1, 1}, 0.54, 0.0, 0.0, false );
             scene->lights->createLight(L"Omni002")->omni( {1183.5, 378.5, 109.5}, {1, 1, 1}, 0.3, 0.0, 0.0, false );
             */
            scene->lights->createLight(L"Spot001")->spot( {783.9f, 359.2f, 619.6f}, {47.2f, 256.3f, -536.5f}, {1, 1, 1}, 1.0f, 111.7f, 2.0f, false );
            scene->lights->createLight(L"Spot002")->spot( {-182.0f, 578.2f, 488.0f}, {256.4, 352.5f, -830.5f}, {1, 1, 1}, 0.63f, 77.2f, 2.0f, false );
        }
    
        Vector<GLObject*>* backLayer = scene->createLayer();
        Vector<GLObject*>* modelLayer = scene->createLayer();
        Vector<GLObject*>* effectLayer = scene->createLayer();
        Vector<GLObject*>* rocketLayer = scene->createLayer();
        Vector<GLObject*>* splashLayer = scene->createLayer();
        Vector<GLObject*>* buttonLayer = scene->createLayer();
        Vector<GLObject*>* blackLayer = scene->createLayer();
    
        { // Феерверки
            for (int i = 0; i < 10; i++) {
                CString name = L"rocket"; name += i;
                GLParticleSystem* s = context->particles->createParticleSystem(name);
                s->createRocket(0, Vec3( {0,0,0} ));
                s->generate();
                
                Vec3 p; p.subtract(scene->camera->target, {0,250,0});
                scene->createLayerObject(rocketLayer, name)->setParticleSystem(name)->objectMatrix.translate(p);
                groups.rocketslist.push( scene->objects->get(name) );
            }
        
            //scene->startParticlesGroup(groups.rocketslist, 2); // test on start
            scene->visibleGroup(groups.rocketslist, true);
        }
    
        { // Модели
            scene->createLayerObject(modelLayer, L"ball_0")->setModel(L"ball")->objectMatrix.translate( {1.947f, 620.468f, -252.468f} );
            scene->createLayerObject(modelLayer, L"cap_0")->setModel(L"cap")->objectMatrix.translate( {4.12f, 572.537f, -249.301f} );
            scene->createLayerObject(modelLayer, L"bar_00")->setModel(L"bar")->objectMatrix.translate( {3.397f, 477.718f, -249.692f} ).multiply(Mat4().rotate({0,-1,0}, 180));
            scene->createLayerObject(modelLayer, L"separator_00")->setModel(L"separator")->objectMatrix.translate( {3.049f, 401.208f, -248.473f} );
            scene->createLayerObject(modelLayer, L"bar_01")->setModel(L"bar")->objectMatrix.translate( {3.397f, 327.124f, -249.692f} ).multiply(Mat4().rotate({0,-1,0}, 90));
            scene->createLayerObject(modelLayer, L"separator_01")->setModel(L"separator")->objectMatrix.translate( {3.049f, 252.739f, -248.473f} );
            scene->createLayerObject(modelLayer, L"bar_02")->setModel(L"bar")->objectMatrix.translate( {3.397f, 179.488f, -249.692f} );
            scene->createLayerObject(modelLayer, L"separator_02")->setModel(L"separator")->objectMatrix.translate( {3.049f, 106.063f, -248.473f} );
        
            scene->createLayerObject(modelLayer, L"ball_1")->setModel(L"ball")->objectMatrix.translate( {315.811f, 620.468f, -252.468f} );
            scene->createLayerObject(modelLayer, L"cap_1")->setModel(L"cap")->objectMatrix.translate( {317.984f, 572.537f, -249.301f} );
            scene->createLayerObject(modelLayer, L"bar_10")->setModel(L"bar")->objectMatrix.translate( {317.207f, 477.718f, -249.692f} ).multiply(Mat4().rotate({0,-1,0}, 180));
            scene->createLayerObject(modelLayer, L"separator_10")->setModel(L"separator")->objectMatrix.translate( {316.914f, 401.208f, -248.473f} );
            scene->createLayerObject(modelLayer, L"bar_11")->setModel(L"bar")->objectMatrix.translate( {317.207f, 327.124f, -249.692f} ).multiply(Mat4().rotate({0,-1,0}, 90));
            scene->createLayerObject(modelLayer, L"separator_11")->setModel(L"separator")->objectMatrix.translate( {316.914f, 252.739f, -248.473f} );
            scene->createLayerObject(modelLayer, L"bar_12")->setModel(L"bar")->objectMatrix.translate( {317.207f, 179.488f, -249.692f} );
            scene->createLayerObject(modelLayer, L"separator_12")->setModel(L"separator")->objectMatrix.translate( {316.914f, 106.063f, -248.473f} );

            scene->createLayerObject(modelLayer, L"ball_2")->setModel(L"ball")->objectMatrix.translate( {626.263f, 620.468f, -252.468f} );
            scene->createLayerObject(modelLayer, L"cap_2")->setModel(L"cap")->objectMatrix.translate( {628.436f, 572.537f, -249.301f} );
            scene->createLayerObject(modelLayer, L"bar_20")->setModel(L"bar")->objectMatrix.translate( {632.114f, 477.718f, -249.692f} ).multiply(Mat4().rotate({0,-1,0}, 180));
            scene->createLayerObject(modelLayer, L"separator_20")->setModel(L"separator")->objectMatrix.translate( {631.676f, 401.208f, -248.473f} );
            scene->createLayerObject(modelLayer, L"bar_21")->setModel(L"bar")->objectMatrix.translate( {632.114f, 327.124f, -249.692f} ).multiply(Mat4().rotate({0,-1,0}, 90));
            scene->createLayerObject(modelLayer, L"separator_21")->setModel(L"separator")->objectMatrix.translate( {631.676f, 252.739f, -248.473f} );
            scene->createLayerObject(modelLayer, L"bar_22")->setModel(L"bar")->objectMatrix.translate( {632.114f, 179.488f, -249.692f} );
            scene->createLayerObject(modelLayer, L"separator_22")->setModel(L"separator")->objectMatrix.translate( {631.676f, 106.063f, -248.473f} );
        
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
            num_12pt = context->sprites->get(L"num_12pt");
            num_12pt_sub = context->sprites->get(L"num_12pt_sub");
            num_18pt = context->sprites->get(L"num_18pt");
            num_18pt_sub = context->sprites->get(L"num_18pt_sub");
            
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
        
            groups.donlist.push( scene->createLayerDrawing(buttonLayer, L"btn_donate0") );
            groups.donlist.push( scene->createLayerDrawing(buttonLayer, L"btn_donate1") );
            groups.donlist.push( scene->createLayerDrawing(buttonLayer, L"btn_donate2") );
            groups.donlist.push( scene->createLayerDrawing(buttonLayer, L"btn_donate3") );
            groups.donlist.push( scene->createLayerDrawing(buttonLayer, L"btn_donate4") );
            groups.donlist.push( scene->createLayerDrawing(buttonLayer, L"btn_donate5") );
            scene->objects->trackEvents(groups.donlist, onButtonEvent);
        
            groups.dbllist.push( scene->createLayerDrawing(effectLayer, L"dbl_sun") );
            groups.dbllist.push( scene->createLayerDrawing(effectLayer, L"dbl_moon") );
            scene->objects->trackEvents(groups.dbllist, onDoubleEvent);
        
            groups.dbleffects.push( scene->objects->get(L"dbl_sun") );
            groups.dbleffects.push( scene->objects->get(L"dbl_moon") );
        }
    
        { // Анимация в удвоении
            dbl_sun = context->drawings->get(L"dbl_sun");
            dbl_moon = context->drawings->get(L"dbl_moon");
            
            question = context->drawings->get(L"question");
        
            scene->createLayerObject(effectLayer, L"effects_double")->setFunc(drawEffectsDouble);

            groups.dbleffects.push( scene->objects->get(L"effects_double") );
        }

        { // Затемнение экрана при переключении сцены
            scene->createLayerObject(blackLayer, L"effects_switch")->setFunc(drawEffectSwitch);
        }
    
        updateStage(STAGE_BARS, NULL, 0, true);
        //updateStage(STAGE_DOUBLE, NULL, 0, true);
        doubleReset(this);
    
        trackBarData1.game = this;
        trackBarData2.game = this;
        trackBarData3.game = this;
        context->touchScreen->trackEvent("bar1", 276, 192, 456, 720, onTrackBar, &trackBarData1);
        context->touchScreen->trackEvent("bar2", 732, 192, 456, 720, onTrackBar, &trackBarData2);
        context->touchScreen->trackEvent("bar3", 1188, 192, 456, 720, onTrackBar, &trackBarData3);
    
        ready = 2;
        
        webLocation(OMVIEW_GAME, CString::format(L"file://%ls", (wchar_t*)(cache->getDataPath(L"mobile", L"mobile.html"))));
        webLocation(OMVIEW_HELP, CString::format(L"file://%ls", (wchar_t*)(cache->getDataPath(L"mobile", L"mobile.html"))));
    }
    
#ifndef OMDEMO
    if (ready == 2) {
        if (queryConfig != NULL) {
            try {
                const JsonNode* state = queryConfig->opt(L"state");
                if (state != NULL) {
                    if ((nextServerConfig != NULL) && (nextServerConfig != serverConfig)) {
                        JsonDestroy(nextServerConfig);
                    }
                    nextServerConfig = queryConfig;
                    queryConfig = NULL;
                    
                    OMGameStage last_stage = (OMGameStage)(state->optInt(L"last_stage", STAGE_UNKNOWN));
                    switch (last_stage) {
                        case STAGE_UNKNOWN:
                        case STAGE_BARS_WAIT:
                            // Если статус на сервере не определен повторяем запрос
                            nextConfigUpdate = currentTimeMillis() + 5000;
                            break;
                        
                        default:
                            if (serverConfig == NULL) {
                                // При первом запросе просто обновляем конфиг с сервера
                                updateStage(last_stage, onUpdateStageConfig, 0, false);
                            } else {
                                int ask = (int)nextServerConfig->optInt(L"ask", -1);
                                int sync = (int)state->optInt(L"sync", -1);
                                
                                if (ask != sync) {
                                    // При рассинхронизации запросов восстанавливаем игровую ситуацию
                                    updateStage(last_stage, onUpdateStageConfig, 500, true);
                                } else {
                                    if ((stage == STAGE_BARS) && (last_stage == stage)) {
                                        const JsonNode* grace = nextServerConfig->opt(L"grace");
                                        if (grace != NULL) {
                                            // При простое и ожидании обновить текущее состояние баланса
                                            conf.points = (GLfloat)grace->optDouble(L"current_deposit", 0.0);
                                            conf.pointsin = (GLfloat)grace->optDouble(L"current_in", 0.0);
                                        }
                                    }
                                    
                                    // При нормальной синхронизации состояние игры обновляется в onFrame
                                    nextConfigUpdate = currentTimeMillis() + 60000;
                                }
                            }
                    }
                }
            } catch (...) {
            }
            
            status.query = 0;
        }
    }
#endif
}

void OMGame::onError(const CString& error) {
#ifdef DEBUG
    CString::format(L"ERROR: %ls", (wchar_t*)error).log();
#endif
    
    if (ready < 2) {
        updateState(OMVIEW_LOAD_ERROR | OMVIEW_ANIMATE);
    } else {
        updateState(OMVIEW_ERROR);
    }
}

void OMGame::onRetry() {
    if (ready < 2) {
        updateState(OMVIEW_LOAD);
    } else {
        updateState(OMVIEW_GAME);
    }
}

void OMGame::onFatal(const CString& error) {
#ifdef DEBUG
    CString::format(L"ERROR: %ls", (wchar_t*)error).log();
#endif
    
    shutdown();
}

void onCalendarDayLoad(const Loader::File* info, void* object, void* userData) {
    int year = (int)(info->file.substring(3, 7));
    int month = (int)(info->file.substring(7, 9));
    int day = (int)(info->file.substring(9, 11));
    
    OMGame* game = (OMGame*)userData;
    JSONObject* json = (JSONObject*)object;
    CString data = JSON::encode(json->root);

    game->webScript(OMVIEW_GAME, CString::format(L"document.calendarDay.onquery('%ls', null, %ls, {year:%d, month:'%02d', day:'%02d'})", (wchar_t*)(info->uri), (wchar_t*)(data), year, month, day));

    AtomicUnlock(&(game->lockCalendarDay));
}

bool onCalendarDayError(const Loader::File* info, void* userData) {
    OMGame* game = (OMGame*)userData;
    AtomicUnlock(&(game->lockCalendarDay));
    
    return true;
}

void onCalendarMonthLoad(const Loader::File* info, void* object, void* userData) {
    int year = (int)(info->file.substring(3, 7));
    int month = (int)(info->file.substring(7, 9));
    
    OMGame* game = (OMGame*)userData;
    JSONObject* json = (JSONObject*)object;
    CString data = JSON::encode(json->root);
    
    game->webScript(OMVIEW_GAME, CString::format(L"document.calendarSelector.onquery('%ls', null, %ls, {year:%d, month:%d})", (wchar_t*)(info->uri), (wchar_t*)(data), year, month));

    AtomicUnlock(&(game->lockCalendarMonth));
}

bool onCalendarMonthError(const Loader::File* info, void* userData) {
    OMGame* game = (OMGame*)userData;
    AtomicUnlock(&(game->lockCalendarMonth));

    return true;
}

void OMGame::onWebLocation(int index, CString& location) {
    if (location.equals(L"ios:init")) {
        if (index == OMVIEW_GAME) {
            webScript(OMVIEW_GAME, CString::format(L"updateLang('%ls'); enableCalendarDay();", (wchar_t*)locale));
            updateState(OMVIEW_GAME | OMVIEW_ANIMATE);
        } else if (index == OMVIEW_HELP) {
            webScript(OMVIEW_HELP,  CString::format(L"updateLang('%ls'); enableInfo();", (wchar_t*)locale));
            updateGrace(0);
        }
    } else if (location.equals(L"ios:close")) {
        if (index != OMVIEW_HELP) {
            updateState(OMVIEW_HIDE | OMVIEW_ANIMATE);
        } else {
            updateState(OMVIEW_GAME | OMVIEW_ANIMATE);
        }
    } else if (location.startsWith(L"ios:calendarDay:")) {
        if (AtomicLockTry(&lockCalendarDay)) {
            context->loader->load(location.replace(L"ios:calendarDay:", L"./calendar/"), L"web", L"calendarDay", onCalendarDayLoad, onCalendarDayError, this);
        }
    } else if (location.startsWith(L"ios:calendarMonth:")) {
        if (AtomicLockTry(&lockCalendarMonth)) {
            context->loader->load(location.replace(L"ios:calendarMonth:", L"./calendar/"), L"web", L"calendarMonth", onCalendarMonthLoad, onCalendarMonthError, this);
        }
    }
}

void OMGame::onWebReady(int index) {
    LOG("webReady: %d", index);
}

void OMGame::onWebFail(int index, CString& error) {
    LOG("webFail: %d", index);
#ifdef DEBUG
    error.log();
#endif
}

OMGame::OMGame(const CString& token, const CString& sessid, const CString& devid, const CString& locale) {
    this->token = token;
    this->sessid = sessid;
    this->devid = devid;
    this->locale = locale;
    
    LOG("OMGame > Setup Cache");

    cache = new Cache(L"om");

    LOG("OMGame > Demo Double Sequence Init");
    conf.sequence.growstep(5);
    conf.sequence.push(1);
    conf.sequence.push(1);
    conf.sequence.push(0);
    conf.sequence.push(0);
    conf.sequence.push(0);

    LOG("OMGame > Setup GLPaint's");
    paint.setColor(0x80FF0000).setStroke(3, 0x800000FF);
    paintLoading.setColor(0xFFFFFFFF);
    paintShadow.setColor(0x80000000);
    paintSwitch.setColor(0x00000000);
    paintSpriteCenter.setColor(0xFFFFFFFF).setAlignX(GLAlignX::CENTER).setAlignY(GLAlignY::MIDDLE);
    paintSpriteLeftCenter.setColor(0xFFFFFFFF).setAlignX(GLAlignX::LEFT).setAlignY(GLAlignY::MIDDLE);

    LOG("OMGame > Setup Base URL");
    setBasePath(L"https://om.jappsy.com/jappsy/");

    LOG("OMGame > Json Preload Config");
    const char *sOMPreloadRes =
        #include "OMPreload.res"
    ;

    LOG("OMGame > Start Preloading");
    preload(sOMPreloadRes);
}

OMGame::~OMGame() {
#ifndef OMDEMO
    if (queryConfig != NULL) {
        JsonDestroy(queryConfig);
        queryConfig = NULL;
    }
    if (serverConfig != NULL) {
        if (serverConfig != nextServerConfig) {
            JsonDestroy(serverConfig);
        }
        serverConfig = NULL;
    }
    if (nextServerConfig != NULL) {
        JsonDestroy(nextServerConfig);
        nextServerConfig = NULL;
    }
#endif
}
