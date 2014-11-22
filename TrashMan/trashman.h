#ifndef trashman_h
#define trashman_h
#ifdef __APPLE__
  #define p_lgetxattr(path,name,value,size) getxattr(path,name,value,size,0,XATTR_NOFOLLOW)
  #define p_lsetxattr(path,name,value,size) setxattr(path,name,value,size,0,XATTR_NOFOLLOW)
#else
  #define p_lgetxattr(path,name,value,size) lgetxattr(path,name,value,size)
  #define p_lsetxattr(path,name,value,size) lsetxattr(path,name,value,size,0)
#endif

// At some point Linux dropped support for ENOATTR as an alias for ENODATA
#ifndef ENOATTR
  #define ENOATTR ENODATA
#endif
#endif
