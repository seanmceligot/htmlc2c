#ifndef __UTIL_H
#define __UTIL_H

#ifdef  __cplusplus
# define TPE_BEGIN_DECLS  extern "C" {
# define TPE_END_DECLS    }
#else
# define TPE_BEGIN_DECLS
# define TPE_END_DECLS
#endif

#define debug(fmt, args...) gencp_verbose?fprintf(stderr, fmt , ## args):0;
#define error(fmt, args...) fprintf(stderr, fmt , ## args);
#define fatal(fmt, args...) fprintf(stderr, fmt , ## args);abort();

#ifndef BOOL
#define BOOL int
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE !FALSE
#endif

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#endif
