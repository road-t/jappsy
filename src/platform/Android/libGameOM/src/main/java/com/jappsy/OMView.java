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
import android.util.AttributeSet;

import com.jappsy.JappsyView;
import com.jappsy.OMEngine;

public class OMView extends JappsyView {
	public OMView(Context context) {
		super(context);
	}

	public OMView(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public OMView(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
	}

	public static OMView create(Activity context) {
		// Check if the system supports OpenGL ES 2.0.
		final ActivityManager activityManager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
		final ConfigurationInfo configurationInfo = activityManager.getDeviceConfigurationInfo();
		final boolean supportsEs2 = configurationInfo.reqGlEsVersion >= 0x20000;

		if (supportsEs2) {
			return new OMView(context);
		}

		return null;
	}

	@Override
	public void onStart() {
		super.onStart();
		if (!isInEditMode()) {
			JappsyEngine.setEngine(m_context, OMEngine.onCreate());
		}
	}
}