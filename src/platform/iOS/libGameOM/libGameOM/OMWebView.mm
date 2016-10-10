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

#import "OMWebView.h"

#import <libJappsyEngine/libJappsyEngine.h>
#include "OMGame.h"
#include <core/uMemory.h>

@interface OMWebView()

@property(nonatomic)OMGame *engine;
@property(nonatomic)int index;

@end

@implementation OMWebView

- (instancetype) init {
    if ((self = [super init])) {
        [self setDelegate:self];
        _engine = NULL;
    }
    return self;
}

- (void)engine:(void*)refEngine index:(int)index {
    _engine = (OMGame*)refEngine;
    _index = index;
}

- (void)loadRequest:(NSURLRequest *)request {
    [super loadRequest:request];
}

- (void)loadHTMLString:(NSString *)string baseURL:(nullable NSURL *)baseURL {
    [super loadHTMLString:string baseURL:baseURL];
}

- (void)loadData:(NSData *)data MIMEType:(NSString *)MIMEType textEncodingName:(NSString *)textEncodingName baseURL:(NSURL *)baseURL {
    [super loadData:data MIMEType:MIMEType textEncodingName:textEncodingName baseURL:baseURL];
}

- (BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType {
    NSString* url = [[request URL] absoluteString];
    if ([url hasPrefix:@"ios:"]) {
        if (_engine != NULL) {
            CString location = url;
            _engine->onWebLocation(_index, location);
        }
        
        return NO;
    }
    
    return YES;
}

- (void)webViewDidStartLoad:(UIWebView *)webView {
}

- (void)webViewDidFinishLoad:(UIWebView *)webView {
    if (_engine != NULL) {
        _engine->onWebReady(_index);
    }
}

- (void)webView:(UIWebView *)webView didFailLoadWithError:(NSError *)error {
    if (_engine != NULL) {
        CString err = [error localizedDescription];
        _engine->onWebFail(_index, err);
    }
}

@end
