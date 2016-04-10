package com.jappsy.exceptions;

public class EIOFileNotFound extends EIO {

	private static final long serialVersionUID = -3317554455244386431L;
	public static final String eIOErrorFileNotFound = "I/O Error: File Not Found";

	public EIOFileNotFound() {
		super(eIOErrorFileNotFound);
	}

	public EIOFileNotFound(String detailMessage) {
		super(detailMessage);
	}

	public EIOFileNotFound(Throwable throwable) {
		super(eIOErrorFileNotFound, throwable);
	}

	public EIOFileNotFound(String detailMessage, Throwable throwable) {
		super(detailMessage, throwable);
	}

}
