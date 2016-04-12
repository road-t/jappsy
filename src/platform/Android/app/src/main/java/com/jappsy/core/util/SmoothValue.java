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

package com.jappsy.core.util;

public class SmoothValue {

    private final Object m_Lock = new Object();
    private double[] items = null;
    private int count = 0;

    public SmoothValue(int count) {
        this.items = new double[count];
        this.count = count;
        for (int i = 0; i < count; i++) {
            this.items[i] = Double.NaN;
        }
    }

    public void put(double value) {
        synchronized (m_Lock) {
            for (int i = count - 1; i > 0; i--) {
                this.items[i] = this.items[i-1];
            }
            this.items[0] = value;
            m_Lock.notifyAll();
        }
    }

    public double value() {
        double total = 0;
        int count = 0;
        synchronized (m_Lock) {
            for (int i = 0; i < this.count; i++) {
                if (this.items[i] != Double.NaN) {
                    total += this.items[i];
                    count++;
                }
            }
            m_Lock.notifyAll();
        }
        if (count > 0) {
            return total / count;
        }
        return Double.NaN;
    }

}
