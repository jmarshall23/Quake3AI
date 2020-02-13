// func_hooks.cpp
//

extern "C" {
#include "../game/q_shared.h"
};

extern "C" int FS_Write(const void* buffer, int len, fileHandle_t f) {
	return 0;
}

extern "C" int FS_ReadFile(const char* qpath, void** buffer) {
	return 0;
}

extern "C" void FS_FreeFile(void* buffer) {

}

extern "C" int FS_FOpenFileRead(const char* qpath, fileHandle_t* file, qboolean uniqueFILE) {
	return 0;
}

extern "C" fileHandle_t	FS_FOpenFileWrite(const char* qpath) {
	return 0;
}

extern "C" void	FS_FCloseFile(fileHandle_t f) {

}

extern "C" void 		QDECL Com_Printf(const char* fmt, ...) {

}

extern "C" void 		QDECL Com_DPrintf(const char* fmt, ...) {

}
extern "C" void 		QDECL Com_Error(int code, const char* fmt, ...) {

}

extern "C" void* Com_Allocate(int bytes) {
	return NULL;
}

extern "C" void Com_Dealloc(void* ptr) {

}