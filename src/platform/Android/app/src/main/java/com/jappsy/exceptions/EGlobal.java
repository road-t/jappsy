package com.jappsy.exceptions;

public class EGlobal extends Exception {

	private static final long serialVersionUID = -5831055767798743666L;

	public EGlobal() {
	}

	public EGlobal(String detailMessage) {
		super(detailMessage);
	}

	public EGlobal(Throwable throwable) {
		super(throwable);
	}

	public EGlobal(String detailMessage, Throwable throwable) {
		super(detailMessage, throwable);
	}

}
