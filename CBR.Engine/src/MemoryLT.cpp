//--------------------------------------------------------------------------------
// MemoryLT.cpp
//
// Copyright (c) 2022, Cristian Vasile
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL CRISTIAN VASILE BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: Cristian Vasile
//--------------------------------------------------------------------------------
#include "pch.h"
#include "Engine/Debug/MemoryLT.h"
#include "Engine/Debug/Logger.h"

#if defined(_DEBUG) || defined(DEBUG)

#include <new>
#ifdef _MSC_VER
#include <sal.h>
#endif

/// Override (only for this file) the _HAS_EXCEPTIONS values. Otherwise a compile error is 
/// generated (aka: error C3861: '__uncaught_exception': identifier not found)
#ifdef _HAS_EXCEPTIONS
#undef _HAS_EXCEPTIONS
#endif //_HAS_EXCEPTIONS
#define _HAS_EXCEPTIONS 2

#if defined new
#undef new
#endif

namespace CBR::Engine::Debug::mlt
{
    typedef void* (*AllocFuncPtr)(std::size_t, const char*, unsigned int);
    typedef void(*FreeFuncPtr)(void*);

    /** Allocates memory for usual usage*/
	void* Alloc(std::size_t size, const char* file, unsigned int line);

    /** Free memory for usual usage*/
	void Free(void* mem);

    struct MemoryAllocationRecord
    {
        /**address returned to the caller after allocation*/
        void* address_;

        /**size of the allocation request*/
        std::size_t size_;

        /**source file of allocation request*/
        const char* file_;

        /**source line of the allocation request*/
        unsigned int line_;

        /**linked list next node*/
        MemoryAllocationRecord* next_;
        /**linked list prev node*/
        MemoryAllocationRecord* prev_;
    };

	class LeakTracker
	{
	public:
		LeakTracker();
		~LeakTracker();
		void* Alloc(std::size_t size, const char* file, unsigned int line);
		void Free(void* p);

		/** Prints all heap and reference leaks to stderr. */
		static void PrintMemoryLeaks();
        static void CheckHeapCorruption();

        void CheckHeapCorruptionAtAddress(void* address);

	private:
		MemoryAllocationRecord* memoryAllocations_;
		int memoryAllocationCount_;
		std::recursive_mutex mutex_;
		std::size_t maxSize_;
		std::size_t maxLine_;
	};

    static std::mutex InitMutex_;
    static bool HeapCorruptionEnabled_ = false;
    static int HeapCorruptionBuferSize_ = 2048;
	static AllocFuncPtr AllocFuncPtr_ = nullptr;
	static FreeFuncPtr  FreeFuncPtr_ = nullptr;

    /** We reserve some memory to hold the mem for s_leakTracker */
    static char MemleakTracker_[sizeof(LeakTracker)] = { 0 };

    /** Is the static pointer for leakTracker */
    static LeakTracker* LeakTracker_ = nullptr;


    void Init(bool heapCorruptionCheck, int buffer)
    {
        std::lock_guard<std::mutex> lk(InitMutex_);
        HeapCorruptionEnabled_ = heapCorruptionCheck;
        HeapCorruptionBuferSize_ = buffer;
        LeakTracker_ = new(MemleakTracker_) LeakTracker;
        AllocFuncPtr_ = Alloc;
		FreeFuncPtr_ = Free;
    }

    void Close()
    {
        std::lock_guard<std::mutex> lk(InitMutex_);
        if (LeakTracker_)
        {
            LeakTracker_->PrintMemoryLeaks();
            AllocFuncPtr_ = nullptr;
            FreeFuncPtr_ = nullptr;
            LeakTracker_ = nullptr;
        }
    }

    void CheckHeapCorruption()
    {
        if (!LeakTracker_)
            return;

        LeakTracker_->CheckHeapCorruption();
    }

    void* Alloc(std::size_t size, const char* file, unsigned int line)
    {
        return LeakTracker_->Alloc(size, file, line);
    }

