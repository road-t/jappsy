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

package com.jappsy;
import android.app.Activity;
import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.content.res.Resources;
import android.graphics.Color;
import android.os.Build;
import android.support.annotation.Nullable;
import android.util.AttributeSet;

import com.jappsy.JappsyView;
import com.jappsy.OMEngine;
import com.jappsy.gui.Constraint;
import com.jappsy.gui.ConstraintView;
import com.jappsy.libgameom.R;

import java.io.InputStream;
import java.util.ArrayList;

import android.content.res.Configuration;
import android.view.View;
import android.widget.RelativeLayout;

public class OMView extends ConstraintView {

	// При переходе в другой раздел приложения прячется игра
	public static final int OMVIEW_VISIBILITY = 0x0300;
	public static final int OMVIEW_HIDE       = 0x0100;
	public static final int OMVIEW_SHOW       = 0x0200;

	// При сворачивании приложения останавливается игра
	public static final int OMVIEW_RUNNING    = 0x0C00;
	public static final int OMVIEW_STOP       = 0x0400;
	public static final int OMVIEW_RUN        = 0x0800;

	// При перевороте экрана в низ останавливается игра и весь вывод на экран
	public static final int OMVIEW_SUSPENDED  = 0xC000;
	public static final int OMVIEW_PAUSE      = 0x4000;
	public static final int OMVIEW_RESUME     = 0x8000;

	public static final int OMVIEW_MINIMIZED  = 0x3000;
	public static final int OMVIEW_MINIMIZE   = 0x1000;
	public static final int OMVIEW_RESTORE    = 0x2000;

	public static final int OMVIEW_ANIMATE    = 0x10000;

	public static final int OMVIEW_UPDATE     = 0;

	public static final int OMVIEW_ACTIVITY   = 0x00FF;
	public static final int OMVIEW_LOAD_ERROR = 1;
	public static final int OMVIEW_LOAD       = 2;
	public static final int OMVIEW_GAME       = 3;
	public static final int OMVIEW_HELP       = 4;
	public static final int OMVIEW_ERROR      = 5;

	public class OMJappsyView extends JappsyView {
		public OMJappsyView(Context context) {
			super(context);

			//setLayoutParams(new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT, 600));
		}

		@Override public void onStart() {
			super.onStart();

			if (m_context != 0) {
				if (!isInEditMode()) {
					engine = OMEngine.onCreate(OMView.this, _basePath, _token, _sessid, _devid, _locale);
					JappsyEngine.setEngine(m_context, engine);
					calendarView.setEngine(engine, OMVIEW_GAME);
					helpView.setEngine(engine, OMVIEW_HELP);
					loadView.setEngine(engine, OMVIEW_LOAD);
					errorView.setEngine(engine, OMVIEW_ERROR);
					OMEngine.setMinimized(engine, ((_state & OMVIEW_MINIMIZE) != 0));
				}
			}

			/*
			if (!isInEditMode()) {
				String basePath = "https://dev03-om.jappsy.com/jappsy/"; // Demo server
				//String basePath = "https://om.jappsy.com/jappsy/"; // Production server
				String token = "e994a237491a85ff72b9f737bbf47047cfbc6dbb0897ea1eea5e75338a4b13c3";
				String sessid = "8ea5f70b15263872760d7e14ce8e579a";
				String devid = "";
				String locale = "RU";

				JappsyEngine.setEngine(m_context, OMEngine.onCreate(null, basePath, token, sessid, devid, locale));
			}
			 */
		}

		@Override public void onStop() {
			super.onStop();
		}
	}

	public interface OMViewCloseCallback {
		void onClose();
	}

	private String _basePath;
	private String _token;
	private String _sessid;
	private String _devid;
	private String _locale;

	private long engine;

	private OMViewCloseCallback onCloseCallback;

	private boolean defaultPortrait;
	private boolean portrait;
	private boolean startup;

	private int _state;

	private OMWebView calendarView;
	private JappsyView gameView;
	private OMWebView helpView;
	private OMWebView loadView;
	private OMWebView errorView;

