extern const char *g_term_error;

#include <signal.h>

#define TERM_ERROR(cond, fmt, ...)                                             \
	do {                                                                       \
		if (!(cond)) {                                                         \
			static char msg[512];                                              \
			snprintf(msg, sizeof(msg), "%s:%d: %s " fmt "\n", __FILE__,        \
					 __LINE__, #cond, ##__VA_ARGS__);                          \
			g_term_error = msg;                                                \
			raise(SIGABRT);                                                    \
		}                                                                      \
	} while (0)
