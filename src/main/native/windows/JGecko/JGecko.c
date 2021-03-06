#include <Windows.h>
#include <zlib.h>

#include <jni.h>

// Why? e.e
#include "../../headers/net_gudenau_jgecko_natives_Windows.h"

#define UNUSED_PARAM(param) { param = param; }

#define malloc(size) ((void*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (SIZE_T) size ))
#define free(pointer) {HeapFree(GetProcessHeap(), 0, (LPVOID) pointer );}

#define SAFE_CLOSE_KEY(key) {if( key ){ RegCloseKey( key ); }}
#define SAFE_FREE(pointer) {if( pointer ){ free( pointer ); }}

/*
* Class:     net_gudenau_jgecko_natives_Windows
* Method:    listSerialDevices
* Signature: ()[Ljava/lang/String;
*/
JNIEXPORT jobjectArray JNICALL Java_net_gudenau_jgecko_natives_Windows_listSerialDevices
(JNIEnv* env, jclass klass) {
	UNUSED_PARAM(klass);

	jclass StringArray = (*env)->FindClass(env, "[Ljava/lang/String;");
	if (!StringArray) {
		return NULL;
	}
	jclass String = (*env)->FindClass(env, "java/lang/String");
	if (!StringArray) {
		return NULL;
	}

	HKEY serialComKey;
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, NULL, 0, KEY_READ, NULL, &serialComKey, NULL) != ERROR_SUCCESS) {
		goto error1;
	}

	DWORD valueCount;
	DWORD maxValueNameLength;
	DWORD maxValueLength;
	if (RegQueryInfoKey(serialComKey, NULL, NULL, NULL, NULL, NULL, NULL, &valueCount, &maxValueNameLength, &maxValueLength, NULL, NULL) != ERROR_SUCCESS) {
		goto error1;
	}

	maxValueNameLength = sizeof(TCHAR) * (maxValueNameLength + 1);
	TCHAR* nameBuffer = malloc(maxValueNameLength);
	if (!nameBuffer) {
		goto error2;
	}
	TCHAR* valueBuffer = malloc(maxValueLength);
	if (!valueBuffer) {
		goto error3;
	}

	jobjectArray resultArray = (*env)->NewObjectArray(env, valueCount, StringArray, NULL);
	if (!resultArray) {
		goto error3;
	}

	DWORD nameBufferSize;
	DWORD valueBufferSize;
	for (DWORD i = 0; i < valueCount; i++) {
		nameBufferSize = maxValueNameLength;
		valueBufferSize = maxValueLength;
		LSTATUS result;
		if ((result = RegEnumValue(serialComKey, i, nameBuffer, &nameBufferSize, NULL, NULL, (void*)valueBuffer, &valueBufferSize)) != ERROR_SUCCESS) {
			goto error3;
		}

		jobjectArray entryArray = (*env)->NewObjectArray(env, 2, String, NULL);
		if (!entryArray) {
			goto error3;
		}

		jstring entryName = (*env)->NewString(env, nameBuffer, nameBufferSize);
		if (!entryName) {
			goto error3;
		}

		jstring entryValue = (*env)->NewString(env, valueBuffer, (valueBufferSize - 1) / sizeof(TCHAR));
		if (!entryValue) {
			goto error3;
		}

		(*env)->SetObjectArrayElement(env, entryArray, 0, entryName);
		(*env)->SetObjectArrayElement(env, entryArray, 1, entryValue);
		(*env)->SetObjectArrayElement(env, resultArray, i, entryArray);
	}

	SAFE_FREE(valueBuffer);
	SAFE_FREE(nameBuffer);
	SAFE_CLOSE_KEY(serialComKey);

	return resultArray;

error3:
	SAFE_FREE(valueBuffer);
error2:
	SAFE_FREE(nameBuffer);
error1:
	SAFE_CLOSE_KEY(serialComKey);

	return NULL;
}

