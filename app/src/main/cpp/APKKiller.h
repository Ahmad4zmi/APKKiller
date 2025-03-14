#include <stdio.h>
#include <iostream>
#include <vector>
#include <jni.h>
#include <thread>
#include <future>
#include <unordered_map>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dlfcn.h>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>

#include <whale.h>

#include "ElfImg.h"
#include "Utils.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "APKKiller", __VA_ARGS__)

#define apk_asset_path "original.apk" // assets/original.apk
#define apk_fake_name "original.apk" // /data/data/<package_name>/cache/original.apk
std::vector<std::vector<uint8_t>> apk_signatures {{0x30,0x82,0x05,0x87,0x30,0x82,0x03,0x6F,0xA0,0x03,0x02,0x01,0x02,0x02,0x15,0x00,0x95,0x72,0xCF,0xB6,0x40,0x7A,0x6A,0xAC,0x6A,0x05,0xD3,0xA2,0x48,0xE0,0xE6,0x28,0x36,0x2E,0x2E,0x58,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x0B,0x05,0x00,0x30,0x74,0x31,0x0B,0x30,0x09,0x06,0x03,0x55,0x04,0x06,0x13,0x02,0x55,0x53,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x08,0x13,0x0A,0x43,0x61,0x6C,0x69,0x66,0x6F,0x72,0x6E,0x69,0x61,0x31,0x16,0x30,0x14,0x06,0x03,0x55,0x04,0x07,0x13,0x0D,0x4D,0x6F,0x75,0x6E,0x74,0x61,0x69,0x6E,0x20,0x56,0x69,0x65,0x77,0x31,0x14,0x30,0x12,0x06,0x03,0x55,0x04,0x0A,0x13,0x0B,0x47,0x6F,0x6F,0x67,0x6C,0x65,0x20,0x49,0x6E,0x63,0x2E,0x31,0x10,0x30,0x0E,0x06,0x03,0x55,0x04,0x0B,0x13,0x07,0x41,0x6E,0x64,0x72,0x6F,0x69,0x64,0x31,0x10,0x30,0x0E,0x06,0x03,0x55,0x04,0x03,0x13,0x07,0x41,0x6E,0x64,0x72,0x6F,0x69,0x64,0x30,0x1E,0x17,0x0D,0x31,0x39,0x31,0x31,0x30,0x34,0x30,0x30,0x31,0x32,0x34,0x32,0x5A,0x17,0x0D,0x34,0x39,0x31,0x31,0x30,0x34,0x30,0x30,0x31,0x32,0x34,0x32,0x5A,0x30,0x74,0x31,0x0B,0x30,0x09,0x06,0x03,0x55,0x04,0x06,0x13,0x02,0x55,0x53,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x08,0x13,0x0A,0x43,0x61,0x6C,0x69,0x66,0x6F,0x72,0x6E,0x69,0x61,0x31,0x16,0x30,0x14,0x06,0x03,0x55,0x04,0x07,0x13,0x0D,0x4D,0x6F,0x75,0x6E,0x74,0x61,0x69,0x6E,0x20,0x56,0x69,0x65,0x77,0x31,0x14,0x30,0x12,0x06,0x03,0x55,0x04,0x0A,0x13,0x0B,0x47,0x6F,0x6F,0x67,0x6C,0x65,0x20,0x49,0x6E,0x63,0x2E,0x31,0x10,0x30,0x0E,0x06,0x03,0x55,0x04,0x0B,0x13,0x07,0x41,0x6E,0x64,0x72,0x6F,0x69,0x64,0x31,0x10,0x30,0x0E,0x06,0x03,0x55,0x04,0x03,0x13,0x07,0x41,0x6E,0x64,0x72,0x6F,0x69,0x64,0x30,0x82,0x02,0x22,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x01,0x05,0x00,0x03,0x82,0x02,0x0F,0x00,0x30,0x82,0x02,0x0A,0x02,0x82,0x02,0x01,0x00,0xC4,0xAB,0x27,0xCF,0x7B,0x8E,0x5D,0x00,0x81,0x0B,0xAF,0xB8,0x79,0x05,0x4F,0x6A,0xC1,0x1B,0x67,0xA8,0xB3,0x28,0xCB,0xDF,0xDA,0x50,0x4C,0x84,0x93,0x91,0x23,0x0F,0xD4,0x78,0xA2,0x1A,0xE2,0xA2,0xDC,0xF2,0xFF,0x3E,0xFA,0x1E,0x79,0xBA,0x0B,0xE2,0x77,0xF2,0xC1,0xA0,0xB9,0x27,0x99,0x70,0xA3,0x26,0x9C,0xBF,0xCD,0x01,0xEB,0xB9,0x53,0xC1,0x33,0xA1,0x05,0xB6,0x19,0xDE,0x34,0xE3,0x61,0xE1,0xD4,0xE6,0x6E,0x2A,0xC1,0x7E,0x13,0x71,0xF5,0x59,0x65,0x69,0x50,0x84,0x06,0xC8,0xD2,0x8B,0x10,0xDC,0x8E,0xAB,0xF5,0xB5,0x9C,0x03,0x35,0x99,0x06,0x69,0x68,0xBF,0xE9,0x7C,0xD6,0x5A,0x91,0x13,0x62,0xCA,0x50,0xD1,0x0E,0x45,0x10,0x0B,0xDA,0x2A,0x89,0xF7,0x27,0xFE,0x55,0x85,0x4C,0x22,0x44,0xA6,0xCD,0x17,0x7E,0x9C,0x45,0x8E,0x71,0xE1,0x05,0x5B,0x60,0x57,0x3A,0x24,0x4E,0x2E,0xB9,0x3D,0x25,0xD2,0x43,0x55,0xC8,0xE4,0xC1,0xDB,0x26,0x77,0xE9,0x03,0x13,0xC8,0x71,0xE7,0x94,0x9F,0x1B,0x1F,0xF9,0x09,0x72,0x85,0x8C,0x2D,0x9C,0x5D,0x47,0x79,0x9D,0xE7,0x61,0xE6,0x70,0xDA,0xEB,0x1C,0xBC,0xD5,0xBA,0xE1,0x98,0xA9,0xE1,0x70,0x2B,0xBC,0xDE,0x27,0x1C,0x49,0xB8,0x2F,0x6E,0xFA,0xC7,0xDB,0xFE,0x2B,0x90,0xC2,0xCB,0x05,0xCB,0x34,0xB3,0xE8,0x8B,0x1D,0x58,0x3D,0x29,0xDB,0x5B,0xDB,0xC6,0x3F,0xC2,0x78,0x31,0x5D,0xBC,0xE5,0x93,0x47,0xE1,0x86,0xCE,0x49,0x25,0xEF,0x56,0xF9,0x1C,0xBA,0xE5,0x64,0xBF,0xFB,0x3E,0x31,0x1E,0x8E,0x6F,0xF1,0xEC,0xFE,0xAF,0xA1,0xE4,0x61,0xCD,0x4F,0x39,0x06,0xA2,0x65,0x4E,0x07,0xEA,0x06,0x79,0x62,0x3D,0x9F,0x58,0x52,0x6A,0x75,0x42,0x5A,0x72,0xB3,0x8C,0xC2,0xF6,0xDD,0x60,0x51,0x95,0xD2,0xAA,0xB1,0x1B,0x80,0xC1,0x84,0xDD,0xB6,0x8A,0x1F,0xAB,0x55,0x1B,0xF4,0x9C,0x42,0x1A,0x37,0x7F,0x73,0xA3,0x67,0xE3,0xE1,0xD4,0x77,0x1A,0xC2,0x81,0xE9,0xBA,0x7A,0xF9,0x96,0xCB,0xB4,0x06,0x39,0x0B,0xDF,0x52,0x2F,0x26,0x44,0x54,0xA0,0xB3,0x4D,0xD9,0x6F,0xC2,0xD8,0x68,0x66,0xD1,0xB9,0xDB,0x93,0xD5,0xE2,0xF7,0xE1,0x82,0xEF,0x22,0xF0,0x85,0x76,0x9B,0x2E,0x44,0xCE,0x76,0x77,0x52,0x9A,0xA8,0x93,0xE1,0xA0,0xBB,0x1C,0x1E,0x1A,0xF9,0x8B,0xC3,0x57,0xD9,0x7C,0x08,0x29,0x5F,0x7D,0xDC,0x5A,0x01,0xAB,0xB2,0x7E,0x8A,0x5F,0x13,0xC6,0xAE,0x90,0x0D,0x62,0xD6,0x9C,0x0B,0x35,0xD4,0x69,0x1D,0xEB,0x0C,0x85,0x32,0x30,0xF2,0x82,0xF1,0xEC,0x79,0x8C,0x20,0x29,0xA4,0xE5,0x87,0x2D,0xE6,0xEE,0x7F,0x77,0x87,0xFA,0xB2,0x13,0x49,0x69,0x70,0x6E,0xD9,0x57,0x91,0xF3,0x73,0xCD,0x0B,0xB8,0xD3,0xB9,0x77,0xE7,0xA7,0xB0,0x92,0x7A,0x91,0xE2,0x3A,0x28,0x17,0x3D,0xCF,0xEE,0x46,0x79,0x5C,0x24,0xE5,0xDC,0xA9,0x17,0x16,0x02,0xCC,0x16,0xE8,0x5E,0xF5,0xA8,0xA3,0x6B,0x1B,0x16,0x9E,0xF4,0x4C,0x23,0x52,0x13,0xDE,0xDB,0x91,0xAB,0xC0,0xBE,0x45,0xF0,0x21,0xBB,0x05,0xF8,0xF2,0xE0,0x7C,0xAE,0x95,0x24,0x7B,0xD7,0xBD,0xD7,0x57,0x5F,0x02,0x03,0x01,0x00,0x01,0xA3,0x10,0x30,0x0E,0x30,0x0C,0x06,0x03,0x55,0x1D,0x13,0x04,0x05,0x30,0x03,0x01,0x01,0xFF,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x0B,0x05,0x00,0x03,0x82,0x02,0x01,0x00,0xB0,0x12,0x07,0x6D,0x1A,0x43,0x2D,0xD2,0xD5,0xD4,0x83,0x58,0xB9,0xA7,0x39,0x97,0x2A,0x2C,0xE7,0x2C,0xD9,0xAB,0xBB,0x6E,0xD4,0x62,0x3A,0xAC,0xF6,0xC7,0xDF,0xD0,0xE8,0x03,0x3A,0xB3,0x1F,0xED,0xF5,0xA9,0xA5,0x04,0xA7,0xB2,0x20,0x19,0xE0,0x5D,0x5F,0xAA,0xE4,0x58,0xD2,0xFD,0x53,0xA0,0x54,0xEF,0xC3,0x45,0x14,0x0F,0xBF,0x70,0x69,0xB5,0x8C,0x93,0x5F,0xE7,0x81,0xE3,0x89,0x88,0x25,0x63,0xD2,0x78,0x07,0x55,0x65,0x19,0x6A,0xBD,0x8C,0xBB,0xC2,0xB0,0xD7,0xCF,0x39,0x7B,0x8A,0x7D,0xDE,0x29,0x37,0xE9,0xDD,0x9C,0x10,0xED,0x7B,0xC9,0x56,0xBA,0x6F,0x8D,0x46,0xF0,0x45,0x1E,0x81,0x8A,0x23,0x67,0xB9,0xAC,0x90,0x62,0x29,0x93,0x77,0xB8,0x92,0xA7,0x70,0x73,0xC7,0xFE,0x66,0xF8,0x2E,0xF3,0x93,0xDA,0xC2,0x1D,0x49,0xE4,0x4F,0xE5,0x4F,0x7C,0x69,0x74,0x60,0xCD,0xED,0x1B,0x3C,0x16,0xFA,0x5D,0xC0,0x1A,0x82,0xB5,0x64,0xE1,0xC5,0xAD,0x95,0x21,0x87,0x9F,0x3F,0xE0,0xB1,0xA6,0xF1,0xCF,0xCF,0x95,0xC6,0x56,0x78,0xF0,0x54,0x40,0x89,0x0D,0xA0,0x75,0x42,0x7F,0xDB,0x0F,0xA0,0x6A,0x5E,0x6D,0x57,0x66,0x48,0xF4,0x83,0xCD,0xFF,0x3D,0x20,0xD3,0x30,0x50,0x49,0xC8,0xCF,0x9E,0x18,0x3B,0xA2,0x10,0xE4,0x3E,0xCC,0x89,0x68,0x3F,0x43,0x18,0xD0,0x59,0x9E,0x23,0x4F,0x87,0xA8,0xF6,0x00,0x1C,0x7A,0x27,0x71,0xA3,0x78,0xEE,0x75,0xEB,0x66,0x8B,0x5B,0xFD,0xFE,0x91,0x66,0xEE,0x75,0xCF,0x71,0x22,0xC6,0xD2,0xAF,0xE5,0x04,0x4F,0xBC,0x81,0x80,0xD0,0xC6,0xE6,0xC5,0x19,0x5E,0xFB,0x60,0xA4,0x4B,0x5C,0xB7,0xD0,0x32,0xAD,0x30,0x15,0xE5,0x06,0xE7,0xB7,0x85,0x99,0x45,0x95,0x73,0x0E,0x0D,0x38,0xFB,0x65,0xD6,0x64,0x2A,0x8D,0x05,0x15,0xEC,0x13,0xAC,0xC6,0x93,0x6B,0x00,0xB1,0x06,0xDD,0xC2,0x4A,0x52,0xB9,0xA1,0x2B,0x99,0x6B,0x2B,0xC0,0x3B,0x99,0xE6,0xE7,0xD5,0x34,0x41,0x4B,0x0F,0x56,0xF8,0x1D,0xE0,0x0C,0x28,0x00,0x01,0x17,0xB2,0x90,0x40,0xD3,0x70,0x13,0xB1,0x39,0x42,0x23,0x31,0x77,0x27,0xFC,0x8B,0xB6,0x18,0x20,0x40,0xFC,0xA1,0x5D,0xF8,0x10,0xB7,0xAA,0xEF,0x28,0x62,0x16,0xA9,0x3B,0x7A,0x05,0x41,0x24,0x1C,0xE7,0x95,0x49,0xB2,0xFF,0x48,0x97,0x44,0xFE,0x6A,0x0E,0xB2,0x56,0x06,0x32,0x2E,0xBB,0x6C,0x80,0xCB,0x32,0x03,0xA0,0x89,0x10,0xF5,0x7D,0xD2,0xD4,0x73,0xC1,0xE3,0x38,0x14,0x9F,0x70,0x68,0x3E,0x4A,0x83,0x77,0x6D,0x23,0xB4,0xF0,0xC7,0xDF,0xFE,0xD1,0xAC,0x2B,0x51,0x1E,0x91,0x4F,0x6A,0x7B,0x2E,0xE5,0xF9,0xCD,0x73,0xBF,0xAE,0x98,0xF0,0x7D,0x18,0x07,0xFF,0x6C,0xBE,0x49,0x85,0x2F,0x96,0x7B,0x21,0x88,0x59,0x79,0x75,0x91,0xDB,0x68,0xFE,0x5F,0x70,0x8B,0x58,0xED,0x3F,0x56,0x8F,0x22,0x47,0xA0,0x19,0x9F,0x1A,0xA6,0xD5,0xE4,0x76,0x6E,0x6F,0xBD,0xE7,0x80,0xFA,0xCB,0x4C,0x15,0x40,0xC4,0xE4,0x76,0x28,0x6C,0xC0,0x0C,0xBF,0x1C,0xA0,0x4C,0xB4,0xDC,0x3C,0x7E,0x5A,0x98,0xED,0xA3,0xA3,0x00,0x58,0x27,0x1C,0x87,0x85,0x18}};