	private ArrayList<Constraint> lastLayout;
	private ArrayList<Constraint> layout;

	private ArrayList<Constraint> lpnCalendar;
	private ArrayList<Constraint> lpmCalendar;
	private ArrayList<Constraint> llnCalendar;
	private ArrayList<Constraint> llmCalendar;

	private ArrayList<Constraint> lpnGame;
	private ArrayList<Constraint> lpmGame;
	private ArrayList<Constraint> llnGame;
	private ArrayList<Constraint> llmGame;

	private ArrayList<Constraint> lpvHelp;
	private ArrayList<Constraint> lphHelp;
	private ArrayList<Constraint> llvHelp;
	private ArrayList<Constraint> llhHelp;

	private ArrayList<Constraint> lpvLoad;
	private ArrayList<Constraint> lphLoad;
	private ArrayList<Constraint> llvLoad;
	private ArrayList<Constraint> llhLoad;

	private ArrayList<Constraint> lpvError;
	private ArrayList<Constraint> lphError;
	private ArrayList<Constraint> llvError;
	private ArrayList<Constraint> llhError;

	private String getResourceHtml(int id) {
		try {
			Resources res = getResources();
			InputStream in_s = res.openRawResource(id);

			byte[] b = new byte[in_s.available()];
			final int read = in_s.read(b);

			String result = new String(b);

			return result.replace("R\"JAPPSYRAWSTRING(\n", "").replace("\n)JAPPSYRAWSTRING\"", "");
		} catch (Exception e) {
			return "";
		}
	}

	public OMView(Context context, String basePath, String token, String sessid, String devid, String locale, OMViewCloseCallback callback) {
		super(context);

		_basePath = basePath;
		_token = token;
		_sessid = sessid;
		_devid = devid;
		_locale = locale;

		final String defaultLocale = "EN";

		if (_locale.length() != 2) {
			_locale = defaultLocale;
		} else {
			final String supportedLocales = "RU EN";
			if (!supportedLocales.contains(_locale)) {
				_locale = defaultLocale;
			}
		}

		engine = 0;

		onCloseCallback = callback;

		calendarView = new OMWebView(context);
		calendarView.setBackgroundColor(Color.TRANSPARENT);
		calendarView.loadDataWithBaseURL("http://localhost/", "<!DOCTYPE html><html><head></head><body style=\"background:#000;\"></body></html>", "text/html; charset=UTF-8", null, null);
		//calendarView.loadUrl("file:///data/user/0/com.jappsy.example/cache/om/mobile/mobile.html");
		addView(calendarView);

		// Check if the system supports OpenGL ES 2.0.
		final ActivityManager activityManager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
		final ConfigurationInfo configurationInfo = activityManager.getDeviceConfigurationInfo();
		final boolean supportsEs2 = configurationInfo.reqGlEsVersion >= 0x20000;

		if (supportsEs2) {
			gameView = new OMJappsyView(context);
		} else {
			gameView = new JappsyView(context); // Dummy View
		}
		addView(gameView);

		helpView = new OMWebView(context);
		helpView.setBackgroundColor(Color.TRANSPARENT);
		helpView.loadDataWithBaseURL("http://localhost/", "<!DOCTYPE html><html><head></head><body style=\"background:#000;\"></body></html>", "text/html; charset=UTF-8", null, null);
		addView(helpView);

		String OMLoadHtml = getResourceHtml(R.raw.omloadhtml).replace("{LANG}", _locale);

		loadView = new OMWebView(context);
		loadView.setBackgroundColor(Color.BLACK);
		loadView.loadDataWithBaseURL("http://localhost/", OMLoadHtml, "text/html; charset=UTF-8", null, null);
		addView(loadView);

		String OMErrorHtml = getResourceHtml(R.raw.omerrorhtml).replace("{LANG}", _locale);

		errorView = new OMWebView(context);
		errorView.setBackgroundColor(Color.TRANSPARENT);
		errorView.loadDataWithBaseURL("http://localhost/", OMErrorHtml, "text/html; charset=UTF-8", null, null);
		addView(errorView);

		lastLayout = null;
		layout = null;
		lpnCalendar = lpmCalendar = llnCalendar = llmCalendar = null;
		lpnGame = lpmGame = llnGame = llmGame = null;
		lpvHelp = lphHelp = llvHelp = llhHelp = null;
		lpvLoad = lphLoad = llvLoad = llhLoad = null;
		lpvError = lphError = llvError = llhError = null;

		startup = true;
		_state = OMVIEW_HIDE | OMVIEW_STOP | OMVIEW_RESUME | OMVIEW_LOAD;

		setBackgroundColor(Color.BLACK);

		initConstraints(context);
	}

