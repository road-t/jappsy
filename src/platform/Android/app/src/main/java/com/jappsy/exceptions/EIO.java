package com.jappsy.exceptions;

public class EIO extends EGlobal {

	private static final long serialVersionUID = 4643547918910849364L;
	private static final String eIOError = "I/O Error";

	public EIO() {
		super(eIOError);
	}

	public EIO(String detailMessage) {
		super(detailMessage);
	}

	public EIO(Throwable throwable) {
		super(eIOError, throwable);
	}

	public EIO(String detailMessage, Throwable throwable) {
		super(detailMessage, throwable);
	}

}
