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

public class EIOReadLimit extends EIO {

	private static final long serialVersionUID = -8492255848351850184L;
	private static final String eIOErrorReadLimit = "I/O Error: Read Limit Exceed";

	public EIOReadLimit() {
		super(eIOErrorReadLimit);
	}

	public EIOReadLimit(String detailMessage) {
		super(detailMessage);
	}

	public EIOReadLimit(Throwable throwable) {
		super(eIOErrorReadLimit, throwable);
	}

	public EIOReadLimit(String detailMessage, Throwable throwable) {
		super(detailMessage, throwable);
	}

}
