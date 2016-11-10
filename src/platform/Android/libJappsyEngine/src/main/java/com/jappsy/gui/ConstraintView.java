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

import android.app.Activity;
import android.content.Context;
import android.os.Handler;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;

import com.jappsy.JappsyView;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Map;

public class ConstraintView extends ViewGroup {

	public static class ConstraintComparator implements Comparator<Constraint> {
		public int compare(Constraint left, Constraint right) {
			return left.m_weight < right.m_weight ? 1 : (left.m_weight == right.m_weight ? 0 : -1);
		}
	}

	private static ConstraintComparator m_constraintsComparator = new ConstraintComparator();

	private ArrayList<Constraint> m_currentConstraints = new ArrayList<Constraint>();
	private ArrayList<Constraint> m_targetConstraints = new ArrayList<Constraint>();

	private Map<View, Layout> m_currentLayouts = new HashMap<View, Layout>();
	private Map<View, Layout> m_targetLayouts = new HashMap<View, Layout>();

	private boolean m_update = true;

	private Handler m_handler = new Handler();

	public interface AnimationCallbacks {
		void onStart();
		void onComplete(boolean finished);
	}

	private static class AnimatedLayout {
		public Layout startLayout;
		public Layout endLayout;
	}

	private double m_animatedDuration = -1.0;
	private long m_animatedStart;
	private long m_animatedEnd;
	private AnimationCallbacks m_animatedCallbacks;
	private Map<View, AnimatedLayout> m_animatedLayouts = new HashMap<View, AnimatedLayout>();

	private long m_lastAnimatedTime;

	private Runnable m_animatedRunnable = new Runnable() {
		@Override public void run() {
			long currentTime = System.currentTimeMillis();
			if (currentTime >= m_animatedEnd) {
				layoutIfNeeded();
			} else {
				double interval = (double)(currentTime - m_animatedStart) / (double)(m_animatedEnd - m_animatedStart);

				for (Map.Entry<View, AnimatedLayout> entry : m_animatedLayouts.entrySet()) {
					View view = entry.getKey();
					AnimatedLayout ani = entry.getValue();
					Layout current = m_currentLayouts.get(view);

					if (current != null) {
						double sl = ani.startLayout.getLeft();
						double st = ani.startLayout.getTop();
						double sr = ani.startLayout.getRight();
						double sb = ani.startLayout.getBottom();
						double sa = ani.startLayout.getAlpha();

						double el = ani.endLayout.getLeft();
						double et = ani.endLayout.getTop();
						double er = ani.endLayout.getRight();
						double eb = ani.endLayout.getBottom();
						double ea = ani.endLayout.getAlpha();

						current.resetState();
						current.setLeft((el - sl) * interval + sl);
						current.setTop((et - st) * interval + st);
						current.setRight((er - sr) * interval + sr);
						current.setBottom((eb - sb) * interval + sb);
						current.setAlpha((ea - sa) * interval + sa);
/*
						view.setLeft((int)current.getLeft());
						view.setTop((int)current.getTop());
						view.setRight((int)current.getRight());
						view.setBottom((int)current.getBottom());
						//view.layout((int)current.getLeft(), (int)current.getTop(), (int)current.getRight(), (int)current.getBottom());
						view.setAlpha((float)current.getAlpha());
*/
						view.invalidate();
						view.requestLayout();
					}
				}

				long delta = (currentTime - m_lastAnimatedTime);
				m_lastAnimatedTime = currentTime;

				if (delta >= 15) {
					m_handler.post(m_animatedRunnable);
				} else {
					m_handler.postDelayed(m_animatedRunnable, 15 - delta);
				}
			}
		}
	};

	public void addConstraint(Constraint constraint) {
		if (!m_targetConstraints.contains(constraint)) {
			m_targetConstraints.add(constraint);
			m_update = true;
		}
	}

	public void addConstraints(ArrayList<Constraint> constraints) {
		for (Constraint constraint : constraints) {
			addConstraint(constraint);
		}
	}

	public void removeConstraint(Constraint constraint) {
		if (m_targetConstraints.remove(constraint)) {
			m_update = true;
		}
	}

	public void removeConstraints(ArrayList<Constraint> constraints) {
		for (Constraint constraint : constraints) {
			removeConstraint(constraint);
		}
	}

	public void clearConstraints() {
		m_targetConstraints.clear();
		m_update = true;
	}

