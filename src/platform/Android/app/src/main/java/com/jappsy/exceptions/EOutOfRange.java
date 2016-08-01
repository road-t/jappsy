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

public class EOutOfRange extends EGlobal {

    private static final long serialVersionUID = 6408016773536511917L;
    private static final String eOutOfRange = "Out Of Range";

    public EOutOfRange() {
        super(eOutOfRange);
    }

    public EOutOfRange(String detailMessage) {
        super(detailMessage);
    }

    public EOutOfRange(Throwable throwable) {
        super(eOutOfRange, throwable);
    }

    public EOutOfRange(String detailMessage, Throwable throwable) {
        super(detailMessage, throwable);
    }

}
