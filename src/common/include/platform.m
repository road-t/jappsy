#import "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

void CNSLog(const char* format, ...) {
	va_list args;
	va_start(args, format);
	NSString *fmt = [NSString stringWithFormat:@"%s", format];
	NSLogv(fmt, args);
	va_end(args);
}

#ifdef __cplusplus
}
#endif