JavaVM *g_vm;
JNIEnv *g_env;
std::string g_pkgName;
jstring g_apkPath;
jobject g_proxy, g_pkgMgr;

const char *getProcessName() {
    FILE *f = fopen("/proc/self/cmdline", "rb");
    if (f) {
        char *buf = new char[64];
        fread(buf, sizeof(char), 64, f);
        fclose(f);
        return buf;
    }
    return 0;
}

std::string getPackageName(jobject obj) {
    auto contextClass = g_env->FindClass("android/content/Context");
    auto getPackageNameMethod = g_env->GetMethodID(contextClass, "getPackageName", "()Ljava/lang/String;");
    return g_env->GetStringUTFChars((jstring) g_env->CallObjectMethod(obj, getPackageNameMethod), 0);
}

jobject getApplicationContext(jobject obj) {
    auto contextClass = g_env->FindClass("android/content/Context");
    auto getApplicationContextMethod = g_env->GetMethodID(contextClass, "getApplicationContext", "()Landroid/content/Context;");
    return g_env->CallObjectMethod(obj, getApplicationContextMethod);
}

jobject getPackageManager(jobject obj) {
    auto contextClass = g_env->FindClass("android/content/Context");
    auto getPackageManagerMethod = g_env->GetMethodID(contextClass, "getPackageManager", "()Landroid/content/pm/PackageManager;");
    return g_env->CallObjectMethod(obj, getPackageManagerMethod);
}