	private void fillLayouts(int l, int t, int r, int b, ArrayList<Constraint> constraints, Map<View, Layout> layouts) {
		layouts.clear();

		for (Constraint constraint : constraints) {
			Layout layout = layouts.get(constraint.m_target);
			if (layout == null) {
				layout = new Layout();
				layouts.put(constraint.m_target, layout);
			}

			double value = 0.0;

			if (constraint.m_relate != null) {
				if (constraint.m_relate == this) {
					switch (constraint.m_relateAttribute) {
						case Constraint.LEFT:
							value = l;
							break;
						case Constraint.RIGHT:
							value = r;
							break;
						case Constraint.TOP:
							value = t;
							break;
						case Constraint.BOTTOM:
							value = b;
							break;
						case Constraint.CENTERX:
							value = (double) (l + r) / 2.0;
							break;
						case Constraint.CENTERY:
							value = (double) (t + b) / 2.0;
							break;
						case Constraint.WIDTH:
							value = (r - l);
							break;
						case Constraint.HEIGHT:
							value = (b - t);
							break;
						case Constraint.ALPHA:
							value = this.getAlpha();
							break;
						default:
							value = 0;
							break;
					}

					value *= constraint.m_multiplier;
				} else {
					Layout relateLayout = layouts.get(constraint.m_relate);
					if (relateLayout != null) {
						switch (constraint.m_relateAttribute) {
							case Constraint.LEFT:
								value = relateLayout.getLeft();
								break;
							case Constraint.RIGHT:
								value = relateLayout.getRight();
								break;
							case Constraint.TOP:
								value = relateLayout.getTop();
								break;
							case Constraint.BOTTOM:
								value = relateLayout.getBottom();
								break;
							case Constraint.CENTERX:
								value = (relateLayout.getLeft() + relateLayout.getRight()) / 2.0;
								break;
							case Constraint.CENTERY:
								value = (relateLayout.getTop() + relateLayout.getBottom()) / 2.0;
								break;
							case Constraint.WIDTH:
								value = relateLayout.getWidth();
								break;
							case Constraint.HEIGHT:
								value = relateLayout.getHeight();
								break;
							case Constraint.ALPHA:
								value = relateLayout.getAlpha();
								break;
							default:
								value = 0;
								break;
						}

						value *= constraint.m_multiplier;
					}
				}
			}

			if (constraint.m_targetAttribute != Constraint.ALPHA) {
				value += constraint.m_value * m_scale;
			} else {
				value += constraint.m_value;
			}

			switch (constraint.m_targetAttribute) {
				case Constraint.LEFT:
					layout.setLeft(value);
					break;
				case Constraint.RIGHT:
					layout.setRight(value);
					break;
				case Constraint.TOP:
					layout.setTop(value);
					break;
				case Constraint.BOTTOM:
					layout.setBottom(value);
					break;
				case Constraint.WIDTH:
					layout.setWidth(value);
					break;
				case Constraint.HEIGHT:
					layout.setHeight(value);
					break;
				case Constraint.ALPHA:
					layout.setAlpha(value);
					break;
			}
		}
	}

	private boolean updateLayouts(int l, int t, int r, int b) {
		boolean animated = false;

		if (m_update) {
			m_update = false;

			fillLayouts(l, t, r, b, m_currentConstraints, m_currentLayouts);
			Collections.sort(m_targetConstraints, m_constraintsComparator);
			fillLayouts(l, t, r, b, m_targetConstraints, m_targetLayouts);

			m_currentConstraints.clear();
			for (Constraint constraint : m_targetConstraints) {
				m_currentConstraints.add(constraint);
			}

			for (Map.Entry<View, Layout> entry : m_targetLayouts.entrySet()) {
				View view = entry.getKey();
				if (!m_currentLayouts.containsKey(view)) {
					m_currentLayouts.put(view, entry.getValue());
				}
			}

			for (Map.Entry<View, Layout> entry : m_targetLayouts.entrySet()) {
				View view = entry.getKey();
				Layout currentLayout = m_currentLayouts.get(view);
				Layout endLayout = entry.getValue();

				if (currentLayout != endLayout) {
					double cl = currentLayout.getLeft();
					double ct = currentLayout.getTop();
					double cw = currentLayout.getWidth();
					double ch = currentLayout.getHeight();
					double ca = currentLayout.getAlpha();

					double el = endLayout.getLeft();
					double et = endLayout.getTop();
					double ew = endLayout.getWidth();
					double eh = endLayout.getHeight();
					double ea = endLayout.getAlpha();

					if ((cl != el) || (ct != et) || (cw != ew) || (ch != eh) || (ca != ea)) {
						Layout startLayout = new Layout();
						startLayout.setLeft(cl);
						startLayout.setTop(ct);
						startLayout.setWidth(cw);
						startLayout.setHeight(ch);
						startLayout.setAlpha(ca);

						AnimatedLayout ani = new AnimatedLayout();
						ani.startLayout = startLayout;
						ani.endLayout = endLayout;

						m_animatedLayouts.put(view, ani);

						animated = true;
					} else {
						m_currentLayouts.put(view, endLayout);
					}
				}
			}

			/*
			for (Map.Entry<View, Layout> entry : m_currentLayouts.entrySet()) {
				View view = entry.getKey();
				Layout current = entry.getValue();

				RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams((int)current.getWidth(), (int)current.getHeight());
				params.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
				params.addRule(RelativeLayout.ALIGN_PARENT_TOP);
				params.leftMargin = (int)current.getLeft();
				params.topMargin = (int)current.getTop();
				view.setLayoutParams(params);
			}*/
		}

		if (animated) {
			if (m_animatedDuration < 0.0) {
				m_animatedDuration = 0.5;
			}

			m_animatedStart = m_lastAnimatedTime = System.currentTimeMillis();
			m_animatedEnd = m_animatedStart + (long)(m_animatedDuration * 1000.0);

			m_handler.removeCallbacks(m_animatedRunnable);
			m_handler.post(m_animatedRunnable);

			return true;
		}

		return false;
	}

