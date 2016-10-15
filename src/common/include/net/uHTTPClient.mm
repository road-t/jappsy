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
#include <core/uCache.h>
#include <net/uLoader.h>
#include <opengl/uGLRender.h>

class HTTPRequest : public CObject {
public:
	CString url;
	void* userData = NULL;
	bool threaded = false;
	int retry = 0;
	int timeout = 30;
	char* post = NULL;
	bool cache = true;
	Stream* cacheStream = NULL;
	
	HTTPClient::onStreamCallback onstream = NULL;
	HTTPClient::onErrorCallback onerror = NULL;
	HTTPClient::onRetryCallback onretry = NULL;
	HTTPClient::onFatalCallback onfatal = NULL;
	HTTPClient::onReleaseCallback onrelease = NULL;
};

wchar_t sWeekDay[7][4] = { L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat" };
wchar_t sMonth[12][4] = { L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec" };

int parseInt(const wchar_t* data, int len) throw(const char*) {
	int value = 0;
	wchar_t* ptr = (wchar_t*)data;
	wchar_t ch;
	while ((len-- > 0) && ((ch = *ptr) != 0)) {
		if ((ch < L'0') || (ch > L'9')) {
			throw eConvert;
		}
		
		value *= 10;
		value += (int)(ch) - 48;
		ptr++;
	}
	
	return value;
}

#include <time.h>

uint64_t RFC2616toUTC(const CString& date) {
	if (date.m_length != 29) {
		return 0;
	}

	wchar_t* ptr = ((wchar_t*)date);
	
	int weekDay = -1;
	for (int i = 0; i < 7; i++) {
		if (memcmp(ptr, sWeekDay[i], sizeof(wchar_t) * 3) == 0) {
			weekDay = i;
			break;
		}
	}
	
	if (weekDay == -1) {
		return 0;
	}
	ptr += 3;
	
	if (memcmp(ptr, L", ", sizeof(wchar_t) * 2) != 0) {
		return 0;
	}
	ptr += 2;
	
	int day = -1;
	try {
		day = parseInt(ptr, 2);
		ptr += 2;
	} catch (...) {
		return 0;
	}
	
	if (*ptr != L' ') {
		return 0;
	}
	ptr++;
	
	int month = -1;
	for (int i = 0; i < 12; i++) {
		if (memcmp(ptr, sMonth[i], sizeof(wchar_t) * 3) == 0) {
			month = i;
			break;
		}
	}
	
	if (month == -1) {
		return 0;
	}
	ptr += 3;
	
	if (*ptr != L' ') {
		return 0;
	}
	ptr++;
	
	int year = -1;
	try {
		year = parseInt(ptr, 4);
		ptr += 4;
	} catch (...) {
		return 0;
	}

	if (*ptr != L' ') {
		return 0;
	}
	ptr++;

	int hour = -1;
	try {
		hour = parseInt(ptr, 2);
		ptr += 2;
	} catch (...) {
		return 0;
	}

	if (*ptr != L':') {
		return 0;
	}
	ptr++;
	
	int minute = -1;
	try {
		minute = parseInt(ptr, 2);
		ptr += 2;
	} catch (...) {
		return 0;
	}
	
	if (*ptr != L':') {
		return 0;
	}
	ptr++;
	
	int second = -1;
	try {
		second = parseInt(ptr, 2);
		ptr += 2;
	} catch (...) {
		return 0;
	}
	
	if (memcmp(ptr, L" GMT", sizeof(wchar_t) * 4) != 0) {
		return 0;
	}
	
	//time_t rt;
	//time(&rt);
	struct tm t = {0};
	//localtime_r(&rt, &t);
	
	t.tm_year = year - 1900;
	t.tm_mon = month;
	t.tm_mday = day;
	t.tm_hour = hour;
	t.tm_min = minute;
	t.tm_sec = second;
	t.tm_wday = weekDay;
	static char UTC[] = "UTC";
	t.tm_zone = UTC;
	
	return ((uint64_t)timegm(&t)) * 1000;
}

CString UTCtoRFC2616(uint64_t time) {
	time_t date = (time_t)(time / 1000);
	struct tm t;
	gmtime_r(&date, &t);

	return CString::format(L"%ls, %02d %ls %04d %02d:%02d:%02d GMT", sWeekDay[t.tm_wday], t.tm_mday, sMonth[t.tm_mon], t.tm_year + 1900, t.tm_hour, t.tm_min, t.tm_sec);
}

Stream* HttpLoadCache(void* userData) {
	try {
		Loader::Info* info = (Loader::Info*)userData;
		if (info->loader->context->engine->cache != NULL) {
			return info->loader->context->engine->cache->getData(info->info->path, info->info->file);
		}
	} catch (...) {
	}
	return NULL;
}

void HttpSaveCache(Stream* stream, void* userData) {
	try {
		Loader::Info* info = (Loader::Info*)userData;
		if (info->loader->context->engine->cache != NULL) {
			info->loader->context->engine->cache->addData(info->info->path, info->info->file, stream);
		}
	} catch (...) {
	}
}

#if defined(__IOS__)

#import <UIKit/UIKit.h>

void* HttpSync(void* threadData) {
	HTTPRequest *http = (HTTPRequest*)threadData;
	@autoreleasepool {
		__sync_synchronize();
		
		if (http->cache) {
			http->cacheStream = HttpLoadCache(http->userData);
		}
		
		NSURL *url = [NSURL URLWithString:(NSString*)(http->url)];
		// FIXED: Decrease Memory Leaking
		//url = [url URLByAppendingQueryString:[NSString stringWithFormat:@"_nocache=%lld+%d", System.nanoTime(), arc4random()]];
		
		NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url];
		if (http->post != NULL) {
			[request setHTTPMethod:@"POST"];
			[request setValue:@"application/json" forHTTPHeaderField:@"Content-Type"];
			[request setHTTPBody:[NSData dataWithBytes:http->post length:strlen(http->post)]];
		} else {
			[request setHTTPMethod:@"GET"];
		}

		[request setValue:@"Jappsy/1.0" forHTTPHeaderField:@"User-Agent"];
		if (http->cacheStream != NULL) {
			[request setValue:(NSString*)UTCtoRFC2616(http->cacheStream->getModificationDate()) forHTTPHeaderField:@"If-Modified-Since"];
		}

		[request setTimeoutInterval:(http->timeout)];
		[request setCachePolicy:NSURLRequestReloadIgnoringCacheData];
		
		volatile Stream* resultStream = NULL;
		volatile CString* resultError = NULL;
		
		do {
			if ([[UIDevice currentDevice].systemVersion floatValue] >= 9.0f) {
				jlock sync = false;
				
				AtomicLock(&sync);
				NSURLSessionDataTask* task = [[NSURLSession sharedSession] dataTaskWithRequest:request completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
					__sync_synchronize();
				
					@autoreleasepool {
						if (response != nil) {
							NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;
							int status = (int)[httpResponse statusCode];
							
							if (status == 304) { // Not Modified
								if (http->cacheStream != NULL) {
									AtomicSetPtr(&resultStream, http->cacheStream);
								}
							} else if (status == 200) { // OK
								if (data != nil) {
									try {
										Stream* stream = NSDataToStream(data);
										
										NSDictionary* headers = [(NSHTTPURLResponse *)response allHeaderFields];
										NSString* modified = [headers valueForKey:@"Last-Modified"];
										if (modified == nil) {
											modified = [headers valueForKey:@"Expires"];
											if (modified == nil) {
												modified = [headers valueForKey:@"Date"];
											}
										}
										
										if (modified != nil) {
											stream->setModificationDate(RFC2616toUTC(modified));
										}
										
										HttpSaveCache(stream, http->userData);
										
										AtomicSetPtr(&resultStream, stream);
									} catch (...) {
									}
								}
							}
						}
						if (error != nil) {
							AtomicSetPtr(&resultError, new CString([error localizedDescription]));
						}
					}

					AtomicUnlock(&sync);
				}];
				[task resume];
			
				AtomicLock(&sync);
				AtomicUnlock(&sync);
				__sync_synchronize();
			} else {
#if __IPHONE_OS_VERSION_MIN_REQUIRED < 90000
				NSURLResponse *response = nil;
				NSError *error = nil;

				NSData *data = [NSURLConnection sendSynchronousRequest:request returningResponse:&response error:&error];
			
				if (response != nil) {
					NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;
					int status = (int)[httpResponse statusCode];
				
					if (status == 304) { // Not Modified
						if (http->cacheStream != NULL) {
							resultStream = http->cacheStream;
						}
					} else if (status == 200) { // OK
						if (data != nil) {
							try {
								Stream* stream = NSDataToStream(data);
								
								NSDictionary* headers = [(NSHTTPURLResponse *)response allHeaderFields];
								NSString* modified = [headers valueForKey:@"Last-Modified"];
								if (modified == nil) {
									modified = [headers valueForKey:@"Expires"];
									if (modified == nil) {
										modified = [headers valueForKey:@"Date"];
									}
								}
								
								if (modified != nil) {
									stream->setModificationDate(RFC2616toUTC(modified));
								}
							
								HttpSaveCache(stream, http->userData);
								
								resultStream = stream;
							} catch (...) {
							}
						}
					}
				}
				if (error != nil) {
					resultError = new CString([error localizedDescription]);
				}
#endif
			}
			
			if (resultStream != NULL) {
				if (resultStream == http->cacheStream) {
					resultStream = NULL;
					try {
						bool result = http->onstream(http->url, http->cacheStream, http->userData);
						delete http->cacheStream;
						http->cacheStream = NULL;
						if (result) {
							break;
						}
					} catch (...) {
						delete http->cacheStream;
						http->cacheStream = NULL;
					}
				} else {
					try {
						bool result = http->onstream(http->url, (Stream*)resultStream, http->userData);
						delete resultStream;
						resultStream = NULL;
						if (result) {
							break;
						}
					} catch (...) {
					}
				}
			}
				
			if (resultStream != NULL) {
				delete resultStream;
				resultStream = NULL;
			}

			int timeout = 1000;
			if (http->onerror != NULL) {
				if (resultError == NULL) {
					resultError = new CString();
				}
				timeout = http->onerror(http->url, *((CString*)resultError), http->userData);
			}
			if (http->retry > 0) {
				if ((http->onretry != NULL) && (!http->onretry(http->url, http->userData))) {
					http->onfatal(http->url, L"Shutdown", http->userData);
					if (resultError != NULL) {
						delete resultError;
						resultError = NULL;
					}
					break;
				}
				http->retry--;
			} else if (http->retry == 0) {
				if (http->cacheStream != NULL) {
					try {
						bool result = http->onstream(http->url, http->cacheStream, http->userData);
						delete http->cacheStream;
						http->cacheStream = NULL;
						if (result) {
							if (resultError != NULL) {
								delete resultError;
								resultError = NULL;
							}
							break;
						}
					} catch (...) {
						delete http->cacheStream;
						http->cacheStream = NULL;
					}
				}
				if (resultError == NULL) {
					resultError = new CString();
				}
				http->onfatal(http->url, *((CString*)resultError), http->userData);
				if (resultError != NULL) {
					delete resultError;
					resultError = NULL;
				}
				break;
			}

			systemSleep(timeout);
		} while (true);
		
		http->onrelease(http->userData);
		if (http->cacheStream != NULL) {
			delete http->cacheStream;
			http->cacheStream = NULL;
		}
		memDelete(http);
	}
	
	return NULL;
}

