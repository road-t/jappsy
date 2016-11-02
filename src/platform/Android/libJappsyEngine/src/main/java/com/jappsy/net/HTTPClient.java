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

package com.jappsy.net;

import android.util.Log;

import com.jappsy.core.Memory;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLConnection;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.net.ssl.HttpsURLConnection;

public class HTTPClient {

	public static HTTPResponse Request(final String uri, final String post, final String modifiedSince, final int timeout) {
		//Log.d("Request", uri);

		HTTPResponse response = new HTTPResponse();

		response.code = 0;
		response.error = null;
		response.data = null;

		try {
			URL url = new URL(uri);
			if (url.getProtocol().toLowerCase().equals("https")) {
				URLConnection connection = url.openConnection();
				HttpsURLConnection httpConnection = (HttpsURLConnection)connection;
				httpConnection.setConnectTimeout(timeout * 1000);
				httpConnection.setReadTimeout(timeout * 1000);

				if (post != null) {
					httpConnection.setRequestMethod("POST");
					httpConnection.setRequestProperty("Content-Type", "application/json");
				} else {
					httpConnection.setRequestMethod("GET");
				}

				httpConnection.setRequestProperty("User-Agent", "Jappsy/1.0");
				if (modifiedSince != null) {
					httpConnection.setRequestProperty("If-Modified-Since", modifiedSince);
				}

				if (post != null) {
					OutputStream os = httpConnection.getOutputStream();
					byte[] outputInBytes = post.getBytes("UTF-8");
					os.write(outputInBytes);
					os.close();
				}

				httpConnection.connect();

				response.code = httpConnection.getResponseCode();
				if (response.code == HttpsURLConnection.HTTP_OK) {
					InputStream stream = httpConnection.getInputStream();
					response.data = Memory.readStream(stream);

					response.modified = httpConnection.getHeaderField("Last-Modified");
					if (response.modified == null) {
						response.modified = httpConnection.getHeaderField("Expires");
						if (response.modified == null) {
							response.modified = httpConnection.getHeaderField("Date");
						}
					}
				} else {
					response.error = httpConnection.getResponseMessage();
				}
			} else {
				URLConnection connection = url.openConnection();
				HttpURLConnection httpConnection = (HttpURLConnection)connection;
				httpConnection.setConnectTimeout(timeout * 1000);
				httpConnection.setReadTimeout(timeout * 1000);

				if (post != null) {
					httpConnection.setRequestMethod("POST");
					httpConnection.setRequestProperty("Content-Type", "application/json");
				} else {
					httpConnection.setRequestMethod("GET");
				}

				httpConnection.setRequestProperty("User-Agent", "Jappsy/1.0");
				if (modifiedSince != null) {
					httpConnection.setRequestProperty("If-Modified-Since", modifiedSince);
				}

				if (post != null) {
					OutputStream os = httpConnection.getOutputStream();
					byte[] outputInBytes = post.getBytes("UTF-8");
					os.write(outputInBytes);
					os.close();
				}

				httpConnection.connect();

				response.code = httpConnection.getResponseCode();
				if (response.code == HttpsURLConnection.HTTP_OK) {
					InputStream stream = httpConnection.getInputStream();
					response.data = Memory.readStream(stream);

					response.modified = httpConnection.getHeaderField("Last-Modified");
					if (response.modified == null) {
						response.modified = httpConnection.getHeaderField("Expires");
						if (response.modified == null) {
							response.modified = httpConnection.getHeaderField("Date");
						}
					}
				} else {
					response.error = httpConnection.getResponseMessage();
				}
			}
		} catch (IOException | OutOfMemoryError e) {
			response.error = e.getMessage();
		}

		return response;
	}

}
