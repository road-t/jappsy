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
#include <data/uJSON.h>
#include "OMBars.h"

//#define OMDEMO

enum OMGameStage {
    STAGE_UNKNOWN = -1,
    STAGE_BARS_WAIT = 10,		// Барабаны во время кручения (ожидаем ответ сервера)
    STAGE_BARS_STOP = 11,		// Барабаны останавливаются
    STAGE_BARS_PRIZE = 12,		// Барабаны остановились (показываем выигрыш)
    STAGE_BARS_FREE = 13,		// Барабаны остановились (показываем бонус фриспин)
    STAGE_BARS_USER = 14,		// Барабаны остановились (ожидаем выбор пользователя)
    STAGE_BARS_TAKE = 15,		// Барабаны забираем накопления в общий счет
    STAGE_BARS = 19,			// Барабаны до кручения
    STAGE_DOUBLE_WAIT = 20,		// Удвоение (ожидаем ответ сервера)
    STAGE_DOUBLE_SHOW = 21,		// Удвоение показ результата
    STAGE_DOUBLE_HIDE = 22,		// Удвоение скрытие результата
    STAGE_DOUBLE_TAKE = 23,		// Удвоение возврат в барабаны
    STAGE_DOUBLE = 29,			// Удвоение (ожидаем выбор пользователя)
};
    
class OMGame : public GLEngine {
public:
    GLScene* scene = NULL;              // У нас всего одна сцена игровая с разными активными слоями

    int ready = 0;                      // Готовность всех ресурсов после загрузки
    
    typedef void (*onNextCallback)(OMGame* engine);
    
    OMGameStage stage = STAGE_UNKNOWN;
    uint64_t nextTime = 0;
    uint64_t nextTimeout = 0;
    OMGameStage nextStage = STAGE_UNKNOWN;
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
        GLfloat points = 0;         // Current Points
        GLfloat pointsin = 0;       // Current Points In (won part)
        GLfloat bet = 1;			// Current Bet
        GLfloat prize = 0;			// Current Spin Prize
        GLfloat prizefree = 0;		// Current Free Spin Prize
        GLfloat total = 0;			// Total Free Spins Prize
        GLfloat doublein = 0;		// Double In Prize
        GLfloat doubleout = 0;		// Double Out Prize
        int32_t spin = 0;			// Current Spin
        int32_t free = 0;			// Total Free Spins
        jbool autospin = false;		// Auto Spin
        Vector<int8_t> sequence;    // Double Sequence (0 sun / 1 moon)
        int32_t dblselect = 0;
        int32_t dblindex = 0;
    } conf;
    
#ifndef OMDEMO
    int32_t nextAsk = -1;               // Server Sync
    JsonNode* queryConfig = NULL;
    JsonNode* serverConfig = NULL;
    JsonNode* nextServerConfig = NULL;
    uint64_t nextConfigUpdate = 0;
#endif
    
    struct {
        Vector<GLObject*> barlist;
        Vector<GLObject*> barlist1;
        Vector<GLObject*> barlist2;
        Vector<GLObject*> barlist3;
        
        Vector<GLObject*> bareffects;
        Vector<GLObject*> dbleffects;
        
        Vector<GLObject*> btnlist;
        Vector<GLObject*> dbllist;
        Vector<GLObject*> donlist;
        
        Vector<GLObject*> rocketslist;
    } groups;
    
    jbool donlistSwitch = false;
    
    GLPaint paint;
    GLPaint paintLoading;
    GLPaint paintShadow;
    GLPaint paintSwitch;
    GLPaint paintSpriteCenter;
    GLPaint paintSpriteLeftCenter;

    CString token;
    CString sessid;
    CString devid;
    CString locale;
    
    OMGame(const CString& token, const CString& sessid, const CString& devid, const CString& locale);
    ~OMGame();
    
    int32_t mantra = 0;
    GLSound* activeMantra = NULL;
    
    void playMantra();
    
    Bars bars;

    void updateGrace(GLfloat diff);
    
    void startBars(bool freespin, bool query = true);
    void updateSounds(OMGameStage stage);
    void updateStage(OMGameStage stage, onNextCallback callback = NULL, uint64_t timeout = 0, bool fade = false);
    void updateStageVisibility();
    
    struct TrackBarData {
        OMGame* game;
        int index;
    };
    
    TrackBarData trackBarData1 = { NULL, 0 };
    TrackBarData trackBarData2 = { NULL, 1 };
    TrackBarData trackBarData3 = { NULL, 2 };
    
    struct {
        int32_t count = 0;
        int32_t total = 0;
        
        int32_t query = 0; // счетчик запросов для определения обработанных запросов хода
    } status;
    
    void onFrame(GLRender* context);
    void onResize(int width, int height);
    void onTouch(const CString& event);
    
    bool minimized = false;
    
    static bool onTrackBar(const CString& event, const GLTouchPoint* cur, const GLTouchPoint* delta, const GLTouchPoint* speed, void* userData);
    static bool onButtonEvent(GLEngine* engine, const CString& event, GLDrawing* drawing);
    
    void doubleSelect(int index, bool query = true);
    static void doubleReset(OMGame* engine);
    static bool onDoubleEvent(GLEngine* engine, const CString& event, GLDrawing* drawing);
    
    void renderNumber12pt(const Vec2& position, GLfloat value, int direction, const wchar_t* prefix = NULL);
    void renderValue12pt(const Vec2& position, const CString& value, int direction);
    void renderNumber18pt(const Vec2& position, GLfloat value, int direction, const wchar_t* prefix = NULL);
    
    static void renderButtonGrace(GLEngine* engine, GLDrawing* drawing);
    static void renderButtonDonate(GLEngine* engine, GLDrawing* drawing);
    static void renderButtonSelectDonate(GLEngine* engine, GLDrawing* drawing);
    static void renderButtonDouble(GLEngine* engine, GLDrawing* drawing);
    
    /// GLFunc
    
    GLDrawing* bkg_bars;
    GLDrawing* bkg_double;
    GLDrawing* bkg_sun;
    GLDrawing* bkg_moon;
    static void drawBackground(GLEngine* engine);
    
    /// GLFunc
    
    GLSprite* num_12pt;
    GLSprite* num_12pt_sub;
    GLSprite* num_18pt;
    GLSprite* num_18pt_sub;
    
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
    
    ///
    
#ifndef OMDEMO
    void updateConfig();
    void queryGenerateBars();
    void queryGenerateDouble(int index);
    void queryTake();
    bool onUpdateConfig(OMGameStage action = STAGE_UNKNOWN);
#endif

    void onStatus(const LoaderStatus& status);
    void onFile(const Loader::File& info, void* object);
    void onError(const CString& error);
    void onRetry();

    void onPreload();
    void onLoad();
    void onFatal(const CString& error);
    
    jlock lockCalendarDay = false;
    jlock lockCalendarMonth = false;
    
    void onWebLocation(int index, CString& location);
    void onWebReady(int index);
    void onWebFail(int index, CString& error);

};

#endif //OMGAME_H