class Reference {
public:
    JNIEnv *env;
    jobject reference;
public:
    Reference(JNIEnv *env, jobject obj) {
        this->env = env;
        this->reference = env->NewGlobalRef(obj);
    }

    Reference(jobject reference) {
        this->env = g_env;
        this->reference = reference;
    }

    jobject get() {
        auto referenceClass = env->FindClass("java/lang/ref/Reference");
        auto get = env->GetMethodID(referenceClass, "get", "()Ljava/lang/Object;");
        return env->CallObjectMethod(reference, get);
    }
};

class WeakReference : public Reference {
public:
    WeakReference(JNIEnv *env, jobject weakReference) : Reference(env, weakReference) {
    }

    WeakReference(jobject weakReference) : Reference(weakReference) {
    }

    static jobject Create(jobject obj) {
        auto weakReferenceClass = g_env->FindClass("java/lang/ref/WeakReference");
        auto weakReferenceClassConstructor = g_env->GetMethodID(weakReferenceClass, "<init>", "(Ljava/lang/Object;)V");
        return g_env->NewObject(weakReferenceClass, weakReferenceClassConstructor, obj);
    }
};

class ArrayList {
private:
    JNIEnv *env;
    jobject arrayList;
public:
    ArrayList(JNIEnv *env, jobject arrayList) {
        this->env = env;
        this->arrayList = arrayList;
    }

