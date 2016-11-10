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

package com.jappsy.gui;

import android.view.View;

public class Constraint {
	public static final int NONE = 0;
	public static final int LEFT = 1;
	public static final int RIGHT = 2;
	public static final int TOP = 3;
	public static final int BOTTOM = 4;
	public static final int CENTERX = 5;
	public static final int CENTERY = 6;
	public static final int WIDTH = 7;
	public static final int HEIGHT = 8;
	public static final int ALPHA = 9;

	public View m_target;
	public int m_targetAttribute;
	public View m_relate;
	public int m_relateAttribute;
	public double m_multiplier;
	public double m_value;
	public int m_weight;

	public Constraint(View target, int targetAttribute, View relate, int relateAttribute, double multiplier, double value) {
		initConstraint(target, targetAttribute, relate, relateAttribute, multiplier, value, 1000);
	}

	public Constraint(View target, int targetAttribute, View relate, int relateAttribute, double multiplier, double value, int weight) {
		initConstraint(target, targetAttribute, relate, relateAttribute, multiplier, value, weight);
	}

	private void initConstraint(View target, int targetAttribute, View relate, int relateAttribute, double multiplier, double value, int weight) {
		m_target = target;
		m_targetAttribute = targetAttribute;
		m_relate = relate;
		m_relateAttribute = relateAttribute;
		m_multiplier = multiplier;
		m_value = value;
		m_weight = weight;
	}
}
