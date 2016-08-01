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

public class EInvalidPointer extends EGlobal {

    private static final long serialVersionUID = 263522194919588044L;
    private static final String eInvalidPointer = "Invalid Pointer";

    public EInvalidPointer() {
        super(eInvalidPointer);
    }

    public EInvalidPointer(String detailMessage) {
        super(detailMessage);
    }

    public EInvalidPointer(Throwable throwable) {
        super(eInvalidPointer, throwable);
    }

    public EInvalidPointer(String detailMessage, Throwable throwable) {
        super(detailMessage, throwable);
    }

}