    ArrayList(jobject arrayList) {
        this->env = g_env;
        this->arrayList = arrayList;
    }

    jobject getObj() {
        return arrayList;
    }

    jobject get(int index) {
        auto arrayListClass = env->FindClass("java/util/ArrayList");
        auto getMethod = env->GetMethodID(arrayListClass, "get", "(I)Ljava/lang/Object;");
        return env->CallObjectMethod(arrayList, getMethod, index);
    }

    void set(int index, jobject value) {
        auto arrayListClass = env->FindClass("java/util/ArrayList");
        auto setMethod = env->GetMethodID(arrayListClass, "set", "(ILjava/lang/Object;)Ljava/lang/Object;");
        env->CallObjectMethod(arrayList, setMethod, index, value);
    }

    int size() {
        auto arrayListClass = env->FindClass("java/util/ArrayList");
        auto sizeMethod = env->GetMethodID(arrayListClass, "size", "()I");
        return env->CallIntMethod(arrayList, sizeMethod);
    }
};

class ArrayMap {
private:
    JNIEnv *env;
    jobject arrayMap;
public:
    ArrayMap(JNIEnv *env, jobject arrayMap) {
        this->env = env;
        this->arrayMap = arrayMap;
    }

    ArrayMap(jobject arrayMap) {
        this->env = g_env;
        this->arrayMap = arrayMap;
    }

    jobject getObj() {
        return arrayMap;
    }

