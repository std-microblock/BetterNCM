#pragma once
#include "pch.h"

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <filesystem>

#include <windows.h>

#include <stdexcept>

class SharedMemory {
private:
    HANDLE hMapFile;
    LPVOID buf;

public:
    SharedMemory(const char* name, size_t size) {
        hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, name);
        if (hMapFile == NULL) {
            throw std::runtime_error("Could not create file mapping object");
        }

        buf = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, size);
        if (buf == NULL) {
            CloseHandle(hMapFile);
            throw std::runtime_error("Could not map file to memory");
        }
    }

    ~SharedMemory() {
        UnmapViewOfFile(buf);
        CloseHandle(hMapFile);
    }

    LPVOID getBuf() const {
        return buf;
    }

    void write(const void* data, size_t size) {
        CopyMemory(buf, data, size);
    }

    void read(void* data, size_t size) {
        CopyMemory(data, buf, size);
    }
};


template<typename T>
class SharedMemoryData {
private:
    SharedMemory mem;
    T* data;

public:
    SharedMemoryData(const char* name) : mem(name, sizeof(T)) {
        data = static_cast<T*>(mem.getBuf());
    }

    void write(const T& value) {
        *data = value;
    }

    T read() {
        return *data;
    }

    void wait_for(const T& val, const int interval=10) const {
        while (*data != val) {
            Sleep(interval);
        }
    }
};

template<>
class SharedMemoryData<std::string> {
private:
    static const size_t MAX_LENGTH = 256;
    SharedMemory mem;
    char* data;

public:
    SharedMemoryData(const char* name) : mem(name, MAX_LENGTH) {
        data = static_cast<char*>(mem.getBuf());
    }

    void write(const std::string& value) {
        memset(data, '\0', MAX_LENGTH);
        strncpy(data, value.c_str(), MAX_LENGTH - 1);
    }

    std::string_view read() {
        return std::string_view(data);
    }

};