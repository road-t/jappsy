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
	char UTC[] = "UTC";
	t.tm_zone = UTC;
	
	return ((uint64_t)timegm(&t)) * 1000;
}

CString UTCtoRFC2616(uint64_t time) {
	time_t date = (time_t)(time / 1000);
	struct tm t;
	gmtime_r(&date, &t);

	return CString::format(L"%ls, %02d %ls %04d %02d:%02d:%02d GMT", sWeekDay[t.tm_wday], t.tm_mday, sMonth[t.tm_mon], t.tm_year + 1900, t.tm_hour, t.tm_min, t.tm_sec);
}

#if defined(__IOS__)

#import <UIKit/UIKit.h>

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
				break;
			}

			if (resultStream != NULL) {
				delete resultStream;
			}
			if (resultError != NULL) {
				delete resultError;
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

void* HttpAsync(void* threadData) {
	return NULL;
}

void* HttpSync(void* threadData) {
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
