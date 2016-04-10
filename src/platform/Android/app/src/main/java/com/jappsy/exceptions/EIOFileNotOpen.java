package com.jappsy.exceptions;

public class EIOFileNotOpen extends EIO {

	private static final long serialVersionUID = 6589548006313165569L;
	public static final String eIOErrorFileNotOpen = "I/O Error: File Not Open";

	public EIOFileNotOpen() {
		super(eIOErrorFileNotOpen);
	}

	public EIOFileNotOpen(String detailMessage) {
		super(detailMessage);
	}

	public EIOFileNotOpen(Throwable throwable) {
		super(eIOErrorFileNotOpen, throwable);
	}

	public EIOFileNotOpen(String detailMessage, Throwable throwable) {
		super(detailMessage, throwable);
	}

}
