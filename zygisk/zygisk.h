struct Api;
struct AppSpecializeArgs;
struct ServerSpecializeArgs;
struct ModuleBase;

struct ModuleBaseVirtualTable {
  // This function is called when the module is loaded into the target process.
  // A Zygisk API handle will be sent as an argument; call utility functions or
  // interface with Zygisk through this handle.
  void (*onLoad)(ModuleBase *`this`, Api *api, JNIEnv *env);

  // This function is called before the app process is specialized.
  // At this point, the process just got forked from zygote, but no app specific
  // specialization is applied. This means that the process does not have any
  // sandbox restrictions and still runs with the same privilege of zygote.
  //
  // All the arguments that will be sent and used for app specialization is
  // passed as a single AppSpecializeArgs object. You can read and overwrite
  // these arguments to change how the app process will be specialized.
  //
  // If you need to run some operations as superuser, you can call
  // Api::connectCompanion() to get a socket to do IPC calls with a root
  // companion process. See Api::connectCompanion() for more info.
  void (*preAppSpecialize)(ModuleBase *`this`, AppSpecializeArgs *args);

  // This function is called after the app process is specialized.
  // At this point, the process has all sandbox restrictions enabled for this
  // application. This means that this function runs as the same privilege of
  // the app's own code.
  void (*postAppSpecialize)(ModuleBase *`this`, const AppSpecializeArgs *args);

  // This function is called before the system server process is specialized.
  // See preAppSpecialize(args) for more info.
  void (*preServerSpecialize)(ModuleBase *`this`, ServerSpecializeArgs *args);

  // This function is called after the system server process is specialized.
  // At this point, the process runs with the privilege of system_server.
  void (*postServerSpecialize)(ModuleBase *`this`,
                               const ServerSpecializeArgs *args);
};

struct ModuleBase {
  ModuleBaseVirtualTable *vtable;
};

// These values are used in Api::setOption(Option)
enum Option : int {
  // Force Magisk's denylist unmount routines to run on this process.
  //
  // Setting this option only makes sense in preAppSpecialize.
  // The actual unmounting happens during app process specialization.
  //
  // Processes added to Magisk's denylist will have all Magisk and its modules'
  // files unmounted
  // from its mount namespace. In addition, all Zygisk code will be unloaded
  // from memory, which
  // also implies that no Zygisk modules (including yours) are loaded.
  //
  // However, if for any reason your module still wants the unmount part of the
  // denylist
  // operation to be enabled EVEN IF THE PROCESS IS NOT ON THE DENYLIST, set
  // this option.
  FORCE_DENYLIST_UNMOUNT = 0,

  // When this option is set, your module's library will be dlclose-ed after
  // post[XXX]Specialize.
  // Be aware that after dlclose-ing your module, all of your code will be
  // unmapped.
  // YOU SHOULD NOT ENABLE THIS OPTION AFTER HOOKING ANY FUNCTION IN THE
  // PROCESS.
  DLCLOSE_MODULE_LIBRARY = 1,
};

// Bit masks of the return value of Api::getFlags()
enum StateFlag : unsigned int {
  // The user has granted root access to the current process
  PROCESS_GRANTED_ROOT = (1u << 0),

  // The current process was added on the denylist
  PROCESS_ON_DENYLIST = (1u << 1),
};

struct ServerSpecializeArgs {
  jint &uid;
  jint &gid;
  jintArray &gids;
  jint &runtime_flags;
  jlong &permitted_capabilities;
  jlong &effective_capabilities;
};

#if 0
// This is old version of zygisk args
struct AppSpecializeArgs {
  // Required arguments. These arguments are guaranteed to exist on all Android
  // versions.
  jint &uid;
  jint &gid;
  jintArray &gids;
  jint &runtime_flags;
  jint &mount_external;
  jstring &se_info;
  jstring &nice_name;
  jstring &instruction_set;
  jstring &app_data_dir;

  // Optional arguments. Please check whether the pointer is null before
  // de-referencing
  jboolean *const is_child_zygote;
  jboolean *const is_top_app;
  jobjectArray *const pkg_data_info_list;
  jobjectArray *const whitelisted_data_info_list;
  jboolean *const mount_data_dirs;
  jboolean *const mount_storage_dirs;
};
#else
// This is actual version of app args, used in ZygiskNext
struct AppSpecializeArgs {
  jint &uid;
  jint &gid;
  jintArray &gids;
  jint &runtime_flags;
  jobjectArray &rlimits;
  jint &mount_external;
  jstring &se_info;
  jstring &nice_name;
  jstring &instruction_set;
  jstring &app_data_dir;