    void LeakTrackerExit()
    {
        AllocFuncPtr_ = nullptr;
        FreeFuncPtr_ = nullptr;

		if (LeakTracker_)
		{
			LeakTracker_->PrintMemoryLeaks();
            LeakTracker_ = nullptr;
		}
    }

    void Free(void* mem)
    {
        LeakTracker_->Free(mem);
    }


    LeakTracker::LeakTracker() 
        : memoryAllocations_(0)
        , memoryAllocationCount_(0)
        , maxSize_(0)
        , maxLine_(0)
    {
        atexit(LeakTrackerExit);

        // construct your heap here
    }

    LeakTracker::~LeakTracker()
    {
    }

    void* Debug::mlt::LeakTracker::Alloc(std::size_t size, const char* file, unsigned int line)
    {
        // nothrow new 或者你显式调用的“非跟踪”路径：不记录，直接返回
        if (file == nullptr) {
            return std::malloc(size);
        }

        unsigned char* mem = nullptr;
        MemoryAllocationRecord* rec = nullptr;
        void* payloadAddr = nullptr;

        if (HeapCorruptionEnabled_) {
            // 带哨兵区
            const std::size_t total =
                sizeof(MemoryAllocationRecord) + size + HeapCorruptionBuferSize_ * 2;
            mem = static_cast<unsigned char*>(std::calloc(total, 1));
            if (!mem) {
                // throwing new 语义：用 bad_alloc 反映 OOM
                throw std::bad_alloc();
            }
            rec = reinterpret_cast<MemoryAllocationRecord*>(mem + HeapCorruptionBuferSize_);
            payloadAddr = mem + HeapCorruptionBuferSize_ + sizeof(MemoryAllocationRecord);
        }
        else {
            // 普通路径
            const std::size_t total = sizeof(MemoryAllocationRecord) + size;
            mem = static_cast<unsigned char*>(std::malloc(total));
            if (!mem) {
                throw std::bad_alloc();
            }
            rec = reinterpret_cast<MemoryAllocationRecord*>(mem);
            payloadAddr = mem + sizeof(MemoryAllocationRecord);
        }

        // —— 从这里开始 rec 一定非空 ——
        {
            std::lock_guard<std::recursive_mutex> lk(mutex_);
            rec->address_ = payloadAddr;
            rec->size_ = size;      // ← 用 size_t，不要强转 unsigned int
            rec->file_ = file;
            rec->line_ = line;
            rec->next_ = memoryAllocations_;
            rec->prev_ = nullptr;

            if (maxSize_ < size) maxSize_ = size;
            if (maxLine_ < line) maxLine_ = line;

            if (memoryAllocations_) memoryAllocations_->prev_ = rec;
            memoryAllocations_ = rec;
            ++memoryAllocationCount_;
        }

        return payloadAddr;
    }


    void LeakTracker::Free(void* payloadAddr)
    {
        if (payloadAddr == 0)
            return;

        unsigned char* mem = nullptr;
        MemoryAllocationRecord* rec = nullptr;

        if (HeapCorruptionEnabled_)
        {
            /// Backup passed in pointer to access memory allocation record
            mem = ((unsigned char*)payloadAddr) - sizeof(MemoryAllocationRecord) - HeapCorruptionBuferSize_;

            rec = (MemoryAllocationRecord*)(((unsigned char*)payloadAddr) - sizeof(MemoryAllocationRecord));
        }
        else
        {
            /// Backup passed in pointer to access memory allocation record
            mem = ((unsigned char*)payloadAddr) - sizeof(MemoryAllocationRecord);

            rec = (MemoryAllocationRecord*)mem;
        }

        /// Sanity check: ensure that address in record matches passed in address
        if (rec->address_ != payloadAddr)
        {
            // This case could be a memory corruption, but most of the cases are memory allocations that was not tracked (because file was null).
            free(payloadAddr);
            return;
        }

        /// Sanity check: ensure that size is smaller than maximum (tracked)
        if (rec->size_ > maxSize_)
        {
            std::cout << ("[memory] CORRUPTION: Attempting to free memory address with invalid memory allocation record (wrong size).\n");
            //std::cout << "S";
            return;
        }

        /// Sanity check: ensure that line is smaller than maximum (tracked)
        if (rec->line_ > maxLine_)
        {
            std::cout << ("[memory] CORRUPTION: Attempting to free memory address with invalid memory allocation record (wrong line).\n");
            //std::cout << "L";
            return;
        }

        if (HeapCorruptionEnabled_)
        {
            CheckHeapCorruptionAtAddress(payloadAddr);
        }

        /// Link this item out
        mutex_.lock();
        if (memoryAllocations_ == rec)
            memoryAllocations_ = rec->next_;
        if (rec->prev_)
            rec->prev_->next_ = rec->next_;
        if (rec->next_)
            rec->next_->prev_ = rec->prev_;
        --memoryAllocationCount_;
        mutex_.unlock();

        /// Free the address from the original alloc location (before mem allocation record)
        free(mem);
    }

