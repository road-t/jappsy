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

#include "uHTTPClient.h"
#include <core/uSystem.h>

class HTTPRequest {
public:
	CString url;
	void* userData = NULL;
	bool threaded = false;
	int retry = 0;
	int timeout = 30;
	
	HTTPClient::onStreamCallback onstream = NULL;
	HTTPClient::onErrorCallback onerror = NULL;
	HTTPClient::onRetryCallback onretry = NULL;
	
	static void run(HTTPRequest* request) throw(const char*);
};

#if defined(__IOS__)

Stream* NSDataToStream(NSData* data) throw(const char*) {
	NSUInteger size = [data length];
	uint8_t* streamptr = memAlloc(uint8_t, streamptr, size + sizeof(wchar_t));
	if (streamptr != NULL) {
		memcpy(streamptr, [data bytes], size);
		*((wchar_t*)(streamptr + size)) = L'\0';
		try {
			return new Stream(streamptr, size, true);
		} catch (...) {
			memFree(streamptr);
			throw;
		}
	} else
		throw eOutOfMemory;
}

void* HttpSync(void* threadData) {
	HTTPRequest *http = (HTTPRequest*)threadData;
	@autoreleasepool {
		__sync_synchronize();
		
		NSURL *url = [NSURL URLWithString:(NSString*)(http->url)];
		// FIXED: Decrease Memory Leaking
		//url = [url URLByAppendingQueryString:[NSString stringWithFormat:@"_nocache=%lld+%d", System.nanoTime(), arc4random()]];
		
		NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url];
		[request setHTTPMethod:@"GET"];
		[request setTimeoutInterval:(http->timeout)];
		[request setCachePolicy:NSURLRequestReloadIgnoringCacheData];
		
		do {
			NSURLResponse *response = nil;
			NSError *error = nil;
			NSData *data = [NSURLConnection sendSynchronousRequest:request returningResponse:&response error:&error];
			
			if (data != nil) {
				try {
					Stream* stream = NSDataToStream(data);
					bool result = http->onstream(http->url, stream, http->userData);
					delete stream;
					if (result) {
						break;
					}
				} catch (...) {
				}
			}
			if (http->retry > 0) {
				if ((http->onretry != NULL) && (!http->onretry(http->url, http->userData))) {
					http->onerror(http->url, L"Shutdown", http->userData);
					break;
				}
				http->retry--;
			} else if (http->retry == 0) {
				http->onerror(http->url, [error localizedDescription], http->userData);
				break;
			}
			[NSThread sleepForTimeInterval:1];
		} while (true);
		
		memDelete(http);
	}
	
	return NULL;
}

void* HttpAsync(void* threadData) {
	HTTPRequest *http = (HTTPRequest*)threadData;
	@autoreleasepool {
		__sync_synchronize();
		
		NSURL *url = [NSURL URLWithString:(NSString*)(http->url)];
		// FIXED: Decrease Memory Leaking
		//url = [url URLByAppendingQueryString:[NSString stringWithFormat:@"_nocache=%lld+%d", System.nanoTime(), arc4random()]];
		
		NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url];
		[request setHTTPMethod:@"GET"];
		[request setTimeoutInterval:(http->timeout)];
		[request setCachePolicy:NSURLRequestReloadIgnoringCacheData];
		
		NSOperationQueue *queue = [[NSOperationQueue alloc] init];
		[NSURLConnection sendAsynchronousRequest:request queue:queue completionHandler:^(NSURLResponse *response, NSData *data, NSError *error) {
			bool repeat = false;
			@autoreleasepool {
				do {
					if (data != nil) {
						try {
							Stream* stream = NSDataToStream(data);
							bool result = http->onstream(http->url, stream, http->userData);
							delete stream;
							if (result) {
								break;
							}
						} catch (...) {
						}
					}
					if (http->retry > 0) {
						if ((http->onretry != NULL) && (!http->onretry(http->url, http->userData))) {
							http->onerror(http->url, L"Shutdown", http->userData);
							break;
						}
						http->retry--;
						repeat = true;
					} else if (http->retry == 0) {
						http->onerror(http->url, [error localizedDescription], http->userData);
						break;
					}
				} while (false);
				if (!repeat) {
					memDelete(http);
				}
			}
			if (repeat) {
				(void)MainThreadAsync(HttpAsync, NULL, http);
			}
		}];
	}
	
	return NULL;
}

#endif

void HTTPClient::Request(const CString& url, bool threaded, int retry, int timeout, void* userData, onStreamCallback onstream, onErrorCallback onerror, onRetryCallback onretry) throw(const char*) {
	HTTPRequest* http = memNew(http, HTTPRequest());
	if (http == NULL)
		throw eOutOfMemory;
	
	http->url = url;
	http->threaded = threaded;
	http->retry = retry;
	http->timeout = timeout;
	http->userData = userData;
	
	http->onstream = onstream;
	http->onerror = onerror;
	http->onretry = onretry;

	if (http->threaded) {
		if (IsMainThread()) {
			HttpAsync(http);
		} else {
			(void)MainThreadAsync(HttpAsync, NULL, http);
		}
	} else {
		if (IsMainThread()) {
			HttpSync(http);
		} else {
			(void)MainThreadSync(HttpSync, http);
		}
	}
}
