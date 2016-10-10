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

#import <UIKit/UIKit.h>
#import <UIKit/UIWebView.h>

@interface OMWebView : UIWebView <UIWebViewDelegate>

- (instancetype _Nonnull) init;
- (void) engine:(void* _Nonnull)refEngine index:(int)index;

- (BOOL) webView:(UIWebView * _Nonnull)webView shouldStartLoadWithRequest:(NSURLRequest * _Nonnull)request navigationType:(UIWebViewNavigationType)navigationType;
- (void) webViewDidStartLoad:(UIWebView * _Nonnull)webView;
- (void) webViewDidFinishLoad:(UIWebView * _Nonnull)webView;
- (void) webView:(UIWebView * _Nonnull)webView didFailLoadWithError:(NSError * _Nonnull)error;

@end