	private void initConstraints(Context context) {
		lpnCalendar = new ArrayList<Constraint>();
		lpnCalendar.add(new Constraint(calendarView, Constraint.LEFT, this, Constraint.LEFT, 1.0, 0.0, 899));
		lpnCalendar.add(new Constraint(calendarView, Constraint.TOP, this, Constraint.TOP, 1.0, 0.0, 899));
		lpnCalendar.add(new Constraint(calendarView, Constraint.RIGHT, this, Constraint.RIGHT, 1.0, 0.0, 899));
		lpnCalendar.add(new Constraint(calendarView, Constraint.BOTTOM, gameView, Constraint.TOP, 1.0, 0.0, 897));

		lpmCalendar = new ArrayList<Constraint>();
		lpmCalendar.add(new Constraint(calendarView, Constraint.LEFT, this, Constraint.LEFT, 1.0, 0.0, 899));
		lpmCalendar.add(new Constraint(calendarView, Constraint.TOP, this, Constraint.TOP, 1.0, 0.0, 899));
		lpmCalendar.add(new Constraint(calendarView, Constraint.RIGHT, this, Constraint.RIGHT, 1.0, 0.0, 899));
		lpmCalendar.add(new Constraint(calendarView, Constraint.BOTTOM, this, Constraint.BOTTOM, 1.0, 0.0, 899));

		llnCalendar = llmCalendar = new ArrayList<Constraint>();
		llmCalendar.add(new Constraint(calendarView, Constraint.LEFT, this, Constraint.LEFT, 1.0, 0.0, 899));
		llmCalendar.add(new Constraint(calendarView, Constraint.TOP, this, Constraint.TOP, 1.0, 0.0, 899));
		llmCalendar.add(new Constraint(calendarView, Constraint.RIGHT, this, Constraint.RIGHT, 1.0, 0.0, 899));
		llmCalendar.add(new Constraint(calendarView, Constraint.BOTTOM, this, Constraint.BOTTOM, 1.0, 0.0, 899));

		lpnGame = new ArrayList<Constraint>();
		lpnGame.add(new Constraint(gameView, Constraint.LEFT, this, Constraint.LEFT, 1.0, 0.0, 899));
		lpnGame.add(new Constraint(gameView, Constraint.RIGHT, this, Constraint.RIGHT, 1.0, 0.0, 899));
		lpnGame.add(new Constraint(gameView, Constraint.BOTTOM, this, Constraint.BOTTOM, 1.0, 0.0, 899));
		lpnGame.add(new Constraint(gameView, Constraint.HEIGHT, gameView, Constraint.WIDTH, (1080.0 / 1920.0), 0.0, 898));

		lpmGame = llmGame = new ArrayList<Constraint>();
		lpmGame.add(new Constraint(gameView, Constraint.RIGHT, this, Constraint.RIGHT, 1.0, -10.0, 899));
		lpmGame.add(new Constraint(gameView, Constraint.BOTTOM, this, Constraint.BOTTOM, 1.0, -10.0, 899));
		lpmGame.add(new Constraint(gameView, Constraint.WIDTH, null, Constraint.NONE, 1.0, 150.0, 899));
		lpmGame.add(new Constraint(gameView, Constraint.HEIGHT, gameView, Constraint.WIDTH, (1080.0 / 1920.0), 0.0, 898));
		//Fix: Android does not support OpenGL View Alpha
		//lpmGame.add(new Constraint(gameView, Constraint.ALPHA, null, Constraint.NONE, 1.0, 0.8, 899));

		llnGame = new ArrayList<Constraint>();
		llnGame.add(new Constraint(gameView, Constraint.LEFT, this, Constraint.LEFT, 1.0, 0.0, 899));
		llnGame.add(new Constraint(gameView, Constraint.TOP, this, Constraint.TOP, 1.0, 0.0, 899));
		llnGame.add(new Constraint(gameView, Constraint.RIGHT, this, Constraint.RIGHT, 1.0, 0.0, 899));
		llnGame.add(new Constraint(gameView, Constraint.BOTTOM, this, Constraint.BOTTOM, 1.0, 0.0, 899));

		lpvHelp = llvHelp = new ArrayList<Constraint>();
		lpvHelp.add(new Constraint(helpView, Constraint.TOP, this, Constraint.TOP, 1.0, 0.0, 899));
		lpvHelp.add(new Constraint(helpView, Constraint.BOTTOM, this, Constraint.BOTTOM, 1.0, 0.0, 899));
		lpvHelp.add(new Constraint(helpView, Constraint.LEFT, this, Constraint.LEFT, 1.0, 0.0, 899));
		lpvHelp.add(new Constraint(helpView, Constraint.WIDTH, this, Constraint.WIDTH, 1.0, 0.0, 899));

		lphHelp = llhHelp = new ArrayList<Constraint>();
		lphHelp.add(new Constraint(helpView, Constraint.TOP, this, Constraint.TOP, 1.0, 0.0, 899));
		lphHelp.add(new Constraint(helpView, Constraint.BOTTOM, this, Constraint.BOTTOM, 1.0, 0.0, 899));
		lphHelp.add(new Constraint(helpView, Constraint.LEFT, this, Constraint.RIGHT, 1.0, 0.0, 899));
		lphHelp.add(new Constraint(helpView, Constraint.WIDTH, this, Constraint.WIDTH, 1.0, 0.0, 899));
		lphHelp.add(new Constraint(helpView, Constraint.ALPHA, null, Constraint.NONE, 1.0, 0.0, 899));

		lpvLoad = llvLoad = new ArrayList<Constraint>();
		lpvLoad.add(new Constraint(loadView, Constraint.LEFT, this, Constraint.LEFT, 1.0, 0.0, 899));
		lpvLoad.add(new Constraint(loadView, Constraint.TOP, this, Constraint.TOP, 1.0, 0.0, 899));
		lpvLoad.add(new Constraint(loadView, Constraint.RIGHT, this, Constraint.RIGHT, 1.0, 0.0, 899));
		lpvLoad.add(new Constraint(loadView, Constraint.BOTTOM, this, Constraint.BOTTOM, 1.0, 0.0, 899));

		lphLoad = llhLoad = new ArrayList<Constraint>();
		lphLoad.add(new Constraint(loadView, Constraint.LEFT, this, Constraint.LEFT, 1.0, 0.0, 899));
		lphLoad.add(new Constraint(loadView, Constraint.TOP, this, Constraint.TOP, 1.0, 0.0, 899));
		lphLoad.add(new Constraint(loadView, Constraint.RIGHT, this, Constraint.RIGHT, 1.0, 0.0, 899));
		lphLoad.add(new Constraint(loadView, Constraint.BOTTOM, this, Constraint.BOTTOM, 1.0, 0.0, 899));
		lphLoad.add(new Constraint(loadView, Constraint.ALPHA, null, Constraint.NONE, 1.0, 0.0, 899));

		lpvError = llvError = new ArrayList<Constraint>();
		lpvError.add(new Constraint(errorView, Constraint.LEFT, this, Constraint.LEFT, 1.0, 0.0, 899));
		lpvError.add(new Constraint(errorView, Constraint.TOP, this, Constraint.TOP, 1.0, 0.0, 899));
		lpvError.add(new Constraint(errorView, Constraint.RIGHT, this, Constraint.RIGHT, 1.0, 0.0, 899));
		lpvError.add(new Constraint(errorView, Constraint.BOTTOM, this, Constraint.BOTTOM, 1.0, 0.0, 899));

		lphError = llhError = new ArrayList<Constraint>();
		lphError.add(new Constraint(errorView, Constraint.LEFT, this, Constraint.LEFT, 1.0, 0.0, 899));
		lphError.add(new Constraint(errorView, Constraint.TOP, this, Constraint.TOP, 1.0, 0.0, 899));
		lphError.add(new Constraint(errorView, Constraint.RIGHT, this, Constraint.RIGHT, 1.0, 0.0, 899));
		lphError.add(new Constraint(errorView, Constraint.BOTTOM, this, Constraint.BOTTOM, 1.0, 0.0, 899));
		lphError.add(new Constraint(errorView, Constraint.ALPHA, null, Constraint.NONE, 1.0, 0.0, 899));

		if (context.getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT) {
			defaultPortrait = true;
			portrait = true;

			layout = new ArrayList<Constraint>();
			layout.addAll(lpnCalendar);
			layout.addAll(lpnGame);
			layout.addAll(lphHelp);
			layout.addAll(lpvLoad);
			layout.addAll(lphError);

			_state |= OMVIEW_RESTORE;
		} else {
			defaultPortrait = false;
			portrait = false;

			layout = new ArrayList<Constraint>();
			layout.addAll(llmCalendar);
			layout.addAll(llmGame);
			layout.addAll(llhHelp);
			layout.addAll(llvLoad);
			layout.addAll(llhError);

			_state |= OMVIEW_MINIMIZE;
		}

		lastLayout = layout;
		addConstraints(layout);

		calendarView.setVisibility(View.GONE);
		gameView.setVisibility(View.GONE);
		helpView.setVisibility(View.GONE);
		loadView.setVisibility(View.VISIBLE);
		errorView.setVisibility(View.GONE);
		setVisibility(View.GONE);
	}

