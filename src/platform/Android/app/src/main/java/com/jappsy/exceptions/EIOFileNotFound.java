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

public class EIOFileNotFound extends EIO {

	private static final long serialVersionUID = -3317554455244386431L;
	public static final String eIOErrorFileNotFound = "I/O Error: File Not Found";

	public EIOFileNotFound() {
		super(eIOErrorFileNotFound);
	}

	public EIOFileNotFound(String detailMessage) {
		super(detailMessage);
	}

	public EIOFileNotFound(Throwable throwable) {
		super(eIOErrorFileNotFound, throwable);
	}

	public EIOFileNotFound(String detailMessage, Throwable throwable) {
		super(detailMessage, throwable);
	}

}
