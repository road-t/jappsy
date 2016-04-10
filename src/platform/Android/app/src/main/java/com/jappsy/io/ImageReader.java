/*
 * Created by VipDev on 02.04.16.
 *
 * Copyright (C) 2016 The Jappsy Open Source Project
 *
 * Project Web Page http://jappsy.com
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

package com.jappsy.io;

import java.io.ByteArrayOutputStream;
import java.io.EOFException;
import java.io.FileDescriptor;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import com.jappsy.core.Log;
import com.jappsy.exceptions.EGlobal;
import com.jappsy.exceptions.EIO;
import com.jappsy.exceptions.EIOInvalidFormat;
import com.jappsy.exceptions.EIOReadLimit;
import com.jappsy.exceptions.ENullPointer;
import com.jappsy.exceptions.EOutOfMemory;
import com.jappsy.io.image.JPEG;
import com.jappsy.io.image.PNG;
import com.jappsy.memory.Memory;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.BitmapRegionDecoder;
import android.graphics.Rect;

public class ImageReader {
	public static final int pngHead = 0x474E5089;		// PNG or MNG Image
	public static final int jpegApp0Head = 0xE0FFD8FF;	// JPEG Image
	public static final int jpegApp1Head = 0xE1FFD8FF;	// JPEG Image
	public static final int sdffHead = 0x46464453;		// Signed Distance Field Font
	public static final int sdfiHead = 0x49464453; 		// Signed Distance Filed Image
	public static final int jimgHead = 0x474D494A;		// Jappsy Image
	private static final int igzChunk = 0x5A472E49; 	// Image Chunk
	private static final int fgzChunk = 0x5A472E46; 	// Font Info Chunk
	private static final int jendChunk = 0x4A454E44;	// End Chunk

	public static int getSize(CacheFile file, int[] whd) throws EOutOfMemory, EIO {
		file.seek(0);
		int head = file.readInt();
		if ((head == sdffHead) || (head == sdfiHead) || (head == jimgHead)) {
			whd[0] = file.readInt();
			whd[1] = file.readInt();
			if (head != jimgHead) {
				whd[2] = file.readUnsignedByte();
			} else {
				whd[2] = 0;
			}
			
			return head;
		} else if ((head == pngHead) || (head == jpegApp0Head) || (head == jpegApp1Head)) {
			file.seek(0);
			
			InputStream is = file.createInputStream(-1);
			try {
				final BitmapFactory.Options getOptions = new BitmapFactory.Options();
				getOptions.inJustDecodeBounds = true;
				getOptions.inScaled = false;
				getOptions.inDensity = 0;
				getOptions.inTargetDensity = 0;
				getOptions.inScreenDensity = 0;
				BitmapFactory.decodeStream(is, null, getOptions);
				whd[0] = getOptions.outWidth;
				whd[1] = getOptions.outHeight;
				whd[2] = 0;
			} catch (OutOfMemoryError e1) {
				throw new EOutOfMemory(e1);
			} finally {
				file.closeStream(is);
			}
			
			return head;
		}

		throw new EIOInvalidFormat();
	}
	
	private static void readFully(InputStream is, int[] intBuffer) throws EOutOfMemory, EIO {
		byte[] buffer = Memory.mmalloc102400();
		try {
			long byteLeft = (long)intBuffer.length * 4;
			int i = 0;
			while (byteLeft > 0) {
				int byteCount = 102400;
				if (byteCount > byteLeft) byteCount = (int)byteLeft;
				int n = 0;
				while (n < byteCount) {
					int count = is.read(buffer, n, byteCount - n);
		            if (count < 0)
		                throw new EOFException();
		            n += count;
		        }
				n = 0;
				while (n < byteCount) {
					intBuffer[i] = ((0x00FF & buffer[n]) << 0) | ((0x00FF & buffer[n+1]) << 8) | ((0x00FF & buffer[n+2]) << 16) | ((0x00FF & buffer[n+3]) << 24);
					i++;
					n += 4;
				}
				byteLeft -= n;
			}
		} catch (EOFException e) {
			throw new EIOReadLimit(e);
		} catch (IOException e) {
			throw new EIO(e);
		} finally {
			Memory.mmfree102400(buffer);
		}
	}

	private static void readFully(InputStream is, int[] intBuffer, int intOffset, int intCount) throws EOutOfMemory, EIO {
		byte[] buffer = Memory.mmalloc102400();
		try {
			long byteLeft = (long)intCount * 4;
			int i = intOffset;
			while (byteLeft > 0) {
				int byteCount = 102400;
				if (byteCount > byteLeft) byteCount = (int)byteLeft;
				int n = 0;
				while (n < byteCount) {
					int count = is.read(buffer, n, byteCount - n);
		            if (count < 0)
		                throw new EOFException();
		            n += count;
		        }
				n = 0;
				while (n < byteCount) {
					intBuffer[i] = ((0x00FF & buffer[n]) << 0) | ((0x00FF & buffer[n+1]) << 8) | ((0x00FF & buffer[n+2]) << 16) | ((0x00FF & buffer[n+3]) << 24);
					i++;
					n += 4;
				}
				byteLeft -= n;
			}
		} catch (EOFException e) {
			throw new EIOReadLimit(e);
		} catch (IOException e) {
			throw new EIO(e);
		} finally {
			Memory.mmfree102400(buffer);
		}
	}

	public static int[] getPixels(CacheFile file, int[] whd) throws EOutOfMemory, EIO {
		file.seek(0);
		
		final int head = file.readInt();
		if ((head == sdffHead) || (head == sdfiHead) || (head == jimgHead)) {
			final int width = file.readInt();
			final int height = file.readInt();
			int distance = 0;
			if (head != jimgHead) {
				distance = file.readUnsignedByte();
			}
			
			if (whd != null) {
				whd[0] = width;
				whd[1] = height;
				whd[2] = distance;
			}

			int len = file.readInt();
			int chunk = file.readInt();
			while (chunk != jendChunk) {
				if (chunk == igzChunk) {
					InputStream is;
					try {
						is = file.createGZIPInputStream(len);
					} catch (EIOInvalidFormat e) {
						is = file.createInputStream(len);
					}
					
					final int intCount = width * height;
					int[] buffer;
					try {
						buffer = new int[intCount];
						readFully(is, buffer);
					} catch (OutOfMemoryError e) {
						throw new EOutOfMemory(e);
					} finally {
						file.closeStream(is);
					}
					
					return buffer;
				} else {
					if (file.skip(len) < 0)
						break;
				}

				len = file.readInt();
				chunk = file.readInt();
			}
		} else if ((head == pngHead) || (head == jpegApp0Head) || (head == jpegApp1Head)) {
			file.seek(0);

			InputStream is = file.createInputStream(-1);
			
			int[] buffer;
			try {
				final BitmapFactory.Options setOptions = new BitmapFactory.Options();
				setOptions.inPreferredConfig = Bitmap.Config.ARGB_8888;
				setOptions.inDither = false;
				setOptions.inScaled = false;
				setOptions.inDensity = 0;
				setOptions.inTargetDensity = 0;
				setOptions.inScreenDensity = 0;
				
				Bitmap bitmap = BitmapFactory.decodeStream(is, null, setOptions);
				if (bitmap != null) {
            		final int width = bitmap.getWidth();
	        		final int height = bitmap.getHeight();

	    			if (whd != null) {
	    				whd[0] = width;
	    				whd[1] = height;
	    				whd[2] = 0;
	    			}

	        		try {
	        			buffer = new int[width * height];
	            		bitmap.getPixels(buffer, 0, width, 0, 0, width, height);
	        		} finally {
	        			bitmap.recycle();
	        		}
	        	} else {
	        		throw new EIOInvalidFormat();
	        	}
			} catch (OutOfMemoryError e) {
				throw new EOutOfMemory(e);
			} finally {
				file.closeStream(is);
			}
			
			return buffer;
		}

		throw new EIOInvalidFormat();
	}

	@SuppressWarnings("unused")
	public static int[] getPixels(CacheFile file, final int fullWidth, final int fullHeight, int x1, int y1, int x2, int y2) throws EOutOfMemory, EIO {
		file.seek(0);
		
		final int head = file.readInt();
		if ((head == sdffHead) || (head == sdfiHead) || (head == jimgHead)) {
			final int width = file.readInt();
			final int height = file.readInt();
			int distance = 0;
			if (head != jimgHead) {
				distance = file.readUnsignedByte();
			}
			
			if ((x1 >= x2) || (y1 >= y2))
				throw new IndexOutOfBoundsException();

			if ((x2 - x1) > fullWidth)
				x2 = fullWidth + x1;
			if ((y2 - y1) > fullHeight)
				y2 = fullHeight + y1;
			if ((x2 < 0) || (y2 < 0))
				throw new IndexOutOfBoundsException();

			int len = file.readInt();
			int chunk = file.readInt();
			while (chunk != jendChunk) {
				if (chunk == igzChunk) {
					InputStream is;
					try {
						is = file.createGZIPInputStream(len);
					} catch (EIOInvalidFormat e) {
						is = file.createInputStream(len);
					}
					
					final int intCount = fullWidth * fullHeight;
					int[] buffer;
					try {
						buffer = new int[intCount];
						int intOffset = 0;
						if (y1 < 0) { intOffset -= y1 * width; y1 = 0; }
						if (x1 < 0) { intOffset -= x1; x1 = 0;  }

						long skipBytes = (y1 * width + x1) * 4; 
						while (skipBytes > 0) {
							long skipped = is.skip(skipBytes);
							skipBytes -= skipped;
						}

						int line = x2 - x1;
						int skip = (width - line) * 4;
						int n = y2 - y1;
						if (skip == 0) {
							readFully(is, buffer, intOffset, line * n);
						} else {
							while (n > 0) {
								readFully(is, buffer, intOffset, line);
								intOffset += fullWidth;
								n--;
								if (n > 0) is.skip(skip);
							}
						}
					} catch (IOException e) {
						throw new EIO(e);
					} catch (OutOfMemoryError e) {
						throw new EOutOfMemory(e);
					} finally {
						file.closeStream(is);
					}
					
					return buffer;
				} else {
					if (file.skip(len) < 0)
						break;
				}

				len = file.readInt();
				chunk = file.readInt();
			}			
		} else if (head == pngHead) {
			int[] buffer = null;
			ByteBuffer bufferFile = null;
			ByteBuffer pixelBuffer = null;
			try {
				FileDescriptor fd = file.getFD();

				bufferFile = FileIO.readFile(fd);
				pixelBuffer = PNG.getPixels(bufferFile, fullWidth, fullHeight, x1, y1, x2, y2);
				buffer = com.jappsy.core.Memory.toIntArray(pixelBuffer);
			} catch (EGlobal e) {
				e.printStackTrace();
			} finally {
				if (bufferFile != null)
					try {
						com.jappsy.core.Memory.free(bufferFile);
					} catch (ENullPointer eNullPointer) {
						eNullPointer.printStackTrace();
					}
				if (pixelBuffer != null)
					try {
						com.jappsy.core.Memory.free(pixelBuffer);
					} catch (ENullPointer eNullPointer) {
						eNullPointer.printStackTrace();
					}
			}

			return buffer;
		} else if ((head == jpegApp0Head) || (head == jpegApp1Head)) {
			int[] buffer = null;
			ByteBuffer bufferFile = null;
			ByteBuffer pixelBuffer = null;
			try {
				FileDescriptor fd = file.getFD();

				bufferFile = FileIO.readFile(fd);
				pixelBuffer = JPEG.getPixels(bufferFile, fullWidth, fullHeight, x1, y1, x2, y2);
				buffer = com.jappsy.core.Memory.toIntArray(pixelBuffer);
			} catch (EGlobal e) {
				e.printStackTrace();
			} finally {
				if (bufferFile != null)
					try {
						com.jappsy.core.Memory.free(bufferFile);
					} catch (ENullPointer eNullPointer) {
						eNullPointer.printStackTrace();
					}
				if (pixelBuffer != null)
					try {
						com.jappsy.core.Memory.free(pixelBuffer);
					} catch (ENullPointer eNullPointer) {
						eNullPointer.printStackTrace();
					}
			}

			return buffer;
		}

		throw new EIOInvalidFormat();
	}
	
	public static final ByteBuffer getFontInfo(CacheFile file, int[] whd) throws EOutOfMemory, EIO {
		file.seek(0);
		
		final int head = file.readInt();
		if (head == sdffHead) {
			final int width = file.readInt();
			final int height = file.readInt();
			final int distance = file.readUnsignedByte();
			
			if (whd != null) {
				whd[0] = width;
				whd[1] = height;
				whd[2] = distance;
			}
			
			int len = file.readInt();
			int chunk = file.readInt();
			while (chunk != jendChunk) {
				if (chunk == fgzChunk) {
					InputStream is;
					try {
						is = file.createGZIPInputStream(len);
					} catch (EIOInvalidFormat e) {
						is = file.createInputStream(len);
					}
					
					ByteBuffer result;
					try {
						ByteArrayOutputStream baos = new ByteArrayOutputStream();
						
				        byte[] buffer = Memory.mmalloc102400();
				        int length;
				        try {
							while ((length = is.read(buffer)) >= 0) {
								baos.write(buffer, 0, length);
							}
						} catch (IOException e) {
							throw new EIO(e);
						} finally {
							Memory.mmfree102400(buffer);
						}
						
						result = ByteBuffer.wrap(baos.toByteArray()).order(ByteOrder.nativeOrder());
					} catch (OutOfMemoryError e) {
						throw new EOutOfMemory(e);
					} finally {
						file.closeStream(is);
					}
					
					return result;
				} else {
					if (file.skip(len) < 0)
						break;
				}

				len = file.readInt();
				chunk = file.readInt();
			}
		}
		
		throw new EIOInvalidFormat();
	}
	
}