	public void orientationChanged(int deviceOrientation) {
		if (deviceOrientation == JappsyView.ORIENTATION_FACEDOWN) {
			updateState(OMVIEW_PAUSE);
		} else if (deviceOrientation == JappsyView.ORIENTATION_FACEUP) {
			updateState(OMVIEW_RESUME);
		} else {
			if (startup) {
				startup = false;
				if (deviceOrientation != JappsyView.ORIENTATION_PORTRAIT) {
					portrait = false;
					if (defaultPortrait) {
						updateState(OMVIEW_RESTORE);
					} else {
						updateState(OMVIEW_MINIMIZE);
					}
				} else {
					portrait = true;
					updateState(OMVIEW_RESTORE);
				}
			} else {
				if (deviceOrientation != JappsyView.ORIENTATION_PORTRAIT) {
					if (portrait) {
						portrait = false;
						updateState(OMVIEW_UPDATE);
					}
				} else {
					if (!portrait) {
						portrait = true;
						updateState(OMVIEW_UPDATE);
					}
				}
			}
		}
	}

	public void updateState(int state) {
		int update = 0;

		if ((state & OMVIEW_RUNNING) != (_state & OMVIEW_RUNNING)) {
			if ((state & OMVIEW_STOP) != 0) {
				_state = (_state & (~OMVIEW_RUNNING)) | OMVIEW_STOP;
				update |= OMVIEW_STOP;
				state = state & (~OMVIEW_ANIMATE);
			} else if ((state & OMVIEW_RUN) != 0) {
				_state = (_state & (~OMVIEW_RUNNING)) | OMVIEW_RUN;
				update |= OMVIEW_RUN;
			}
		}

		if ((state & OMVIEW_VISIBILITY) != (_state & OMVIEW_VISIBILITY)) {
			if ((state & OMVIEW_HIDE) != 0) {
				_state = (_state & (~OMVIEW_VISIBILITY)) | OMVIEW_HIDE;
				update |= OMVIEW_HIDE;
			} else if ((state & OMVIEW_SHOW) != 0) {
				_state = (_state & (~OMVIEW_VISIBILITY)) | OMVIEW_SHOW;
				update |= OMVIEW_SHOW;
			}
		}

		if ((state & OMVIEW_SUSPENDED) != (_state & OMVIEW_SUSPENDED)) {
			if ((state & OMVIEW_PAUSE) != 0) {
				_state = (_state & (~OMVIEW_SUSPENDED)) | OMVIEW_PAUSE;
				update |= OMVIEW_PAUSE;
			} else if ((state & OMVIEW_RESUME) != 0) {
				_state = (_state & (~OMVIEW_SUSPENDED)) | OMVIEW_RESUME;
				update |= OMVIEW_RESUME;
			}
		}

		if ((state & OMVIEW_MINIMIZED) != (_state & OMVIEW_MINIMIZED)) {
			if ((state & OMVIEW_MINIMIZE) != 0) {
				_state = (_state & (~OMVIEW_MINIMIZED)) | OMVIEW_MINIMIZE;
				update |= OMVIEW_MINIMIZE;
				if (engine != 0) {
					OMEngine.setMinimized(engine, true);
				}
			} else if ((state & OMVIEW_RESTORE) != 0) {
				_state = (_state & (~OMVIEW_MINIMIZED)) | OMVIEW_RESTORE;
				update |= OMVIEW_RESTORE;
				if (engine != 0) {
					OMEngine.setMinimized(engine, false);
				}
			}
		}

		if ( ((state & OMVIEW_ACTIVITY) != 0) && ((state & OMVIEW_ACTIVITY) != (_state & OMVIEW_ACTIVITY)) ) {
			_state = (_state & (~OMVIEW_ACTIVITY)) | (state & OMVIEW_ACTIVITY);
			update |= (state & OMVIEW_ACTIVITY);
		}

		if ((state == OMVIEW_UPDATE) || (update != 0)) {
			if ( ((_state & OMVIEW_STOP) != 0) || ((_state & OMVIEW_HIDE) != 0) || ((_state & OMVIEW_PAUSE) != 0) ) {
				gameView.onPause();
			} else {
				gameView.onResume();
			}

			final int activity = (_state & OMVIEW_ACTIVITY);

			layout = new ArrayList<Constraint>();

			if (portrait) {
				if ((_state & OMVIEW_MINIMIZE) != 0) {
					layout.addAll(lpmCalendar);
					layout.addAll(lpmGame);
				} else {
					layout.addAll(lpnCalendar);
					layout.addAll(lpnGame);
				}
				switch (activity) {
					case OMVIEW_LOAD_ERROR:
						layout.addAll(lphHelp);
						layout.addAll(lpvLoad);
						layout.addAll(lpvError);
						break;
					case OMVIEW_LOAD:
						layout.addAll(lphHelp);
						layout.addAll(lpvLoad);
						layout.addAll(lphError);
						break;
					case OMVIEW_GAME:
						layout.addAll(lphHelp);
						layout.addAll(lphLoad);
						layout.addAll(lphError);
						break;
					case OMVIEW_HELP:
						layout.addAll(lpvHelp);
						layout.addAll(lphLoad);
						layout.addAll(lphError);
						break;
					case OMVIEW_ERROR:
						layout.addAll(lphHelp);
						layout.addAll(lphLoad);
						layout.addAll(lpvError);
						break;
				}
			} else {
				if ((_state & OMVIEW_MINIMIZE) != 0) {
					layout.addAll(llmCalendar);
					layout.addAll(llmGame);
				} else {
					layout.addAll(llnCalendar);
					layout.addAll(llnGame);
				}
				switch (activity) {
					case OMVIEW_LOAD_ERROR:
						layout.addAll(llhHelp);
						layout.addAll(llvLoad);
						layout.addAll(llvError);
						break;
					case OMVIEW_LOAD:
						layout.addAll(llhHelp);
						layout.addAll(llvLoad);
						layout.addAll(llhError);
						break;
					case OMVIEW_GAME:
						layout.addAll(llhHelp);
						layout.addAll(llhLoad);
						layout.addAll(llhError);
						break;
					case OMVIEW_HELP:
						layout.addAll(llvHelp);
						layout.addAll(llhLoad);
						layout.addAll(llhError);
						break;
					case OMVIEW_ERROR:
						layout.addAll(llhHelp);
						layout.addAll(llhLoad);
						layout.addAll(llvError);
						break;
				}
			}

			switch (activity) {
				case OMVIEW_LOAD_ERROR:
				case OMVIEW_ERROR:
					errorView.requestFocus();
					break;
				case OMVIEW_LOAD:
					loadView.requestFocus();
					break;
				case OMVIEW_GAME:
					calendarView.requestFocus();
					break;
				case OMVIEW_HELP:
					helpView.requestFocus();
					break;
			}

			if ((state & OMVIEW_ANIMATE) != 0) {
				layoutIfNeeded();
			} else {
				if ((_state & OMVIEW_HIDE) != 0) {
					if ((update & OMVIEW_HIDE) != 0) {
						if (onCloseCallback != null) {
							onCloseCallback.onClose();
						}
					}
					//[self setHidden:YES];
				} else {
					setVisibility(View.VISIBLE);
				}

				if ((_state & OMVIEW_PAUSE) != 0) {
					calendarView.setVisibility(View.GONE);
					gameView.setVisibility(View.INVISIBLE);
					helpView.setVisibility(View.GONE);
					loadView.setVisibility(View.GONE);
					errorView.setVisibility(View.GONE);
				} else {
					switch (activity) {
						case OMVIEW_LOAD_ERROR:
							calendarView.setVisibility(View.GONE);
							gameView.setVisibility(View.GONE);
							helpView.setVisibility(View.GONE);
							loadView.setVisibility(View.VISIBLE);
							errorView.setVisibility(View.VISIBLE);
							break;
						case OMVIEW_LOAD:
							calendarView.setVisibility(View.GONE);
							gameView.setVisibility(View.VISIBLE); // Fix: Android требует активное OpenGL окно, чтобы запустить загрузку OMEngine
							helpView.setVisibility(View.GONE);
							loadView.setVisibility(View.VISIBLE);
							errorView.setVisibility(View.GONE);
							break;
						case OMVIEW_GAME:
							calendarView.setVisibility(View.VISIBLE);
							gameView.setVisibility(View.VISIBLE);
							helpView.setVisibility(View.GONE);
							loadView.setVisibility(View.GONE);
							errorView.setVisibility(View.GONE);
							break;
						case OMVIEW_HELP:
							calendarView.setVisibility(View.VISIBLE);
							gameView.setVisibility(View.VISIBLE);
							helpView.setVisibility(View.VISIBLE);
							loadView.setVisibility(View.GONE);
							errorView.setVisibility(View.GONE);
							break;
						case OMVIEW_ERROR:
							calendarView.setVisibility(View.VISIBLE);
							gameView.setVisibility(View.VISIBLE);
							helpView.setVisibility(View.GONE);
							loadView.setVisibility(View.GONE);
							errorView.setVisibility(View.VISIBLE);
							break;
					}
				}
			}

			removeConstraints(lastLayout);
			addConstraints(layout);
			lastLayout = layout;

			if ((state & OMVIEW_ANIMATE) != 0) {
				if ((_state & OMVIEW_HIDE) != 0) {
					if ((update & OMVIEW_HIDE) != 0) {
						if (onCloseCallback != null) {
							onCloseCallback.onClose();
						}
					}
				} else {
					setVisibility(View.VISIBLE);
				}

				if ((_state & OMVIEW_PAUSE) != 0) {
				} else {
					switch (activity) {
						case OMVIEW_LOAD_ERROR:
							loadView.setVisibility(View.VISIBLE);
							errorView.setVisibility(View.VISIBLE);
							break;
						case OMVIEW_LOAD:
							loadView.setVisibility(View.VISIBLE);
							break;
						case OMVIEW_GAME:
							calendarView.setVisibility(View.VISIBLE);
							gameView.setVisibility(View.VISIBLE);
							break;
						case OMVIEW_HELP:
							calendarView.setVisibility(View.VISIBLE);
							gameView.setVisibility(View.VISIBLE);
							helpView.setVisibility(View.VISIBLE);
							break;
						case OMVIEW_ERROR:
							calendarView.setVisibility(View.VISIBLE);
							gameView.setVisibility(View.VISIBLE);
							errorView.setVisibility(View.VISIBLE);
							break;
					}
				}

				gameView.setVisibility(View.GONE);

				animate(0.3, new AnimationCallbacks() {
					@Override public void onStart() { }

					@Override public void onComplete(boolean finished) {
						if (finished) {
							if ((_state & OMVIEW_HIDE) != 0) {
								//[self setHidden:YES];
							} else {
							}

							if ((_state & OMVIEW_PAUSE) != 0) {
								calendarView.setVisibility(View.GONE);
								gameView.setVisibility(View.GONE);
								helpView.setVisibility(View.GONE);
								loadView.setVisibility(View.GONE);
								errorView.setVisibility(View.GONE);
							} else {
								switch (activity) {
									case OMVIEW_LOAD_ERROR:
										calendarView.setVisibility(View.GONE);
										gameView.setVisibility(View.GONE);
										helpView.setVisibility(View.GONE);
										break;
									case OMVIEW_LOAD:
										calendarView.setVisibility(View.GONE);
										gameView.setVisibility(View.GONE);
										helpView.setVisibility(View.GONE);
										errorView.setVisibility(View.GONE);
										break;
									case OMVIEW_GAME:
										gameView.setVisibility(View.VISIBLE);
										helpView.setVisibility(View.GONE);
										loadView.setVisibility(View.GONE);
										errorView.setVisibility(View.GONE);
										break;
									case OMVIEW_HELP:
										gameView.setVisibility(View.VISIBLE);
										loadView.setVisibility(View.GONE);
										errorView.setVisibility(View.GONE);
										break;
									case OMVIEW_ERROR:
										gameView.setVisibility(View.VISIBLE);
										helpView.setVisibility(View.GONE);
										loadView.setVisibility(View.GONE);
										break;
								}
							}
						}
					}
				});

				layoutIfNeeded();
			}
		}
	}

