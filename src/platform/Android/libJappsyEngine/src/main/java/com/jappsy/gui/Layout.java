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

public class Layout {
	public static final int LEFT = 0x0001;
	public static final int TOP = 0x0002;
	public static final int RIGHT = 0x0004;
	public static final int BOTTOM = 0x0008;
	public static final int WIDTH = 0x0010;
	public static final int HEIGHT = 0x0020;
	public static final int ALPHA = 0x0040;

	private double m_left;
	private double m_top;
	private double m_right;
	private double m_bottom;
	private double m_width;
	private double m_height;
	private double m_alpha = 1.0;

	private int m_state = 0;

	public void resetState() {
		m_state = 0;
	}

	public int getState() {
		return m_state;
	}

	public void setLeft(double left) {
		m_left = left;
		m_state |= LEFT;

		if ((m_state & WIDTH) != 0) {
			m_state &= (~RIGHT);
		}
	}

	public void setRight(double right) {
		m_right = right;
		m_state |= RIGHT;

		if ((m_state & WIDTH) != 0) {
			m_state &= (~LEFT);
		}
	}

	public void setTop(double top) {
		m_top = top;
		m_state |= TOP;

		if ((m_state & HEIGHT) != 0) {
			m_state &= (~BOTTOM);
		}
	}

	public void setBottom(double bottom) {
		m_bottom = bottom;
		m_state |= BOTTOM;

		if ((m_state & HEIGHT) != 0) {
			m_state &= (~TOP);
		}
	}

	public void setWidth(double width) {
		m_width = width;
		m_state |= WIDTH;

		if ((m_state & LEFT) != 0) {
			m_state &= (~RIGHT);
		}
	}

	public void setHeight(double height) {
		m_height = height;
		m_state |= HEIGHT;

		if ((m_state & TOP) != 0) {
			m_state &= (~BOTTOM);
		}
	}

	public void setAlpha(double alpha) {
		m_alpha = alpha;
		m_state |= ALPHA;
	}

	public boolean isReady() {
		int width = 0;

		if ((m_state & LEFT) != 0) {
			width++;
		}

		if ((m_state & RIGHT) != 0) {
			width++;
		}

		if ((m_state & WIDTH) != 0) {
			width++;
		}

		if (width < 2) {
			return false;
		}

		int height = 0;

		if ((m_state & TOP) != 0) {
			height++;
		}

		if ((m_state & BOTTOM) != 0) {
			height++;
		}

		if ((m_state & HEIGHT) != 0) {
			height++;
		}

		return height >= 2;
	}

	public double getLeft() {
		if ((m_state & LEFT) != 0) {
			return m_left;
		} else if ((m_state & (RIGHT | WIDTH)) == (RIGHT | WIDTH)) {
			return m_right - m_width;
		}
		return 0;
	}

	public double getRight() {
		if ((m_state & RIGHT) != 0) {
			return m_right;
		} else if ((m_state & (LEFT | WIDTH)) == (LEFT | WIDTH)) {
			return m_left + m_width;
		}
		return 0;
	}

	public double getTop() {
		if ((m_state & TOP) != 0) {
			return m_top;
		} else if ((m_state & (BOTTOM | HEIGHT)) == (BOTTOM | HEIGHT)) {
			return m_bottom - m_height;
		}
		return 0;
	}

	public double getBottom() {
		if ((m_state & BOTTOM) != 0) {
			return m_bottom;
		} else if ((m_state & (TOP | HEIGHT)) == (TOP | HEIGHT)) {
			return m_top + m_height;
		}
		return 0;
	}

	public double getWidth() {
		if ((m_state & WIDTH) != 0) {
			return m_width;
		} else if ((m_state & (LEFT | RIGHT)) == (LEFT | RIGHT)) {
			return m_right - m_left;
		}
		return 0;
	}

	public double getHeight() {
		if ((m_state & HEIGHT) != 0) {
			return m_height;
		} else if ((m_state & (TOP | BOTTOM)) == (TOP | BOTTOM)) {
			return m_bottom - m_top;
		}
		return 0;
	}

	public double getAlpha() {
		return m_alpha;
	}
}