  // Optional arguments. Please check whether the pointer is null before
  // de-referencing
  jintArray *const fds_to_ignore;
  jboolean *const is_child_zygote;
  jboolean *const is_top_app;
  jobjectArray *const pkg_data_info_list;
  jobjectArray *const whitelisted_data_info_list;
  jboolean *const mount_data_dirs;
  jboolean *const mount_storage_dirs;
  jboolean *const mount_sysprop_overrides;
};

#endif

struct module_abi {
  long api_version;
  ModuleBase *_this;

  void (*preAppSpecialize)(ModuleBase *, AppSpecializeArgs *);
  void (*postAppSpecialize)(ModuleBase *, const AppSpecializeArgs *);
  void (*preServerSpecialize)(ModuleBase *, ServerSpecializeArgs *);
  void (*postServerSpecialize)(ModuleBase *, const ServerSpecializeArgs *);
};

#if 0
struct api_table {
  // These first 2 entries are permanent, shall never change
  void *_this;
  bool (*registerModule)(api_table *, module_abi *);

  // Utility functions
  void (*hookJniNativeMethods)(JNIEnv *, const char *, JNINativeMethod *, int);
  void (*pltHookRegister)(const char *, const char *, void *, void **);
  void (*pltHookExclude)(const char *, const char *);
  bool (*pltHookCommit)();

  // Zygisk functions
  int (*connectCompanion)(void * /* _this */);
  void (*setOption)(void * /* _this */, Option);
};
#else
// This is new version used in ZygiskNext
struct api_table {
  // These first 2 entries are permanent, shall never change
  void *_this;
  bool (*registerModule)(api_table *, module_abi *);

  void (*hookJniNativeMethods)(JNIEnv *, const char *, JNINativeMethod *, int);
  void (*pltHookRegister)(dev_t, ino_t, const char *, void *, void **);
  bool (*exemptFd)(int);
  bool (*pltHookCommit)();
  int (*connectCompanion)(void * /* impl */);
  void (*setOption)(void * /* impl */, Option);
  int (*getModuleDir)(void * /* impl */);
  uint32_t (*getFlags)(void * /* impl */);
};
#endif

struct Api {

  // Connect to a root companion process and get a Unix domain socket for IPC.
  //
  // This API only works in the pre[XXX]Specialize functions due to SELinux
  // restrictions.
  //
  // The pre[XXX]Specialize functions run with the same privilege of zygote.
  // If you would like to do some operations with superuser permissions,
  // register a handler function that would be called in the root process with
  // REGISTER_ZYGISK_COMPANION(func). Another good use case for a companion
  // process is that if you want to share some resources across multiple
  // processes, hold the resources in the companion process and pass it over.
  //
  // Returns a file descriptor to a socket that is connected to the socket
  // passed to your module's companion request handler. Returns -1 if the
  // connection attempt failed.
  int connectCompanion();

  // Set various options for your module.
  // Please note that this function accepts one single option at a time.
  // Check zygisk::Option for the full list of options available.
  void setOption(Option opt);

  // Hook JNI native methods for a class
  //
  // Lookup all registered JNI native methods and replace it with your own
  // functions. The original function pointer will be saved in each
  // JNINativeMethod's fnPtr. If no matching class, method name, or signature is
  // found, that specific JNINativeMethod.fnPtr will be set to nullptr.
  void hookJniNativeMethods(JNIEnv *env, const char *className,
                            JNINativeMethod *methods, int numMethods);

  // For ELFs loaded in memory matching `regex`, replace function `symbol` with
  // `newFunc`. If `oldFunc` is not nullptr, the original function pointer will
  // be saved to `oldFunc`.
  void pltHookRegister(const char *regex, const char *symbol, void *newFunc,
                       void **oldFunc);

  // For ELFs loaded in memory matching `regex`, exclude hooks registered for
  // `symbol`. If `symbol` is nullptr, then all symbols will be excluded.
  void pltHookExclude(const char *regex, const char *symbol);

  // Commit all the hooks that was previously registered.
  // Returns false if an error occurred.
  bool pltHookCommit();

private:
  api_table *impl;
  // template <class T> friend void internal::entry_impl(internal::api_table *,
  // JNIEnv *);
};