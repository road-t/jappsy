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

#ifndef OMGAME_H
#define OMGAME_H

#include <opengl/uGLEngine.h>
#include <event/uMotionEvent.h>
#include <data/uStream.h>
#include <net/uLoader.h>
#include <opengl/uGLPaint.h>
#include <data/uVector.h>
#include "OMBars.h"

enum OMGameStage {
    STAGE_UNKNOWN = -1,
    STAGE_BARS_WAIT = 0,		// Барабаны во время кручения (ожидаем ответ сервера)
    STAGE_BARS_STOP = 1,		// Барабаны останавливаются
    STAGE_BARS_PRIZE = 2,		// Барабаны остановились (показываем выигрыш)
    STAGE_BARS_FREE = 3,		// Барабаны остановились (показываем бонус фриспин)
    STAGE_BARS_USER = 4,		// Барабаны остановились (ожидаем выбор пользователя)
    STAGE_BARS_TAKE = 5,		// Барабаны забираем накопления в общий счет
    STAGE_BARS = 6,				// Барабаны до кручения
    STAGE_DOUBLE_WAIT = 7,		// Удвоение (ожидаем ответ сервера)
    STAGE_DOUBLE_SHOW = 8,		// Удвоение показ результата
    STAGE_DOUBLE_HIDE = 9,		// Удвоение скрытие результата
    STAGE_DOUBLE_TAKE = 10,		// Удвоение возврат в барабаны
    STAGE_DOUBLE = 11,			// Удвоение (ожидаем выбор пользователя)
};
    
class OMGame : public GLEngine {
public:
    GLScene* scene = NULL;              // У нас всего одна сцена игровая с разными активными слоями

    bool ready = false;                 // Готовность всех ресурсов после загрузки
    
    typedef void (*onNextCallback)(OMGame* engine);
    
    OMGameStage stage = STAGE_UNKNOWN;
    uint64_t nextTime = 0;
    uint64_t nextTimeout = 0;
    OMGameStage nextStage = STAGE_BARS;
    onNextCallback nextCallback = NULL;
    
    uint64_t prizeTimeout = 0;          // Таймаут показа приза
    uint64_t takeTimeout = 0;			// Таймаут сбора очков
    uint64_t freespinTime = 0;          // Время начала выпадения фриспина
    uint64_t takeTime = 0;              // Время начала забирания приза
    
    uint64_t rollTimeout = 0;			// Таймаут кручения барабана (ожидания от сервера)
    uint64_t selectTimeout = 0;         // Таймаут выбора затмения (ожидания от сервера)
    uint64_t doubleShowTimeout = 0;     // Таймаут показа затмения
    uint64_t doubleHideTimeout = 0;     // Таймаут скрытия затмения
    uint64_t localTime = 0;             // Локальное время для эффектов
    
    struct {
        int32_t points = 1000;		// Current Points
        int32_t bet = 99;			// Current Bet
        int32_t prize = 0;			// Current Spin Prize
        int32_t prizefree = 0;		// Current Free Spin Prize
        int32_t total = 0;			// Total Free Spins Prize
        int32_t doublein = 10;		// Double In Prize
        int32_t doubleout = 20;		// Double Out Prize
        int32_t spin = 0;			// Current Spin
        int32_t free = 0;			// Total Free Spins
        jbool autospin = false;		// Auto Spin
        Vector<int8_t> sequence;    // Double Sequence (0 sun / 1 moon)
        int32_t dblselect = 0;
        int32_t dblindex = 0;
    } conf;
    
    struct {
        Vector<GLObject*> barlist;
        Vector<GLObject*> barlist1;
        Vector<GLObject*> barlist2;
        Vector<GLObject*> barlist3;
        
        Vector<GLObject*> bareffects;
        Vector<GLObject*> dbleffects;
        
        Vector<GLObject*> btnlist;
        Vector<GLObject*> dbllist;
    } groups;

    GLPaint paint;
    GLPaint paintShadow;
    GLPaint paintSwitch;
    GLPaint paintSpriteCenter;
    GLPaint paintSpriteLeftCenter;

    OMGame();
    
    Bars bars;

    void startBars(bool freespin);
    void updateStage(OMGameStage stage, onNextCallback callback = NULL, uint64_t timeout = 0, bool fade = false);
    void updateStageVisibility();
    
    struct TrackBarData {
        OMGame* game;
        int index;
    };
    
    TrackBarData trackBarData1 = { NULL, 1 };
    TrackBarData trackBarData2 = { NULL, 2 };
    TrackBarData trackBarData3 = { NULL, 3 };
    
    void onFrame(GLRender* context);
    void onTouch(const CString& event);
    
    static bool onTrackBar(const CString& event, const Vec2& cur, const Vec2& delta, const Vec2& speed, void* userData);
    static bool onButtonEvent(GLEngine* engine, const CString& event, GLDrawing* drawing);
    
    void doubleSelect(int index);
    static void doubleReset(OMGame* engine);
    static bool onDoubleEvent(GLEngine* engine, const CString& event, GLDrawing* drawing);
    
    GLSprite* num_sm;
    static void renderButtonGrace(GLEngine* engine, GLDrawing* drawing);
    static void renderButtonDonate(GLEngine* engine, GLDrawing* drawing);
    static void renderButtonDouble(GLEngine* engine, GLDrawing* drawing);
    
    void onFile(const CString& url, void* object);
    void onStatus(const LoaderStatus& status);
    void onReady();
    void onError(const CString& error);
    
    /// GLFunc
    
    GLDrawing* bkg_bars;
    GLDrawing* bkg_double;
    GLDrawing* bkg_sun;
    GLDrawing* bkg_moon;
    static void drawBackground(GLEngine* engine);
    
    /// GLFunc
    
    GLSprite* num;
    
    GLDrawing* text_om;
    GLDrawing* text_mani;
    GLDrawing* text_padme;
    GLDrawing* text_hum;
    GLDrawing* text_free4;
    GLDrawing* text_free8;
    GLDrawing* text_doublein;
    GLDrawing* text_doubleout;
    GLDrawing* text_sequence;
    GLSprite* sequence;
    
    static void drawSplash(GLEngine* engine);
    
    /// GLFunc
    
    static void drawEffectsBars(GLEngine* engine);
    
    /// GLFunc
    
    static void drawEffectsTorsion(GLEngine* engine);

    /// GLFunc
    
    GLDrawing* question;
    
    static void drawEffectsDouble(GLEngine* engine);
    
    /// GLFunc
    
    static void drawEffectSwitch(GLEngine* engine);
    
    ///
    
    GLDrawing* dbl_sun;
    GLDrawing* dbl_moon;
};

#endif //OMGAME_H