	public void onLocation(int index, String url) {
		OMWebView webView = null;

		switch (index) {
			case OMVIEW_GAME: webView = calendarView; break;
			case OMVIEW_HELP: webView = helpView; break;
			case OMVIEW_LOAD: webView = loadView; break;
			case OMVIEW_ERROR: webView = errorView; break;
		}

		if (webView != null) {
			webView.loadUrl(url);
		}
	}

	public void onScript(int index, String script) {
		OMWebView webView = null;

		switch (index) {
			case OMVIEW_GAME: webView = calendarView; break;
			case OMVIEW_HELP: webView = helpView; break;
			case OMVIEW_LOAD: webView = loadView; break;
			case OMVIEW_ERROR: webView = errorView; break;
		}

		if (webView != null) {
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
				webView.evaluateJavascript(script, null);
			} else {
				webView.loadUrl("javascript:" + script);
			}
		}
	}

	public void onPause() {
		gameView.onPause();
	}

	public void onResume() {
		gameView.onResume();
	}

	public void enterFullscreen() {
		gameView.enterFullscreen();
	}

	public void exitFullscreen() {
		gameView.exitFullscreen();
	}

	public void mixerVolume(float volume) {
		gameView.mixerVolume(volume);
	}

	public boolean isMixerPlaying() {
		return gameView.isMixerPlaying();
	}
}