void* HttpAsync(void* threadData) {
	HTTPRequest *http = (HTTPRequest*)threadData;
	@autoreleasepool {
		__sync_synchronize();
		
		if (http->cache) {
			http->cacheStream = HttpLoadCache(http->userData);
		}

		NSURL *url = [NSURL URLWithString:(NSString*)(http->url)];
		// FIXED: Decrease Memory Leaking
		//url = [url URLByAppendingQueryString:[NSString stringWithFormat:@"_nocache=%lld+%d", System.nanoTime(), arc4random()]];
		
		NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url];
		if (http->post != NULL) {
			[request setHTTPMethod:@"POST"];
			[request setValue:@"application/json" forHTTPHeaderField:@"Content-Type"];
			[request setHTTPBody:[NSData dataWithBytes:http->post length:strlen(http->post)]];
		} else {
			[request setHTTPMethod:@"GET"];
		}
		
		[request setValue:@"Jappsy/1.0" forHTTPHeaderField:@"User-Agent"];
		if (http->cacheStream != NULL) {
			[request setValue:(NSString*)UTCtoRFC2616(http->cacheStream->getModificationDate()) forHTTPHeaderField:@"If-Modified-Since"];
		}
													  
		[request setTimeoutInterval:(http->timeout)];
		[request setCachePolicy:NSURLRequestReloadIgnoringCacheData];
		
		NSOperationQueue *queue = [[NSOperationQueue alloc] init];
		if ([[UIDevice currentDevice].systemVersion floatValue] >= 9.0f) {
			NSURLSessionDataTask* task = [[NSURLSession sharedSession] dataTaskWithRequest:request completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
				bool repeat = false;
				int timeout = 1000;
				@autoreleasepool {
					do {
						if (response != nil) {
							NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;
							int status = (int)[httpResponse statusCode];
							
							if (status == 304) { // Not Modified
								if (http->cacheStream != NULL) {
									try {
										bool result = http->onstream(http->url, http->cacheStream, http->userData);
										delete http->cacheStream;
										http->cacheStream = NULL;
										if (result) {
											break;
										}
									} catch (...) {
										delete http->cacheStream;
										http->cacheStream = NULL;
									}
								}
							} else if (status == 200) { // OK
								if (data != nil) {
									try {
										Stream* stream = NSDataToStream(data);
										
										NSDictionary* headers = [(NSHTTPURLResponse *)response allHeaderFields];
										NSString* modified = [headers valueForKey:@"Last-Modified"];
										if (modified == nil) {
											modified = [headers valueForKey:@"Expires"];
											if (modified == nil) {
												modified = [headers valueForKey:@"Date"];
											}
										}
										
										if (modified != nil) {
											stream->setModificationDate(RFC2616toUTC(modified));
										}
										
										HttpSaveCache(stream, http->userData);
										
										bool result = http->onstream(http->url, stream, http->userData);
										delete stream;
										if (result) {
											break;
										}
									} catch (...) {
									}
								}
							}
						}
						if (http->onerror != NULL) {
							timeout = http->onerror(http->url, [error localizedDescription], http->userData);
						}
						if (http->retry > 0) {
							if ((http->onretry != NULL) && (!http->onretry(http->url, http->userData))) {
								http->onfatal(http->url, L"Shutdown", http->userData);
								break;
							}
							http->retry--;
							repeat = true;
						} else if (http->retry == 0) {
							if (http->cacheStream != NULL) {
								try {
									bool result = http->onstream(http->url, http->cacheStream, http->userData);
									delete http->cacheStream;
									http->cacheStream = NULL;
									if (result) {
										break;
									}
								} catch (...) {
									delete http->cacheStream;
									http->cacheStream = NULL;
								}
							}
							http->onfatal(http->url, [error localizedDescription], http->userData);
							break;
						} else {
							repeat = true;
						}
					} while (false);
					if (!repeat) {
						http->onrelease(http->userData);
						if (http->cacheStream != NULL) {
							delete http->cacheStream;
							http->cacheStream = NULL;
						}
						memDelete(http);
					}
				}
				if (repeat) {
					systemSleep(timeout);
					(void)MainThreadAsync(HttpAsync, NULL, http);
				}
			}];
			[task resume];
		} else {
#if __IPHONE_OS_VERSION_MIN_REQUIRED < 90000
			[NSURLConnection sendAsynchronousRequest:request queue:queue completionHandler:^(NSURLResponse *response, NSData *data, NSError *error) {
				bool repeat = false;
				int timeout = 1000;
				@autoreleasepool {
					do {
						if (response != nil) {
							NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;
							int status = (int)[httpResponse statusCode];
							
							if (status == 304) { // Not Modified
								if (http->cacheStream != NULL) {
									try {
										bool result = http->onstream(http->url, http->cacheStream, http->userData);
										delete http->cacheStream;
										http->cacheStream = NULL;
										if (result) {
											break;
										}
									} catch (...) {
										delete http->cacheStream;
										http->cacheStream = NULL;
									}
								}
							} else if (status == 200) { // OK
								if (data != nil) {
									try {
										Stream* stream = NSDataToStream(data);
										
										NSDictionary* headers = [(NSHTTPURLResponse *)response allHeaderFields];
										NSString* modified = [headers valueForKey:@"Last-Modified"];
										if (modified == nil) {
											modified = [headers valueForKey:@"Expires"];
											if (modified == nil) {
												modified = [headers valueForKey:@"Date"];
											}
										}
										
										if (modified != nil) {
											stream->setModificationDate(RFC2616toUTC(modified));
										}
										
										HttpSaveCache(stream, http->userData);
										
										bool result = http->onstream(http->url, stream, http->userData);
										delete stream;
										if (result) {
											break;
										}
									} catch (...) {
									}
								}
							}
						}
						if (http->onerror != NULL) {
							timeout = http->onerror(http->url, [error localizedDescription], http->userData);
						}
						if (http->retry > 0) {
							if ((http->onretry != NULL) && (!http->onretry(http->url, http->userData))) {
								http->onfatal(http->url, L"Shutdown", http->userData);
								break;
							}
							http->retry--;
							repeat = true;
						} else if (http->retry == 0) {
							if (http->cacheStream != NULL) {
								try {
									bool result = http->onstream(http->url, http->cacheStream, http->userData);
									delete http->cacheStream;
									http->cacheStream = NULL;
									if (result) {
										break;
									}
								} catch (...) {
									delete http->cacheStream;
									http->cacheStream = NULL;
								}
							}
							http->onfatal(http->url, [error localizedDescription], http->userData);
							break;
						} else {
							repeat = true;
						}
					} while (false);
					if (!repeat) {
						http->onrelease(http->userData);
						if (http->cacheStream != NULL) {
							delete http->cacheStream;
							http->cacheStream = NULL;
						}
						memDelete(http);
					}
				}
				if (repeat) {
					systemSleep(timeout);
					(void)MainThreadAsync(HttpAsync, NULL, http);
				}
			}];
#endif
		}
	}
	
	return NULL;
}