    jobject valueAt(int index) {
        auto arrayMapClass = env->FindClass("android/util/ArrayMap");
        auto valueAtMethod = env->GetMethodID(arrayMapClass, "valueAt", "(I)Ljava/lang/Object;");
        return env->CallObjectMethod(arrayMap, valueAtMethod, index);
    }

    jobject setValueAt(int index, jobject value) {
        auto arrayMapClass = env->FindClass("android/util/ArrayMap");
        auto setValueAtMethod = env->GetMethodID(arrayMapClass, "setValueAt", "(ILjava/lang/Object;)Ljava/lang/Object;");
        return env->CallObjectMethod(arrayMap, setValueAtMethod, index, value);
    }

    int size() {
        auto arrayMapClass = env->FindClass("android/util/ArrayMap");
        auto sizeMethod = env->GetMethodID(arrayMapClass, "size", "()I");
        return env->CallIntMethod(arrayMap, sizeMethod);
    }
};

class Method {
private:
    JNIEnv *env;
    jobject method;
    jmethodID getNameMethod;
    jmethodID invokeMethod;

    void initMethod(jobject method) {
        this->method = method;

        jclass methodClass = env->FindClass("java/lang/reflect/Method");
        getNameMethod = env->GetMethodID(methodClass, "getName", "()Ljava/lang/String;");
        invokeMethod = env->GetMethodID(methodClass, "invoke", "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");

        auto setAccessibleMethod = env->GetMethodID(methodClass, "setAccessible", "(Z)V");
        env->CallVoidMethod(method, setAccessibleMethod, true);
    }

public:
    Method(JNIEnv *env, jobject method) {
        this->env = env;
        initMethod(method);
    }

    Method(jobject method) {
        this->env = g_env;
        initMethod(method);
    }

    ~Method() {
        env->DeleteLocalRef(method);
    }

    const char *getName() {
        return env->GetStringUTFChars((jstring) env->CallObjectMethod(method, getNameMethod), 0);
    }

    jobject invoke(jobject object, jobjectArray args = 0) {
        return env->CallObjectMethod(method, invokeMethod, object, args);
    }
};

class Field {
private:
    JNIEnv *env;
    jobject field;
    jmethodID getMethod;
    jmethodID setMethod;
public:
    Field(JNIEnv *env, jobject field) {
        this->env = env;
        this->field = field;

        jclass fieldClass = env->FindClass("java/lang/reflect/Field");
        getMethod = env->GetMethodID(fieldClass, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");
        setMethod = env->GetMethodID(fieldClass, "set", "(Ljava/lang/Object;Ljava/lang/Object;)V");

        auto setAccessibleMethod = env->GetMethodID(fieldClass, "setAccessible", "(Z)V");
        env->CallVoidMethod(field, setAccessibleMethod, true);
    }

    ~Field() {
        env->DeleteGlobalRef(field);
    }

    jobject getField() {
        return field;
    }

    jobject get(jobject obj) {
        return env->CallObjectMethod(field, getMethod, obj);
    }

    void set(jobject obj, jobject value) {
        env->CallVoidMethod(field, setMethod, obj, value);
    }
};

class Class {
private:
    JNIEnv *env;
    jobject clazz;

    void initClass(const char *className) {
        auto classClass = env->FindClass("java/lang/Class");
        auto forNameMethod = env->GetStaticMethodID(classClass, "forName", "(Ljava/lang/String;)Ljava/lang/Class;");
        clazz = env->NewGlobalRef(env->CallStaticObjectMethod(classClass, forNameMethod, env->NewStringUTF(className)));
    }

public:
    Class(JNIEnv *env, const char *className) {
        this->env = env;
        initClass(className);
    }

    Class(const char *className) {
        this->env = g_env;
        initClass(className);
    }

    ~Class() {
        env->DeleteGlobalRef(clazz);
    }

    jobject getClass() {
        return clazz;
    }

    Field getField(const char *fieldName) {
        auto future = std::async([&]() -> jobject {
            JNIEnv *env;
            g_vm->AttachCurrentThread(&env, NULL);

            auto classClass = env->FindClass("java/lang/Class");
            jmethodID getDeclaredFieldMethod = env->GetMethodID(classClass, "getDeclaredField", "(Ljava/lang/String;)Ljava/lang/reflect/Field;");

            auto fieldNameObj = env->NewStringUTF(fieldName);
            auto result = env->CallObjectMethod(clazz, getDeclaredFieldMethod, fieldNameObj);
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                env->ExceptionClear();
            }

            if (result) {
                result = env->NewGlobalRef(result);
            }

            env->DeleteLocalRef(fieldNameObj);
            env->DeleteLocalRef(classClass);

            g_vm->DetachCurrentThread();
            return result;
        });
        return Field(env, future.get());
    }
};

void patch_ApplicationInfo(jobject obj) {
    if (obj) {
        LOGI("-------- Patching ApplicationInfo - %p", obj);
        Class applicationInfoClass("android.content.pm.ApplicationInfo");

        auto sourceDirField = applicationInfoClass.getField("sourceDir");
        auto publicSourceDirField = applicationInfoClass.getField("publicSourceDir");

        sourceDirField.set(obj, g_apkPath);
        publicSourceDirField.set(obj, g_apkPath);
    }
}

