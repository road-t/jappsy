package com.jappsy.exceptions;

public class EIOReadLimit extends EIO {

	private static final long serialVersionUID = -8492255848351850184L;
	private static final String eIOErrorReadLimit = "I/O Error: Read Limit Exceed";

	public EIOReadLimit() {
		super(eIOErrorReadLimit);
	}

	public EIOReadLimit(String detailMessage) {
		super(detailMessage);
	}

	public EIOReadLimit(Throwable throwable) {
		super(eIOErrorReadLimit, throwable);
	}

	public EIOReadLimit(String detailMessage, Throwable throwable) {
		super(detailMessage, throwable);
	}

}
