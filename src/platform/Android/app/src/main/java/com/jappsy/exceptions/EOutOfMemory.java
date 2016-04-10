package com.jappsy.exceptions;

public class EOutOfMemory extends EGlobal {

	private static final long serialVersionUID = 4363935052299232040L;
	private static final String eOutOfMemory = "Out of Memory";
	
	public EOutOfMemory() {
		super(eOutOfMemory);
	}

	public EOutOfMemory(String detailMessage) {
		super(detailMessage);
	}

	public EOutOfMemory(Throwable throwable) {
		super(eOutOfMemory, throwable);
	}

	public EOutOfMemory(String detailMessage, Throwable throwable) {
		super(detailMessage, throwable);
	}

}