	public void layoutIfNeeded() {
		updateLayouts(0, 0, getRight()-getLeft(), getBottom()-getTop());

		if (m_animatedLayouts.size() != 0) {
			for (Map.Entry<View, AnimatedLayout> entry : m_animatedLayouts.entrySet()) {
				View view = entry.getKey();
				Layout current = entry.getValue().endLayout;

				m_currentLayouts.put(view, current);
/*
				view.setLeft((int)current.getLeft());
				view.setTop((int)current.getTop());
				view.setRight((int)current.getRight());
				view.setBottom((int)current.getBottom());
				//view.layout((int)current.getLeft(), (int)current.getTop(), (int)current.getRight(), (int)current.getBottom());
				view.setAlpha((float)current.getAlpha());
*/
				view.invalidate();
				view.requestLayout();
			}
			m_animatedLayouts.clear();
		}

		if (m_animatedCallbacks != null) {
			m_animatedCallbacks.onComplete(true);
			m_animatedCallbacks = null;
		}

		m_animatedDuration = -1.0;
	}

	public void animate(double duration, AnimationCallbacks callbacks) {
		if (m_animatedCallbacks != null) {
			m_animatedCallbacks.onComplete(false);
			m_animatedCallbacks = null;
		}

		m_animatedDuration = duration;
		m_animatedCallbacks = callbacks;

		if (!updateLayouts(0, 0, getRight()-getLeft(), getBottom()-getTop())) {
			layoutIfNeeded();
		}
	}

	private double m_scale = 1.0;

	public ConstraintView(Context context) {
		super(context);

		Display display = ((Activity)context).getWindowManager().getDefaultDisplay();
		DisplayMetrics metrics = new DisplayMetrics();
		display.getMetrics(metrics);
		m_scale = metrics.density;

		setLayoutParams(new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));

		invalidate();
	}

	int lastWidthMeasureSpec = 0;
	int lastHeightMeasureSpec = 0;

	@Override protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		final int widthMode = MeasureSpec.getMode(widthMeasureSpec);
		final int heightMode = MeasureSpec.getMode(heightMeasureSpec);
		final int widthSize = MeasureSpec.getSize(widthMeasureSpec);
		final int heightSize = MeasureSpec.getSize(heightMeasureSpec);

		if (((widthMode == MeasureSpec.UNSPECIFIED) || (widthMode == MeasureSpec.EXACTLY)) &&
				((heightMode == MeasureSpec.UNSPECIFIED) || (heightMode == MeasureSpec.EXACTLY))) {
			if ((lastWidthMeasureSpec != widthMeasureSpec) || (lastHeightMeasureSpec != heightMeasureSpec)) {
				lastWidthMeasureSpec = widthMeasureSpec;
				lastHeightMeasureSpec = heightMeasureSpec;
				m_update = true;
			}

			updateLayouts(0, 0, widthSize, heightSize);

			for (Map.Entry<View, Layout> entry : m_currentLayouts.entrySet()) {
				View view = entry.getKey();
				if (view.getVisibility() != View.GONE) {
					Layout current = entry.getValue();

					view.measure((int) current.getWidth(), (int) current.getHeight());
				}
			}
		} else {
			// Unsupported measure spec
		}

		super.onMeasure(widthMeasureSpec, heightMeasureSpec);
	}

	@Override protected void onLayout(boolean changed, int l, int t, int r, int b) {
		//if (changed) {
		//	m_update = true;
		//}
		//updateLayouts(0, 0, r-l, b-t);

		final int count = getChildCount();

		for (int i = 0; i < count; i++) {
			final View child = getChildAt(i);
			if ((child != null) && (child.getVisibility() != GONE)) {
				Layout layout = m_currentLayouts.get(child);
				if (layout != null) {
					child.layout((int)layout.getLeft(), (int)layout.getTop(), (int)layout.getRight(), (int)layout.getBottom());
					child.setAlpha((float)layout.getAlpha());
				} else {
					child.layout(0, 0, 0, 0);
				}
			}
		}
	}
}
