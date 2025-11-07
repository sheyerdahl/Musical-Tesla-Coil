//
// PSRAM Allocator for ESP32
// This header provides PSRAM-based memory allocation for the midifile library
//

#ifndef _PSRAM_ALLOCATOR_H_INCLUDED
#define _PSRAM_ALLOCATOR_H_INCLUDED

#if defined(ESP_PLATFORM) || defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP32S3)
#include <esp_heap_caps.h>
#ifdef ARDUINO
#include <Arduino.h>
#endif

// PSRAM allocation functions
inline void* ps_malloc_impl(size_t size) {
    void* ptr = heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    if (ptr == nullptr && size > 0) {
        // Fallback to regular heap if PSRAM is full
        ptr = heap_caps_malloc(size, MALLOC_CAP_DEFAULT);
    }
    return ptr;
}

inline void* ps_calloc_impl(size_t num, size_t size) {
    void* ptr = heap_caps_calloc(num, size, MALLOC_CAP_SPIRAM);
    if (ptr == nullptr && size > 0) {
        // Fallback to regular heap if PSRAM is full
        ptr = heap_caps_calloc(num, size, MALLOC_CAP_DEFAULT);
    }
    return ptr;
}

inline void* ps_realloc_impl(void* ptr, size_t size) {
    if (ptr == nullptr) {
        return ps_malloc_impl(size);
    }
    // Check if pointer is in PSRAM
    if (heap_caps_get_allocated_size(ptr) > 0) {
        void* new_ptr = heap_caps_realloc(ptr, size, MALLOC_CAP_SPIRAM);
        if (new_ptr == nullptr && size > 0) {
            // Fallback to regular heap
            new_ptr = heap_caps_realloc(ptr, size, MALLOC_CAP_DEFAULT);
        }
        return new_ptr;
    }
    // If not in PSRAM, use regular realloc
    return heap_caps_realloc(ptr, size, MALLOC_CAP_DEFAULT);
}

inline void ps_free_impl(void* ptr) {
    if (ptr != nullptr) {
        heap_caps_free(ptr);
    }
}

// Override operator new and delete for classes that should use PSRAM
// This will be included in the class definitions

#else
// Non-ESP32 platform: use regular malloc
inline void* ps_malloc_impl(size_t size) {
    return malloc(size);
}

inline void* ps_calloc_impl(size_t num, size_t size) {
    return calloc(num, size);
}

inline void* ps_realloc_impl(void* ptr, size_t size) {
    return realloc(ptr, size);
}

inline void ps_free_impl(void* ptr) {
    free(ptr);
}
#endif

#endif /* _PSRAM_ALLOCATOR_H_INCLUDED */

