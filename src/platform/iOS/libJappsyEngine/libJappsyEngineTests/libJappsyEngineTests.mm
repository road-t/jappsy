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

#import <XCTest/XCTest.h>

//#import <Foundation/Foundation.h>

#include <platform.h>
#include <jappsy.h>
#include <core/uMemory.h>
#include <core/uString.h>
#include <cipher/uCipher.h>

@interface libJappsyEngineTests : XCTestCase

@end

@implementation libJappsyEngineTests

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
	
	jappsyInit();
}

- (void)tearDown {
	jappsyQuit();
	
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testExample {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
}

- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
		
		String* s = new String(L"Test String: ");
		for (int i = 0; i < 10; i++) {
			s->concat(L"Concat Long String ");
		}
		
		wchar_t* data = (wchar_t*)(*s);
		uint32_t size = 0;
		/*int len = */wcs_strlen(data, &size);
		
		uint32_t gzipSize = 0;
		char* gzip = (char*)gzip_encode_fast(data, size, &gzipSize);
		
		if (gzip != NULL) {
			uint32_t checkSize = 0;
			wchar_t* check = (wchar_t*)gzip_decode(gzip, gzipSize, &checkSize);
			
			if (check != NULL) {
				if (wcscmp(data, check) == 0) {
					// OK
					LOG("GZip %s", "OK");
				} else {
					// Fail
					LOG("GZip %s", "Fail");
				}
				memFree(check);
			}
			
			memFree(gzip);
		}
		
		delete s;
    }];
}

@end