/*
* Class:     net_gudenau_jgecko_natives_Windows
* Method:    doCreateFile
* Signature: (Ljava/lang/String;IIJIIJ)J
*/
JNIEXPORT jlong JNICALL Java_net_gudenau_jgecko_natives_Windows_doCreateFile
	(JNIEnv* env, jclass klass, jstring lpFileName, jint dwDesiredAccess, jint dwShareMode, jlong lpSecurityAttributes, jint dwCreationDisposition, jint dwFlagsAndAttributes, jlong hTemplateFile) {
	UNUSED_PARAM(klass);

	const jchar* lpFileNameValue = (*env)->GetStringChars(env, lpFileName, NULL);
	HANDLE handle = CreateFile((LPCWSTR)lpFileNameValue, (DWORD)dwDesiredAccess, (DWORD)dwShareMode, (LPSECURITY_ATTRIBUTES)((void*)lpSecurityAttributes), (DWORD)dwCreationDisposition, (DWORD)dwFlagsAndAttributes, (HANDLE)(void*)hTemplateFile);
	(*env)->ReleaseStringChars(env, lpFileName, lpFileNameValue);
	return (jlong)handle;
}

/*
* Class:     net_gudenau_jgecko_natives_Windows
* Method:    doCloseHandle
* Signature: (J)Z
*/
JNIEXPORT jboolean JNICALL Java_net_gudenau_jgecko_natives_Windows_doCloseHandle
	(JNIEnv* env, jclass klass, jlong handle) {
	UNUSED_PARAM(env);
	UNUSED_PARAM(klass);
	return CloseHandle((HANDLE)handle) != 0;
}

/*
* Class:     net_gudenau_jgecko_natives_Windows
* Method:    doGetRealFD
* Signature: (Ljava/io/FileDescriptor;)J
*/
JNIEXPORT jlong JNICALL Java_net_gudenau_jgecko_natives_Windows_doGetRealFD
	(JNIEnv* env, jclass klass, jobject descriptor) {
	UNUSED_PARAM(klass);

	jclass FileDescriptor = (*env)->FindClass(
		env,
		"java/io/FileDescriptor"
	);
	if (!FileDescriptor) {
		return 0;
	}

	jfieldID fd = (*env)->GetFieldID(
		env,
		FileDescriptor,
		"handle",
		"J"
	);
	if (!fd) {
		return 0;
	}

	return (*env)->GetLongField(
		env,
		descriptor,
		fd
	);
}

/*
* Class:     net_gudenau_jgecko_natives_Windows
* Method:    getBufferAddress
* Signature: (Ljava/nio/ByteBuffer;)J
*/
JNIEXPORT jlong JNICALL Java_net_gudenau_jgecko_natives_Windows_getBufferAddress
	(JNIEnv* env, jclass klass, jobject buffer) {
	UNUSED_PARAM(klass);
	
	return (jlong)((*env)->GetDirectBufferAddress(env, buffer));
}

/*
* Class:     net_gudenau_jgecko_natives_Windows
* Method:    getCOMMTIMEOUTSSize
* Signature: ()I
*/
JNIEXPORT jint JNICALL Java_net_gudenau_jgecko_natives_Windows_getCOMMTIMEOUTSSize
	(JNIEnv* env, jclass klass) {
	UNUSED_PARAM(env);
	UNUSED_PARAM(klass);

	return (jint)(sizeof(COMMTIMEOUTS));
}

/*
* Class:     net_gudenau_jgecko_natives_Windows
* Method:    GetCommTimeouts
* Signature: (JJ)Z
*/
JNIEXPORT jboolean JNICALL Java_net_gudenau_jgecko_natives_Windows_GetCommTimeouts
(JNIEnv* env, jclass klass, jlong handle, jlong buffer) {
	UNUSED_PARAM(env);
	UNUSED_PARAM(klass);

	return GetCommTimeouts((HANDLE)handle, (COMMTIMEOUTS*)((void*)buffer)) != 0;
}


