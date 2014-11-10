#ifndef prename_h
#define prename_h

#ifdef __cplusplus
extern "C" {
#endif

extern void prename_setproctitle_init(int argc, char **argv, char **envp);
extern void prename_setproctitle(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
