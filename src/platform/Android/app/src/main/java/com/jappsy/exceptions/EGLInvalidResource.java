package com.jappsy.exceptions;

public class EGLInvalidResource extends EGL {

	private static final long serialVersionUID = -5668646538866434923L;
	private static final String eGLInvalidResource = "OpenGL Error: Invalid Resource";

	public EGLInvalidResource() {
		super(eGLInvalidResource);
	}

	public EGLInvalidResource(String detailMessage) {
		super(detailMessage);
	}

	public EGLInvalidResource(Throwable throwable) {
		super(eGLInvalidResource, throwable);
	}

	public EGLInvalidResource(String detailMessage, Throwable throwable) {
		super(detailMessage, throwable);
	}

}