/*
* Class:     net_gudenau_jgecko_natives_Windows
* Method:    SetCommTimeouts
* Signature: (JJ)Z
*/
JNIEXPORT jboolean JNICALL Java_net_gudenau_jgecko_natives_Windows_SetCommTimeouts
(JNIEnv* env, jclass klass, jlong handle, jlong buffer) {
	UNUSED_PARAM(env);
	UNUSED_PARAM(klass);

	return SetCommTimeouts((HANDLE)handle, (COMMTIMEOUTS*)((void*)buffer)) != 0;
}


/*
* Class:     net_gudenau_jgecko_natives_Windows
* Method:    SetCommMask
* Signature: (JI)Z
*/
JNIEXPORT jboolean JNICALL Java_net_gudenau_jgecko_natives_Windows_SetCommMask
(JNIEnv* env, jclass klass, jlong handle, jint mask) {
	UNUSED_PARAM(env);
	UNUSED_PARAM(klass);

	return SetCommMask((HANDLE)handle, (DWORD)mask) != 0;
}


/*
* Class:     net_gudenau_jgecko_natives_Windows
* Method:    PurgeComm
* Signature: (JI)Z
*/
JNIEXPORT jboolean JNICALL Java_net_gudenau_jgecko_natives_Windows_PurgeComm
(JNIEnv* env, jclass klass, jlong handle, jint flags) {
	UNUSED_PARAM(env);
	UNUSED_PARAM(klass);

	return PurgeComm((HANDLE)handle, (DWORD)flags) != 0;
}

/*
* Class:     net_gudenau_jgecko_natives_Windows
* Method:    GetLastError
* Signature: ()I
*/
JNIEXPORT jint JNICALL Java_net_gudenau_jgecko_natives_Windows_GetLastError
	(JNIEnv* env, jclass klass) {
	UNUSED_PARAM(env);
	UNUSED_PARAM(klass);

	return (jint)GetLastError();
}

/*
* Class:     net_gudenau_jgecko_natives_Windows
* Method:    ReadFile
* Signature: (JJI[IJ)Z
*/
JNIEXPORT jboolean JNICALL Java_net_gudenau_jgecko_natives_Windows_ReadFile
	(JNIEnv* env, jclass klass, jlong handle, jlong buffer, jint toRead, jintArray read, jlong overlapped) {
	UNUSED_PARAM(klass);

	DWORD readBuffer;
	jboolean result = ReadFile((HANDLE)handle, (void*)buffer, toRead, &readBuffer, (LPOVERLAPPED)overlapped) != 0;
	(*env)->SetIntArrayRegion(env, read, 0, 1, (const jint*)&readBuffer);
	return result;
}

/*
* Class:     net_gudenau_jgecko_natives_Windows
* Method:    WriteFile
* Signature: (JJI[IJ)Z
*/
JNIEXPORT jboolean JNICALL Java_net_gudenau_jgecko_natives_Windows_WriteFile
	(JNIEnv* env, jclass klass, jlong handle, jlong buffer, jint toWrite, jintArray writen, jlong overlapped) {
	UNUSED_PARAM(env);
	UNUSED_PARAM(klass);
	
	DWORD writeBuffer;
	jboolean result = WriteFile((HANDLE)handle, (void*)buffer, toWrite, &writeBuffer, (LPOVERLAPPED)overlapped) != 0;
	(*env)->SetIntArrayRegion(env, writen, 0, 1, (const jint*)&writeBuffer);
	return result;
}

/*
* Class:     net_gudenau_jgecko_natives_Windows
* Method:    compress2
* Signature: (JJJII)I
*/
JNIEXPORT jint JNICALL Java_net_gudenau_jgecko_natives_Windows_compress2
	(JNIEnv* env, jclass klass, jlong dest, jlong destSize, jlong source, jint sourceSize, jint level) {
	UNUSED_PARAM(env);
	UNUSED_PARAM(klass);

	return (jint)compress2((void*)dest, (uLongf*)destSize, (void*)source, (uLongf)sourceSize, (int)level);
}
