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

package com.jappsy.exceptions;

public class EIO extends EGlobal {

	private static final long serialVersionUID = 4643547918910849364L;
	private static final String eIOError = "I/O Error";

	public EIO() {
		super(eIOError);
	}

	public EIO(String detailMessage) {
		super(detailMessage);
	}

	public EIO(Throwable throwable) {
		super(eIOError, throwable);
	}

	public EIO(String detailMessage, Throwable throwable) {
		super(detailMessage, throwable);
	}

}
