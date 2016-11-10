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

import android.content.Context;
import android.graphics.Bitmap;
import android.os.Build;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.webkit.ConsoleMessage;
import android.webkit.JsResult;
import android.webkit.WebChromeClient;
import android.webkit.WebResourceError;
import android.webkit.WebResourceRequest;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.RelativeLayout;

public class OMWebView extends WebView {
	private long engine = 0;
	private int index = 0;

	private static class OMWebChromeClient extends WebChromeClient {
		/*
		@Override public void onConsoleMessage(String message, int lineNumber, String sourceID) {
			Log.d("WebView(" + String.valueOf(lineNumber) + ")", message);
		}

		@Override public boolean onConsoleMessage(ConsoleMessage consoleMessage) {
			onConsoleMessage(consoleMessage.message(), consoleMessage.lineNumber(),
					consoleMessage.sourceId());
			return false;
		}
		*/

		@Override public boolean onJsAlert(WebView view, String url, String message,
								 JsResult result) {
			Log.d("WebView", "Alert: " + message);

			return false;
		}
	}

	private class OMWebViewClient extends WebViewClient {
		@Override public boolean shouldOverrideUrlLoading(WebView view, String url) {
			if (url.startsWith("ios:")) {
				if (engine != 0) {
					OMEngine.onWebLocation(engine, index, url);
				}

				return true;
			} else if (url.startsWith("log:")) {
				Log.d("WebView", url);

				return true;
			}

			return false;
		}

		@Override public void onPageFinished(WebView view, String url) {
			if (engine != 0) {
				OMEngine.onWebReady(engine, index);
			}
		}

		@Override public void onReceivedError(WebView view, int errorCode, String description, String failingUrl) {
			if (engine != 0) {
				OMEngine.onWebFail(engine, index, "Error #" + String.valueOf(errorCode) + ": " + description);
			}
		}

		@Override public void onReceivedError(WebView view, WebResourceRequest request, WebResourceError error) {
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
				if (request.isForMainFrame()) {
					onReceivedError(view,
							error.getErrorCode(), error.getDescription().toString(),
							request.getUrl().toString());
				}
			} else {
				onReceivedError(view, 0, "Unknown error!", request.toString());
			}
		}
	}

	public OMWebView(Context context) {
		super(context);
		setLayoutParams(new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.MATCH_PARENT));

		setLayerType(View.LAYER_TYPE_HARDWARE, null);

		setWebChromeClient(new OMWebChromeClient());
		setWebViewClient(new OMWebViewClient());
		clearCache(true);
		clearHistory();

		setInitialScale(100);

		WebSettings settings = getSettings();
		settings.setUserAgentString(settings.getUserAgentString() + " Jappsy/1.0");
		settings.setAppCacheEnabled(false);
		settings.setCacheMode(WebSettings.LOAD_NO_CACHE);
		settings.setSupportZoom(false);
		settings.setJavaScriptEnabled(true);
		settings.setJavaScriptCanOpenWindowsAutomatically(false);
		settings.setAllowFileAccess(true);
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
			settings.setAllowFileAccessFromFileURLs(true);
			settings.setAllowUniversalAccessFromFileURLs(true);
		}
		settings.setAllowContentAccess(true);
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
			settings.setMixedContentMode(WebSettings.MIXED_CONTENT_ALWAYS_ALLOW);
		}
		settings.setDomStorageEnabled(true);
	}

	public void setEngine(long engine, int index) {
		this.engine = engine;
		this.index = index;
	}

	@Override public boolean onTouchEvent(MotionEvent event) {
		super.onTouchEvent(event);
		return true;
	}
}