void patch_LoadedApk(jobject obj) {
    if (obj) {
        LOGI("-------- Patching LoadedApk - %p", obj);
        Class loadedApkClass("android.app.LoadedApk");

        auto mApplicationInfoField = loadedApkClass.getField("mApplicationInfo");
        patch_ApplicationInfo(mApplicationInfoField.get(obj));

        auto mAppDirField = loadedApkClass.getField("mAppDir");
        auto mResDirField = loadedApkClass.getField("mResDir");

        mAppDirField.set(obj, g_apkPath);
        mResDirField.set(obj, g_apkPath);
    }
}

void patch_AppBindData(jobject obj) {
    if (obj) {
        LOGI("-------- Patching AppBindData - %p", obj);
        Class appBindDataClass("android.app.ActivityThread$AppBindData");

        auto infoField = appBindDataClass.getField("info");
        patch_LoadedApk(infoField.get(obj));

        auto appInfoField = appBindDataClass.getField("appInfo");
        patch_ApplicationInfo(appInfoField.get(obj));
    }
}

void patch_ContextImpl(jobject obj) {
    if (obj) {
        LOGI("-------- Patching ContextImpl - %p", obj);
        Class contextImplClass("android.app.ContextImpl");

        auto mPackageInfoField = contextImplClass.getField("mPackageInfo");
        patch_LoadedApk(mPackageInfoField.get(obj));

        auto mPackageManagerField = contextImplClass.getField("mPackageManager");
        mPackageManagerField.set(obj, g_proxy);
    }
}

void patch_Application(jobject obj) {
    if (obj) {
        LOGI("-------- Patching Application - %p", obj);
        Class applicationClass("android.app.Application");

        auto mLoadedApkField = applicationClass.getField("mLoadedApk");
        patch_LoadedApk(mLoadedApkField.get(obj));

        // patch_ContextImpl(getApplicationContext(obj));
    }
}

AAssetManager *g_assetManager;

void extractAsset(std::string assetName, std::string extractPath) {
    LOGI("-------- Extracting %s to %s", assetName.c_str(), extractPath.c_str());
    AAssetManager *assetManager = g_assetManager;
    AAsset *asset = AAssetManager_open(assetManager, assetName.c_str(), AASSET_MODE_UNKNOWN);
    if (!asset) {
        return;
    }

    int fd = open(extractPath.c_str(), O_CREAT | O_WRONLY, 0644);
    if (fd < 0) {
        AAsset_close(asset);
        return;
    }

    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    int bytesRead;
    while ((bytesRead = AAsset_read(asset, buffer, BUFFER_SIZE)) > 0) {
        int bytesWritten = write(fd, buffer, bytesRead);
        if (bytesWritten != bytesRead) {
            AAsset_close(asset);
            close(fd);
            return;
        }
    }

    AAsset_close(asset);
    close(fd);
}

void patch_PackageManager(jobject obj) {
    if (!obj) return;

    Class activityThreadClass("android.app.ActivityThread");
    auto sCurrentActivityThreadField = activityThreadClass.getField("sCurrentActivityThread");
    auto sCurrentActivityThread = sCurrentActivityThreadField.get(NULL);

    auto sPackageManagerField = activityThreadClass.getField("sPackageManager");
    g_pkgMgr = g_env->NewGlobalRef(sPackageManagerField.get(NULL));

    Class iPackageManagerClass("android.content.pm.IPackageManager");

    auto classClass = g_env->FindClass("java/lang/Class");
    auto getClassLoaderMethod = g_env->GetMethodID(classClass, "getClassLoader", "()Ljava/lang/ClassLoader;");

    auto classLoader = g_env->CallObjectMethod(iPackageManagerClass.getClass(), getClassLoaderMethod);
    auto classArray = g_env->NewObjectArray(1, classClass, NULL);
    g_env->SetObjectArrayElement(classArray, 0, iPackageManagerClass.getClass());

    auto apkKillerClass = g_env->FindClass("com/kuro/APKKiller");
    auto myInvocationHandlerField = g_env->GetStaticFieldID(apkKillerClass, "myInvocationHandler", "Ljava/lang/reflect/InvocationHandler;");
    auto myInvocationHandler = g_env->GetStaticObjectField(apkKillerClass, myInvocationHandlerField);

    auto proxyClass = g_env->FindClass("java/lang/reflect/Proxy");
    auto newProxyInstanceMethod = g_env->GetStaticMethodID(proxyClass, "newProxyInstance", "(Ljava/lang/ClassLoader;[Ljava/lang/Class;Ljava/lang/reflect/InvocationHandler;)Ljava/lang/Object;");
    g_proxy = g_env->NewGlobalRef(g_env->CallStaticObjectMethod(proxyClass, newProxyInstanceMethod, classLoader, classArray, myInvocationHandler));

    sPackageManagerField.set(sCurrentActivityThread, g_proxy);

    auto pm = getPackageManager(obj);
    Class applicationPackageManagerClass("android.app.ApplicationPackageManager");
    auto mPMField = applicationPackageManagerClass.getField("mPM");
    mPMField.set(pm, g_proxy);
}

