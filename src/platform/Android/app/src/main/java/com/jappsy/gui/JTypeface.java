package com.jappsy.gui;

import android.graphics.Typeface;

public class JTypeface {
	public final static int DEFAULT = 8;
	public final static int DEFAULT_BOLD = 9;
	public final static int MONOSPACE = 4;
	public final static int SANS_SERIF = 8;
	public final static int SERIF = 0;
	
	public final static int NORMAL = 0;
	public final static int BOLD = 1;
	public final static int ITALIC = 2;
	public final static int BOLD_ITALIC = 3;
	
	public Typeface m_typeface;
	public int style;
	
	public JTypeface(int typeface) {
		if ((typeface >= 0) && (typeface < 12)) {
			style = typeface;
			
			switch (typeface) {
			case 0: m_typeface = Typeface.SERIF; break;
			case 1: m_typeface = Typeface.create(Typeface.SERIF, Typeface.BOLD);
			case 2: m_typeface = Typeface.create(Typeface.SERIF, Typeface.ITALIC);
			case 3: m_typeface = Typeface.create(Typeface.SERIF, Typeface.BOLD_ITALIC);
			case 4: m_typeface = Typeface.MONOSPACE; break;
			case 5: m_typeface = Typeface.create(Typeface.MONOSPACE, Typeface.BOLD);
			case 6: m_typeface = Typeface.create(Typeface.MONOSPACE, Typeface.ITALIC);
			case 7: m_typeface = Typeface.create(Typeface.MONOSPACE, Typeface.BOLD_ITALIC);
			case 8: m_typeface = Typeface.SANS_SERIF; break;
			case 9: m_typeface = Typeface.create(Typeface.SANS_SERIF, Typeface.BOLD);
			case 10: m_typeface = Typeface.create(Typeface.SANS_SERIF, Typeface.ITALIC);
			case 11: m_typeface = Typeface.create(Typeface.SANS_SERIF, Typeface.BOLD_ITALIC);
			}
		} else {
			style = 0;
			m_typeface = Typeface.SANS_SERIF;
		}
	}
}