#elif defined(__JNI__)

static const char _clsHttpClient[] = "com/jappsy/net/HTTPClient";
const char* clsHttpClient = _clsHttpClient;

static const char _clsHttpResponse[] = "com/jappsy/net/HTTPResponse";
const char* clsHttpResponse = _clsHttpResponse;

jclass clazzHttpClient;
jclass clazzHttpResponse;

struct HTTPRequestResponse {
	HTTPRequest* http;
	Stream* resultStream;
	CString* resultError;
};

void* HttpRequest(void* threadData) {
	HTTPRequest *http = (HTTPRequest*)threadData;

	CString::format(L"HttpRequest: %ls", (wchar_t*)(http->url)).log();

	JNIEnv* env = GetThreadEnv();
	jstring juri = http->url.toJString(env);
	jstring jpost = http->post == NULL ? NULL : env->NewStringUTF(http->post);
	jmethodID method = env->GetStaticMethodID(clazzHttpClient, "Request", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Lcom/jappsy/net/HTTPResponse;");
	jfieldID jfCode = env->GetFieldID(clazzHttpResponse, "code", "I");
	jfieldID jfError = env->GetFieldID(clazzHttpResponse, "error", "Ljava/lang/String;");
	jfieldID jfData = env->GetFieldID(clazzHttpResponse, "data", "[B");
	jfieldID jfModified = env->GetFieldID(clazzHttpResponse, "modified", "Ljava/lang/String;");

	int status = 0;
	Stream* stream = NULL;

	HTTPRequestResponse* requestResponse = NULL;
	try {
		requestResponse = (HTTPRequestResponse *) mmalloc(sizeof(HTTPRequestResponse));
		requestResponse->http = http;
		requestResponse->resultError = NULL;
		requestResponse->resultStream = NULL;
	} catch (...) {
		http->onfatal(http->url, eOutOfMemory, http->userData);
		http->onrelease(http->userData);
		memDelete(http);
		goto HttpRequest_response;
	}

	if ((jfCode == NULL) || (jfError == NULL) || (jfData == NULL) || (jfModified == NULL)) {
		requestResponse->resultError = new CString(eInvalidPointer);
		goto HttpRequest_response;
	}

	{
		__sync_synchronize();

		if (http->cache) {
			http->cacheStream = HttpLoadCache(http->userData);
		}

		jstring jmodifiedSince = NULL;
		if (http->cacheStream != NULL) {
			jmodifiedSince = UTCtoRFC2616(http->cacheStream->getModificationDate()).toJString(env);
		}

		jobject response = env->CallStaticObjectMethod(clazzHttpClient, method, juri, jpost,
													   jmodifiedSince, http->timeout);
		if (!(env->ExceptionCheck()) && (response != NULL)) {
			status = env->GetIntField(response, jfCode);

			jstring JresponseError = (jstring) env->GetObjectField(response, jfError);
			if (JresponseError != NULL) {
				const char *NresponseError = env->GetStringUTFChars(JresponseError, 0);
				if (NresponseError != NULL) {
					requestResponse->resultError = new CString(NresponseError);
					env->ReleaseStringUTFChars(JresponseError, NresponseError);
				}
			} else {
				jbyteArray JresponseData = (jbyteArray) env->GetObjectField(response, jfData);
				try {
					stream = jbyteArrayToStream(env, JresponseData);

					if (stream != NULL) {
						jstring JresponseModified = (jstring) env->GetObjectField(response,
																				  jfModified);
						if (JresponseModified != NULL) {
							const char *NresponseModified = env->GetStringUTFChars(
									JresponseModified, 0);
							if (NresponseModified != NULL) {
								stream->setModificationDate(RFC2616toUTC(NresponseModified));
								env->ReleaseStringUTFChars(JresponseModified, NresponseModified);
							}
						}
					}
				} catch (const char *e) {
					requestResponse->resultError = new CString(e);
				} catch (...) {
					requestResponse->resultError = new CString(eUnknown);
				}
			}
		}
	}

	if (status == 304) { // Not Modified
		if (http->cacheStream != NULL) {
			requestResponse->resultStream = http->cacheStream;
		}
	} else if (status == 200) { // OK
		if (stream != NULL) {
			try {
				HttpSaveCache(stream, http->userData);

				requestResponse->resultStream = stream;
			} catch (...) {
			}
		}
	}

HttpRequest_response:

	ReleaseThreadEnv();

	return requestResponse;
}

void* HttpRequestResultSync(void* threadData) {
	HTTPRequestResponse* requestResponse = (HTTPRequestResponse*)threadData;

	if (requestResponse == NULL) {
		LOG("HttpRequestResultSync: NULL");

		return NULL;
	}

	__sync_synchronize();

	CString::format(L"HttpRequestResultSync: %ls", (wchar_t*)(requestResponse->http->url)).log();

	int timeout = 1000;

	if (requestResponse->resultStream != NULL) {
		if (requestResponse->resultStream == requestResponse->http->cacheStream) {
			requestResponse->resultStream = NULL;
			try {
				bool result = requestResponse->http->onstream(requestResponse->http->url, requestResponse->http->cacheStream, requestResponse->http->userData);
				delete requestResponse->http->cacheStream;
				requestResponse->http->cacheStream = NULL;
				if (result) {
					timeout = 0;
					goto HttpRequestResultSync_break;
				}
			} catch (...) {
				delete requestResponse->http->cacheStream;
				requestResponse->http->cacheStream = NULL;
			}
		} else {
			try {
				bool result = requestResponse->http->onstream(requestResponse->http->url, (Stream*)requestResponse->resultStream, requestResponse->http->userData);
				delete requestResponse->resultStream;
				requestResponse->resultStream = NULL;
				if (result) {
					timeout = 0;
					goto HttpRequestResultSync_break;
				}
			} catch (...) {
			}
		}
	}

	if (requestResponse->resultStream != NULL) {
		delete requestResponse->resultStream;
		requestResponse->resultStream = NULL;
	}

	if (requestResponse->http->onerror != NULL) {
		if (requestResponse->resultError == NULL) {
			requestResponse->resultError = new CString();
		}
		timeout = requestResponse->http->onerror(requestResponse->http->url, *((CString*)requestResponse->resultError), requestResponse->http->userData);
	}
	if (requestResponse->http->retry > 0) {
		if ((requestResponse->http->onretry != NULL) && (!requestResponse->http->onretry(requestResponse->http->url, requestResponse->http->userData))) {
			requestResponse->http->onfatal(requestResponse->http->url, L"Shutdown", requestResponse->http->userData);
			timeout = 0;
			goto HttpRequestResultSync_break;
		}
		requestResponse->http->retry--;
	} else if (requestResponse->http->retry == 0) {
		if (requestResponse->http->cacheStream != NULL) {
			try {
				bool result = requestResponse->http->onstream(requestResponse->http->url, requestResponse->http->cacheStream, requestResponse->http->userData);
				delete requestResponse->http->cacheStream;
				requestResponse->http->cacheStream = NULL;
				if (result) {
					timeout = 0;
					goto HttpRequestResultSync_break;
				}
			} catch (...) {
				delete requestResponse->http->cacheStream;
				requestResponse->http->cacheStream = NULL;
			}
		}
		if (requestResponse->resultError == NULL) {
			requestResponse->resultError = new CString();
		}
		requestResponse->http->onfatal(requestResponse->http->url, *((CString*)requestResponse->resultError), requestResponse->http->userData);
		timeout = 0;
		goto HttpRequestResultSync_break;
	}

HttpRequestResultSync_break:

	if (requestResponse->resultError != NULL) {
		delete requestResponse->resultError;
		requestResponse->resultError = NULL;
	}

	if (requestResponse->http->cacheStream != NULL) {
		delete requestResponse->http->cacheStream;
		requestResponse->http->cacheStream = NULL;
	}

	if (timeout == 0) {
		requestResponse->http->onrelease(requestResponse->http->userData);
		memDelete(requestResponse->http);
	}

	mmfree(requestResponse);

	return (void*)(intptr_t)(timeout);
}

void* HttpRequestResultAsync(void* resultData);
void* HttpRequestResult(void* threadData, void* resultData) {
	HTTPRequestResponse* requestResponse = (HTTPRequestResponse*)resultData;

	if (requestResponse != NULL) {
		CString::format(L"HttpRequestResult: %ls", (wchar_t*)(requestResponse->http->url)).log();

		MainThreadAsync(HttpRequestResultAsync, NULL, requestResponse);
	}

	return NULL;
}

struct HTTPRequestRepeat {
	HTTPRequest* http;
	int timeout;
};

void* HttpRequestRepeat(void* threadData) {
	HTTPRequestRepeat* repeat = (HTTPRequestRepeat*)threadData;

	CString::format(L"HttpRequestRepeat: %ls", (wchar_t*)(repeat->http->url)).log();

	systemSleep(repeat->timeout);

	MainThreadAsync(HttpRequestResultAsync, NULL, HttpRequest(repeat->http));

	mmfree(repeat);
	return NULL;
}

void* HttpRequestResultAsync(void* resultData) {
	HTTPRequestResponse* requestResponse = (HTTPRequestResponse*)resultData;
	HTTPRequest* http = NULL;
	if (requestResponse != NULL) {
		http = requestResponse->http;

		CString::format(L"HttpRequestResultAsync: %ls", (wchar_t*)(http->url)).log();
	} else {
		LOG("HttpRequestResultAsync: NULL");
	}

	int repeatTimeout = (int)(intptr_t)(HttpRequestResultSync(requestResponse));
	if ((repeatTimeout != 0) && (http != NULL)) {
		HTTPRequestRepeat* repeat = (HTTPRequestRepeat*)mmalloc(sizeof(HTTPRequestRepeat));
		if (repeat == NULL) {
			http->onfatal(http->url, eOutOfMemory, http->userData);
			http->onrelease(http->userData);
			memDelete(http);
			return NULL;
		}

		repeat->http = http;
		repeat->timeout = repeatTimeout;

		NewThreadAsync(HttpRequestRepeat, NULL, repeat);
	}

	return NULL;
}

#endif

void HTTPClient::Request(const CString& url, char* post, bool threaded, int retry, int timeout, bool cache, void* userData, onStreamCallback onstream, onErrorCallback onerror, onRetryCallback onretry, onFatalCallback onfatal, onReleaseCallback onrelease) throw(const char*) {
	HTTPRequest* http = memNew(http, HTTPRequest());

	http->url = url;
	http->threaded = threaded;
	http->retry = retry;
	http->timeout = timeout;
	http->post = post;
	http->cache = cache;
	http->cacheStream = NULL;
	http->userData = userData;

	http->onstream = onstream;
	http->onerror = onerror;
	http->onretry = onretry;
	http->onfatal = onfatal;
	http->onrelease = onrelease;

#if defined(__IOS__)
	if (http->threaded) {
		if (IsMainThread()) {
			HttpAsync(http);
		} else {
			MainThreadAsync(HttpAsync, NULL, http);
		}
	} else {
		if (IsMainThread()) {
			HttpSync(http);
		} else {
			(void)MainThreadSync(HttpSync, http);
		}
	}
#elif defined(__JNI__)
	if (http->threaded) {
		NewThreadAsync(HttpRequest, HttpRequestResult, http);
	} else {
		if (IsMainThread()) {
			LOG("Error: Main thread sync request not supported by platform!");
		} else {
			int repeatTimeout = 0;
			do {
				repeatTimeout = (int) (intptr_t) (HttpRequestResultSync(HttpRequest(http)));
			} while (repeatTimeout != 0);
		}
	}
#else
	#error Platform not supported!
#endif
}

void uHTTPClientInit() {
#if defined(__JNI__)
	JNIEnv* env = GetThreadEnv();

	jclass clazz;

	clazz = env->FindClass(clsHttpClient);
	if (env->ExceptionCheck()) {
		LOG("JNIEnv: FindClass %s (Fail)", clsHttpClient);
		clazzHttpClient = NULL;
	} else {
		LOG("JNIEnv: FindClass %s (OK)", clsHttpClient);
		clazzHttpClient = (jclass)(env->NewGlobalRef(clazz));
	}

	clazz = env->FindClass(clsHttpResponse);
	if (env->ExceptionCheck()) {
		LOG("JNIEnv: FindClass %s (Fail)", clsHttpResponse);
		clazzHttpResponse = NULL;
	} else {
		LOG("JNIEnv: FindClass %s (OK)", clsHttpResponse);
		clazzHttpResponse = (jclass)(env->NewGlobalRef(clazz));
	}

	ReleaseThreadEnv();
#endif
}

void uHTTPClientQuit() {
#if defined(__JNI__)
	JNIEnv* env = GetThreadEnv();

	if (clazzHttpClient != NULL) {
		env->DeleteGlobalRef(clazzHttpClient);
	}

	if (clazzHttpResponse != NULL) {
		env->DeleteGlobalRef(clazzHttpResponse);
	}

	ReleaseThreadEnv();
#endif
}