void APKKill(JNIEnv *env, jclass clazz, jobject context) {
    env->PushLocalFrame(64); // We call this so that we don't need to manually delete the local refs

    g_env = env;
    g_assetManager = AAssetManager_fromJava(env, env->CallObjectMethod(context, env->GetMethodID(env->FindClass("android/content/Context"), "getAssets", "()Landroid/content/res/AssetManager;")));

    std::string apkPkg = getPackageName(context);
    g_pkgName = apkPkg;

    auto procName = getProcessName();
    LOGI("-------- Killing %s", procName);

    char apkDir[512];
    sprintf(apkDir, "/data/data/%s/cache", apkPkg.c_str());
    mkdir(apkDir, 0777);

    std::string apkPath = "/data/data/";
    apkPath += apkPkg;
    apkPath += "/cache/";
    apkPath += apk_fake_name;

    if (access(apkPath.c_str(), F_OK) == -1) {
        extractAsset(apk_asset_path, apkPath);
    }

    g_apkPath = (jstring) env->NewGlobalRef(g_env->NewStringUTF(apkPath.c_str()));

    Class activityThreadClass("android.app.ActivityThread");
    auto sCurrentActivityThreadField = activityThreadClass.getField("sCurrentActivityThread");
    auto sCurrentActivityThread = sCurrentActivityThreadField.get(NULL);

    auto mBoundApplicationField = activityThreadClass.getField("mBoundApplication");
    patch_AppBindData(mBoundApplicationField.get(sCurrentActivityThread));

    auto mInitialApplicationField = activityThreadClass.getField("mInitialApplication");
    patch_Application(mInitialApplicationField.get(sCurrentActivityThread));

    auto mAllApplicationsField = activityThreadClass.getField("mAllApplications");
    auto mAllApplications = mAllApplicationsField.get(sCurrentActivityThread);
    ArrayList list(mAllApplications);
    for (int i = 0; i < list.size(); i++) {
        auto application = list.get(i);
        patch_Application(application);
        list.set(i, application);
    }
    mAllApplicationsField.set(sCurrentActivityThread, list.getObj());

    auto mPackagesField = activityThreadClass.getField("mPackages");
    auto mPackages = mPackagesField.get(sCurrentActivityThread);
    ArrayMap mPackagesMap(mPackages);
    for (int i = 0; i < mPackagesMap.size(); i++) {
        WeakReference loadedApk(mPackagesMap.valueAt(i));
        patch_LoadedApk(loadedApk.get());
        mPackagesMap.setValueAt(i, WeakReference::Create(loadedApk.get()));
    }
    mPackagesField.set(sCurrentActivityThread, mPackagesMap.getObj());

    auto mResourcePackagesField = activityThreadClass.getField("mResourcePackages");
    auto mResourcePackages = mResourcePackagesField.get(sCurrentActivityThread);
    ArrayMap mResourcePackagesMap(mResourcePackages);
    for (int i = 0; i < mResourcePackagesMap.size(); i++) {
        WeakReference loadedApk(mResourcePackagesMap.valueAt(i));
        patch_LoadedApk(loadedApk.get());
        mResourcePackagesMap.setValueAt(i, WeakReference::Create(loadedApk.get()));
    }
    mResourcePackagesField.set(sCurrentActivityThread, mResourcePackagesMap.getObj());

    // patch_ContextImpl(getApplicationContext(context));
    patch_PackageManager(context);

    env->PopLocalFrame(0);
}