    void LeakTracker::PrintMemoryLeaks()
    {
        printf("\n");

		LeakTracker_->mutex_.lock();
        /// Dump general heap memory leaks
		if (LeakTracker_->memoryAllocationCount_ == 0)
        {
            LOG_INFO("[memory] All HEAP allocations successfully cleaned up (no leaks detected).");
        }
        else
        {
            LOG_WARN("[memory] WARNING: %d  HEAP allocations still active in memory.", LeakTracker_->memoryAllocationCount_);

			MemoryAllocationRecord* rec = LeakTracker_->memoryAllocations_;
            
            while (rec)
            {
                if (strlen(rec->file_) > 0)
                {
                    LOG_WARN("[memory] LEAK: At address %p, size %zu, %s:%u.\n", +rec->address_, rec->size_, rec->file_, rec->line_);
                }
                rec = rec->next_;
            }
        }

		LeakTracker_->mutex_.unlock();
    }

    void LeakTracker::CheckHeapCorruptionAtAddress(void* address)
    {
        /// Backup passed in pointer to access memory allocation record
        unsigned char* mem = ((unsigned char*)address) - sizeof(MemoryAllocationRecord) - HeapCorruptionBuferSize_;

        MemoryAllocationRecord* rec = (MemoryAllocationRecord*)(((unsigned char*)address) - sizeof(MemoryAllocationRecord));

        unsigned char* addrStart = mem;
        unsigned char* addrEnd = addrStart + HeapCorruptionBuferSize_;
        for (; addrStart < addrEnd; addrStart++)
        {
            if (*addrStart != 0)
            {
                LOG_WARN("[memory] CORRUPTION: before address %p, size %zu, %s:%u.\n", rec->address_, rec->size_, rec->file_, rec->line_);
                break;
            }
        }

        addrStart = (unsigned char*)address + rec->size_;
        addrEnd = addrStart + HeapCorruptionBuferSize_;
        for (; addrStart < addrEnd; addrStart++)
        {
            if (*addrStart != 0)
            {
                LOG_WARN("[memory] CORRUPTION: after address %p, size %zu, %s:%u.\n", rec->address_, rec->size_, rec->file_, rec->line_);
            }
        }
    }

    void LeakTracker::CheckHeapCorruption()
    {
        if (!HeapCorruptionEnabled_)
            return;

        MemoryAllocationRecord* rec = LeakTracker_->memoryAllocations_;
                      
        while (rec)
        {
            LeakTracker_->CheckHeapCorruptionAtAddress(rec->address_);

            rec = rec->next_;
        }
    }
} //CBR::Engine::Debug



#ifdef _MSC_VER
#pragma warning( disable : 4290 )
#endif

void* operator new (std::size_t size, const char* file, int line)
{
	if(CBR::Engine::Debug::mlt::AllocFuncPtr_)
		return CBR::Engine::Debug::mlt::AllocFuncPtr_(size, file, line);
	else
		return malloc(size);
}

void* operator new[](std::size_t size, const char* file, int line)
{
	return operator new (size, file, line);
}

