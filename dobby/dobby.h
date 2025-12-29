// Taken from:
// https://github.com/jmpews/Dobby/blob/5dfc8546/include/dobby.h

// namespace dobby {

typedef __SIZE_TYPE__ __darwin_size_t;
typedef unsigned long uintptr_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef unsigned char uint8_t;
typedef __darwin_size_t size_t;

typedef uintptr_t addr_t;
typedef uint32_t addr32_t;
typedef uint64_t addr64_t;

typedef void *asm_func_t;

struct DobbyRegisterContext;

// memory code patch
int DobbyCodePatch(void *address, uint8_t *buffer, uint32_t buffer_size);

// function inline hook
int DobbyHook(void *address, void *fake_func, void **out_origin_func);

// dynamic binary instruction instrument
// for Arm64, can't access q8 - q31, unless enable full floating-point register
// pack
typedef void (*dobby_instrument_callback_t)(void *address,
                                            DobbyRegisterContext *ctx);
int DobbyInstrument(void *address, dobby_instrument_callback_t pre_handler);

// destroy and restore code patch
int DobbyDestroy(void *address);

const char *DobbyGetVersion();

// symbol resolver
void *DobbySymbolResolver(const char *image_name, const char *symbol_name);

// import table replace
int DobbyImportTableReplace(char *image_name, char *symbol_name,
                            void *fake_func, void **orig_func);

// for arm, Arm64, try use b xxx instead of ldr absolute indirect branch
// for x86, x64, always use absolute indirect jump
void dobby_set_near_trampoline(bool enable);

// register callback for alloc near code block
typedef addr_t (*dobby_alloc_near_code_callback_t)(uint32_t size, addr_t pos,
                                                   size_t range);
void dobby_register_alloc_near_code_callback(
    dobby_alloc_near_code_callback_t handler);

void dobby_set_options(
    bool enable_near_trampoline,
    dobby_alloc_near_code_callback_t alloc_near_code_callback);

// }; // namespace dobby