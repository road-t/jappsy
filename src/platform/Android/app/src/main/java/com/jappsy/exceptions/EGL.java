package com.jappsy.exceptions;

public class EGL extends EGlobal {

	private static final long serialVersionUID = 964824524675022277L;
	private static final String eGLError = "OpenGL Error";

	public EGL() {
		super(eGLError);
	}

	public EGL(String detailMessage) {
		super(detailMessage);
	}

	public EGL(Throwable throwable) {
		super(eGLError, throwable);
	}

	public EGL(String detailMessage, Throwable throwable) {
		super(detailMessage, throwable);
	}

}