#ifdef _MSC_VER
_Ret_notnull_ _Post_writable_byte_size_(size)
#endif
void* operator new (std::size_t size) noexcept(false)
{
    return operator new (size, nullptr, 0);
}

#ifdef _MSC_VER
_Ret_notnull_ _Post_writable_byte_size_(size)
#endif
void* operator new[](std::size_t size) noexcept(false)
{
    return operator new (size, nullptr, 0);
}

#ifdef _MSC_VER
_Ret_maybenull_ _Success_(return != 0) _Post_writable_byte_size_(size)
#endif
void* operator new (std::size_t size, const std::nothrow_t&) noexcept
{
    return operator new (size, nullptr, 0);
}

#ifdef _MSC_VER
_Ret_maybenull_ _Success_(return != 0) _Post_writable_byte_size_(size)
#endif
void* operator new[](std::size_t size, const std::nothrow_t&) noexcept
{
    return operator new (size, nullptr, 0);
}

void operator delete (void* p) noexcept
{
	if(CBR::Engine::Debug::mlt::FreeFuncPtr_)
        CBR::Engine::Debug::mlt::FreeFuncPtr_(p);
	else
		free(p);
}

void operator delete[](void* p) noexcept
{
	if(CBR::Engine::Debug::mlt::FreeFuncPtr_)
        CBR::Engine::Debug::mlt::FreeFuncPtr_(p);
	else
		free(p);
}

void operator delete (void* p, const char* file, int line) noexcept
{
	if(CBR::Engine::Debug::mlt::FreeFuncPtr_)
        CBR::Engine::Debug::mlt::FreeFuncPtr_(p);
	else
		free(p);
}

void operator delete[](void* p, const char* file, int line) noexcept
{
	if(CBR::Engine::Debug::mlt::FreeFuncPtr_)
        CBR::Engine::Debug::mlt::FreeFuncPtr_(p);
	else
		free(p);
}

#ifdef _MSC_VER
#pragma warning( default : 4290 )
#endif

namespace CBR::Engine::Debug::mlt
{
    void* BaseLeakTracker::operator new(size_t size)
	{
		if(Debug::mlt::AllocFuncPtr_)
			return Debug::mlt::AllocFuncPtr_(size, nullptr, 0);
		else
			return malloc(size);
	}

    void BaseLeakTracker::operator delete(void* p)
	{
		if(Debug::mlt::FreeFuncPtr_)
            Debug::mlt::FreeFuncPtr_(p);
		else
			free(p);
    }

    void* BaseLeakTracker::operator new[](size_t size)
	{
		if(Debug::mlt::AllocFuncPtr_)
			return Debug::mlt::AllocFuncPtr_(size, nullptr, 0);
		else
			return malloc(size);
    }

    void BaseLeakTracker::operator delete[](void* p)
	{
		if(Debug::mlt::FreeFuncPtr_)
            Debug::mlt::FreeFuncPtr_(p);
		else
			free(p);
	};

    void* BaseLeakTracker::operator new(size_t size, const char *file, int line)
	{
		if(Debug::mlt::AllocFuncPtr_)
			return Debug::mlt::AllocFuncPtr_(size, file, line);
		else
			return malloc(size);
	};

    void BaseLeakTracker::operator delete(void* p, const char *file, int line)
	{
		if(Debug::mlt::FreeFuncPtr_)
            Debug::mlt::FreeFuncPtr_(p);
		else
			free(p);
	};

    void* BaseLeakTracker::operator new[](size_t size, const char *file, int line)
	{
		if(Debug::mlt::AllocFuncPtr_)
			return Debug::mlt::AllocFuncPtr_(size, file, line);
		else
			return malloc(size);
	}

    void BaseLeakTracker::operator delete[](void* p, const char *file, int line)
	{
		if(Debug::mlt::FreeFuncPtr_)
            Debug::mlt::FreeFuncPtr_(p);
		else
			free(p);
	};
} // namespace CBR::Engine::Debug::mlt
#endif //_DEBUG