jobject processInvoke(JNIEnv *env, jclass clazz, jobject method, jobjectArray args) {
    env->PushLocalFrame(64);

    auto Integer_intValue = [&](jobject param) -> int {
        auto integerClass = env->FindClass("java/lang/Integer");
        auto intValueMethod = env->GetMethodID(integerClass, "intValue", "()I");
        return env->CallIntMethod(param, intValueMethod);
    };

    Method mMethod(env, method);
    const char *mName = mMethod.getName();
    auto mResult = mMethod.invoke(g_pkgMgr, args);

    if (!strcmp(mName, "getPackageInfo")) {
        const jobject packageInfo = mResult;
        if (packageInfo) {
            const char *packageName = env->GetStringUTFChars((jstring) env->GetObjectArrayElement(args, 0), 0);
            int flags = Integer_intValue(env->GetObjectArrayElement(args, 1));
            if (!strcmp(packageName, g_pkgName.c_str())) {
                if ((flags & 0x40) != 0) {
                    Class packageInfoClass(env, "android.content.pm.PackageInfo");
                    auto applicationInfoField = packageInfoClass.getField("applicationInfo");
                    auto applicationInfo = applicationInfoField.get(packageInfo);
                    if (applicationInfo) {
                        Class applicationInfoClass(env, "android.content.pm.ApplicationInfo");
                        auto sourceDirField = applicationInfoClass.getField("sourceDir");
                        auto publicSourceDirField = applicationInfoClass.getField("publicSourceDir");

                        sourceDirField.set(applicationInfo, g_apkPath);
                        publicSourceDirField.set(applicationInfo, g_apkPath);
                    }
                    applicationInfoField.set(packageInfo, applicationInfo);
                    auto signaturesField = packageInfoClass.getField("signatures");

                    auto signatureClass = env->FindClass("android/content/pm/Signature");
                    auto signatureConstructor = env->GetMethodID(signatureClass, "<init>", "([B)V");
                    auto signatureArray = env->NewObjectArray(apk_signatures.size(), signatureClass, NULL);
                    for (int i = 0; i < apk_signatures.size(); i++) {
                        auto signature = env->NewByteArray(apk_signatures[i].size());
                        env->SetByteArrayRegion(signature, 0, apk_signatures[i].size(), (jbyte *) apk_signatures[i].data());
                        env->SetObjectArrayElement(signatureArray, i, env->NewObject(signatureClass, signatureConstructor, signature));
                    }
                    signaturesField.set(packageInfo, signatureArray);
                } else if ((flags & 0x8000000) != 0) {
                    Class packageInfoClass(env, "android.content.pm.PackageInfo");
                    auto applicationInfoField = packageInfoClass.getField("applicationInfo");
                    auto applicationInfo = applicationInfoField.get(packageInfo);
                    if (applicationInfo) {
                        Class applicationInfoClass(env, "android.content.pm.ApplicationInfo");
                        auto sourceDirField = applicationInfoClass.getField("sourceDir");
                        auto publicSourceDirField = applicationInfoClass.getField("publicSourceDir");

                        sourceDirField.set(applicationInfo, g_apkPath);
                        publicSourceDirField.set(applicationInfo, g_apkPath);
                    }
                    applicationInfoField.set(packageInfo, applicationInfo);

                    auto signingInfoField = packageInfoClass.getField("signingInfo");
                    auto signingInfo = signingInfoField.get(packageInfo);

                    Class signingInfoClass(env, "android.content.pm.SigningInfo");
                    auto mSigningDetailsField = signingInfoClass.getField("mSigningDetails");
                    auto mSigningDetails = mSigningDetailsField.get(signingInfo);

                    Class signingDetailsClass(env, "android.content.pm.PackageParser$SigningDetails");
                    auto signaturesField = signingDetailsClass.getField("signatures");
                    auto pastSigningCertificatesField = signingDetailsClass.getField("pastSigningCertificates");

                    auto signatureClass = env->FindClass("android/content/pm/Signature");
                    auto signatureConstructor = env->GetMethodID(signatureClass, "<init>", "([B)V");
                    auto signatureArray = env->NewObjectArray(apk_signatures.size(), signatureClass, NULL);
                    for (int i = 0; i < apk_signatures.size(); i++) {
                        auto signature = env->NewByteArray(apk_signatures[i].size());
                        env->SetByteArrayRegion(signature, 0, apk_signatures[i].size(), (jbyte *) apk_signatures[i].data());
                        env->SetObjectArrayElement(signatureArray, i, env->NewObject(signatureClass, signatureConstructor, signature));
                    }
                    signaturesField.set(mSigningDetails, signatureArray);
                    pastSigningCertificatesField.set(mSigningDetails, signatureArray);
                } else {
                    Class packageInfoClass(env, "android.content.pm.PackageInfo");
                    auto applicationInfoField = packageInfoClass.getField("applicationInfo");
                    auto applicationInfo = applicationInfoField.get(packageInfo);
                    if (applicationInfo) {
                        Class applicationInfoClass(env, "android.content.pm.ApplicationInfo");
                        auto sourceDirField = applicationInfoClass.getField("sourceDir");
                        auto publicSourceDirField = applicationInfoClass.getField("publicSourceDir");

                        sourceDirField.set(applicationInfo, g_apkPath);
                        publicSourceDirField.set(applicationInfo, g_apkPath);
                    }
                    applicationInfoField.set(packageInfo, applicationInfo);
                }
            }
        }
    } else if (!strcmp(mName, "getApplicationInfo")) {
        const char *packageName = env->GetStringUTFChars((jstring) env->GetObjectArrayElement(args, 0), 0);
        if (!strcmp(packageName, g_pkgName.c_str())) {
            auto applicationInfo = mResult;
            if (applicationInfo) {
                Class applicationInfoClass(env, "android.content.pm.ApplicationInfo");

                auto sourceDirField = applicationInfoClass.getField("sourceDir");
                auto publicSourceDirField = applicationInfoClass.getField("publicSourceDir");

                sourceDirField.set(applicationInfo, g_apkPath);
                publicSourceDirField.set(applicationInfo, g_apkPath);
            }
        }
    } else if (!strcmp(mName, "getInstallerPackageName")) {
        const char *packageName = env->GetStringUTFChars((jstring) env->GetObjectArrayElement(args, 0), 0);
        if (!strcmp(packageName, g_pkgName.c_str())) {
            mResult = env->NewStringUTF("com.android.vending");
        }
    }

    if (mResult) {
        mResult = env->NewGlobalRef(mResult);
    }
    return env->PopLocalFrame(mResult); // make sure all local refs are deleted except for the result
}
