#ifndef _dbg_h
#define _dbg_h

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...) fprintf(stderr, "%s:%d: [DBG] >> " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

/**
 * Get errno or "None" if it's not set
 */
#define clean_errno() (errno == 0 ? "" : strerror(errno))

/** General log macro with level string */
#define loge(lvl, M, ...) fprintf(stderr, "%s:%d: %s [" lvl "] >> " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

/** Log error */
#define log_err(M, ...) loge("ERR ", M, ##__VA_ARGS__)

/** Log warning */
#define log_warn(M, ...) loge("WARN", M, ##__VA_ARGS__)

/** Log info */
#define log_info(M, ...) fprintf(stderr, "%s:%d: [INFO] >> " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

/** Check condition */
#define check(A, M, ...) if (!(A)) { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

/** Always fail */
#define sentinel(M, ...) { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

/** Check memory */
#define check_mem(A) check((A), "Out of Memory.")

/** Check condition debug */
#define check_debug(A, M, ...) if (!(A)) { debug(M, ##__VA_ARGS__); errno = 0; goto error; }

#endif
