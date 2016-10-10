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

- (instancetype) init;
- (void) engine:(void*)refEngine index:(int)index;

- (void) loadRequest:(NSURLRequest *)request;
- (void) loadHTMLString:(NSString *)string baseURL:(nullable NSURL *)baseURL;
- (void) loadData:(NSData *)data MIMEType:(NSString *)MIMEType textEncodingName:(NSString *)textEncodingName baseURL:(NSURL *)baseURL;
- (BOOL) webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType;
- (void) webViewDidStartLoad:(UIWebView *)webView;
- (void) webViewDidFinishLoad:(UIWebView *)webView;
- (void) webView:(UIWebView *)webView didFailLoadWithError:(NSError *)error;

@end
