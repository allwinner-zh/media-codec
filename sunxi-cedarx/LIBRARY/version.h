#ifndef CDX_VERSION_H
#define CDX_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAJOR_VERSION "2.1"
#define LIBRARY_SVN_REPOSITORY ""
#define LIBRARY_SVN_VERSION ""
#define LIBRARY_SVN_DATE ""
#define LIBRARY_RELEASE_AUTHOR "xxx"

static inline void LogVersionInfo(void)
{
    logd("\n"
         ">>>>>>>>>>>>>>>>>>>>>>>>>>>>> CedarX 2.0 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n" 
         "version: %s.%s\n"
         "svn repo:'%s'\n"
         "last change date: %s\n"
         "author: %s\n"
         "----------------------------------------------------------------------\n",
         MAJOR_VERSION, LIBRARY_SVN_VERSION,
         LIBRARY_SVN_REPOSITORY,
         LIBRARY_SVN_DATE,
         LIBRARY_RELEASE_AUTHOR);
}

/* usage: TagVersionInfo(myLibTag) */
#define TagVersionInfo(tag) \
    static void VersionInfo_##tag(void) __attribute__((constructor));\
    void VersionInfo_##tag(void) \
    { \
        logd("-------library tag: %s-------", #tag);\
        LogVersionInfo(); \
    }


#ifdef __cplusplus
}
#endif

#endif

