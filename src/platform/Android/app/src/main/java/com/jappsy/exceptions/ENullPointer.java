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

public class ENullPointer extends EGlobal {

    private static final long serialVersionUID = 805912871065085598L;
    private static final String eNullPointer = "Null Pointer";

    public ENullPointer() {
        super(eNullPointer);
    }

    public ENullPointer(String detailMessage) {
        super(detailMessage);
    }

    public ENullPointer(Throwable throwable) {
        super(eNullPointer, throwable);
    }

    public ENullPointer(String detailMessage, Throwable throwable) {
        super(detailMessage, throwable);
    }

}
