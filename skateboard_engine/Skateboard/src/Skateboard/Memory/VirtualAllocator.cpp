#include "sktbdpch.h"
#include "VirtualAllocator.h"

#define SKTBD_LOG_COMPONENT "ALLOCATOR"
#include "Skateboard/Log.h"

#ifdef SKTBD_PLATFORM_WINDOWS
#include <combaseapi.h>
#endif

#include <mutex>
#include <algorithm>
#include <utility>
#include <cstdlib>
#include <cstdint>

#ifndef _WIN32
#include <shared_mutex>
#endif

namespace Skateboard::MemoryUtils
{
enum SuballocationType
{
    SUBALLOCATION_TYPE_FREE = 0,
    SUBALLOCATION_TYPE_ALLOCATION = 1,
};

#define MA_DEBUG_MARGIN (0)

#ifndef MA_DEBUG_LOG
#define MA_DEBUG_LOG(format, ...)
/*
#define MA_DEBUG_LOG(format, ...) do { \
    wprintf(format, __VA_ARGS__); \
    wprintf(L"\n"); \
} while(false)
*/
#endif

#ifndef _MA_FUNCTIONS

static void* DefaultAllocate(size_t Size, size_t Alignment, void* /*pPrivateData*/)
{
#ifdef _WIN32
    return _aligned_malloc(Size, Alignment);
#else
    return aligned_alloc(Alignment, Size);
#endif
}
static void DefaultFree(void* pMemory, void* /*pPrivateData*/)
{
#ifdef _WIN32
    return _aligned_free(pMemory);
#else
    return free(pMemory);
#endif
}

static void* Malloc(const ALLOCATION_CALLBACKS& allocs, size_t size, size_t alignment)
{
    void* const result = (*allocs.pAllocate)(size, alignment, allocs.pPrivateData);
    SKTBD_ASSERT(result);
    return result;
}
static void Free(const ALLOCATION_CALLBACKS& allocs, void* memory)
{
    (*allocs.pFree)(memory, allocs.pPrivateData);
}

template<typename T>
static T* Allocate(const ALLOCATION_CALLBACKS& allocs)
{
    return (T*)Malloc(allocs, sizeof(T), __alignof(T));
}
template<typename T>
static T* AllocateArray(const ALLOCATION_CALLBACKS& allocs, size_t count)
{
    return (T*)Malloc(allocs, sizeof(T) * count, __alignof(T));
}

#define MA_NEW(allocs, type) new(Allocate<type>(allocs))(type)
#define MA_NEW_ARRAY(allocs, type, count) new(AllocateArray<type>((allocs), (count)))(type)

template<typename T>
void MA_DELETE(const ALLOCATION_CALLBACKS& allocs, T* memory)
{
    if (memory)
    {
        memory->~T();
        Free(allocs, memory);
    }
}
template<typename T>
void MA_DELETE_ARRAY(const ALLOCATION_CALLBACKS& allocs, T* memory, size_t count)
{
    if (memory)
    {
        for (size_t i = count; i--; )
        {
            memory[i].~T();
        }
        Free(allocs, memory);
    }
}

static void SetupAllocationCallbacks(ALLOCATION_CALLBACKS& outAllocs, const ALLOCATION_CALLBACKS* allocationCallbacks)
{
    if (allocationCallbacks)
    {
        outAllocs = *allocationCallbacks;
        SKTBD_ASSERT(outAllocs.pAllocate != NULL && outAllocs.pFree != NULL);
    }
    else
    {
        outAllocs.pAllocate = &DefaultAllocate;
        outAllocs.pFree = &DefaultFree;
        outAllocs.pPrivateData = NULL;
    }
}

#define SAFE_RELEASE(ptr)   do { if(ptr) { (ptr)->Release(); (ptr) = NULL; } } while(false)

#define MA_VALIDATE(cond) do { if(!(cond)) { \
    SKTBD_ASSERT(0, "Validation failed: " #cond); \
    return false; \
} } while(false)

template<typename T>
static T MA_MIN(const T& a, const T& b) { return a <= b ? a : b; }
template<typename T>
static T MA_MAX(const T& a, const T& b) { return a <= b ? b : a; }

template<typename T>
static void MA_SWAP(T& a, T& b) { T tmp = a; a = b; b = tmp; }

// Scans integer for index of first nonzero bit from the Least Significant Bit (LSB). If mask is 0 then returns UINT8_MAX
static uint8_t BitScanLSB(uint64_t mask)
{
#if defined(_MSC_VER) && defined(_WIN64)
    unsigned long pos;
    if (_BitScanForward64(&pos, mask))
        return static_cast<uint8_t>(pos);
    return UINT8_MAX;
#elif defined __GNUC__ || defined __clang__
    return static_cast<uint8_t>(__builtin_ffsll(mask)) - 1U;
#else
    uint8_t pos = 0;
    uint64_t bit = 1;
    do
    {
        if (mask & bit)
            return pos;
        bit <<= 1;
    } while (pos++ < 63);
    return UINT8_MAX;
#endif
}
// Scans integer for index of first nonzero bit from the Least Significant Bit (LSB). If mask is 0 then returns UINT8_MAX
static uint8_t BitScanLSB(uint32_t mask)
{
#ifdef _MSC_VER
    unsigned long pos;
    if (_BitScanForward(&pos, mask))
        return static_cast<uint8_t>(pos);
    return UINT8_MAX;
#elif defined __GNUC__ || defined __clang__
    return static_cast<uint8_t>(__builtin_ffs(mask)) - 1U;
#else
    uint8_t pos = 0;
    uint32_t bit = 1;
    do
    {
        if (mask & bit)
            return pos;
        bit <<= 1;
    } while (pos++ < 31);
    return UINT8_MAX;
#endif
}

// Scans integer for index of first nonzero bit from the Most Significant Bit (MSB). If mask is 0 then returns UINT8_MAX
static uint8_t BitScanMSB(uint64_t mask)
{
#if defined(_MSC_VER) && defined(_WIN64)
    unsigned long pos;
    if (_BitScanReverse64(&pos, mask))
        return static_cast<uint8_t>(pos);
#elif defined __GNUC__ || defined __clang__
    if (mask)
        return 63 - static_cast<uint8_t>(__builtin_clzll(mask));
#else
    uint8_t pos = 63;
    uint64_t bit = 1ULL << 63;
    do
    {
        if (mask & bit)
            return pos;
        bit >>= 1;
    } while (pos-- > 0);
#endif
    return UINT8_MAX;
}
// Scans integer for index of first nonzero bit from the Most Significant Bit (MSB). If mask is 0 then returns UINT8_MAX
static uint8_t BitScanMSB(uint32_t mask)
{
#ifdef _MSC_VER
    unsigned long pos;
    if (_BitScanReverse(&pos, mask))
        return static_cast<uint8_t>(pos);
#elif defined __GNUC__ || defined __clang__
    if (mask)
        return 31 - static_cast<uint8_t>(__builtin_clz(mask));
#else
    uint8_t pos = 31;
    uint32_t bit = 1UL << 31;
    do
    {
        if (mask & bit)
            return pos;
        bit >>= 1;
    } while (pos-- > 0);
#endif
    return UINT8_MAX;
}

/*
Returns true if given number is a power of two.
T must be unsigned integer number or signed integer but always nonnegative.
For 0 returns true.
*/
template <typename T>
static bool IsPow2(T x) { return (x & (x - 1)) == 0; }

// Aligns given value up to nearest multiply of align value. For example: AlignUp(11, 8) = 16.
// Use types like uint32_t, uint64_t as T.
template <typename T>
static T AlignUp(T val, T alignment)
{
    MA_HEAVY_ASSERT(IsPow2(alignment));
    return (val + alignment - 1) & ~(alignment - 1);
}
// Aligns given value down to nearest multiply of align value. For example: AlignUp(11, 8) = 8.
// Use types like uint32_t, uint64_t as T.
template <typename T>
static T AlignDown(T val, T alignment)
{
    MA_HEAVY_ASSERT(IsPow2(alignment));
    return val & ~(alignment - 1);
}

// Division with mathematical rounding to nearest number.
template <typename T>
static T RoundDiv(T x, T y) { return (x + (y / (T)2)) / y; }
template <typename T>
static T DivideRoundingUp(T x, T y) { return (x + y - 1) / y; }

static wchar_t HexDigitToChar(uint8_t digit)
{
    if (digit < 10)
        return L'0' + digit;
    else
        return L'A' + (digit - 10);
}

/*
Performs binary search and returns iterator to first element that is greater or
equal to `key`, according to comparison `cmp`.

Cmp should return true if first argument is less than second argument.

Returned value is the found element, if present in the collection or place where
new element with value (key) should be inserted.
*/
template <typename CmpLess, typename IterT, typename KeyT>
static IterT BinaryFindFirstNotLess(IterT beg, IterT end, const KeyT& key, const CmpLess& cmp)
{
    size_t down = 0, up = (end - beg);
    while (down < up)
    {
        const size_t mid = (down + up) / 2;
        if (cmp(*(beg + mid), key))
        {
            down = mid + 1;
        }
        else
        {
            up = mid;
        }
    }
    return beg + down;
}

/*
Performs binary search and returns iterator to an element that is equal to `key`,
according to comparison `cmp`.

Cmp should return true if first argument is less than second argument.

Returned value is the found element, if present in the collection or end if not
found.
*/
template<typename CmpLess, typename IterT, typename KeyT>
static IterT BinaryFindSorted(const IterT& beg, const IterT& end, const KeyT& value, const CmpLess& cmp)
{
    IterT it = BinaryFindFirstNotLess<CmpLess, IterT, KeyT>(beg, end, value, cmp);
    if (it == end ||
        (!cmp(*it, value) && !cmp(value, *it)))
    {
        return it;
    }
    return end;
}

#endif // _MA_FUNCTIONS

#ifndef _MA_STATISTICS_FUNCTIONS

static void ClearStatistics(Statistics& outStats)
{
    outStats.BlockCount = 0;
    outStats.AllocationCount = 0;
    outStats.BlockBytes = 0;
    outStats.AllocationBytes = 0;
}

static void ClearDetailedStatistics(DetailedStatistics& outStats)
{
    ClearStatistics(outStats.Stats);
    outStats.UnusedRangeCount = 0;
    outStats.AllocationSizeMin = UINT64_MAX;
    outStats.AllocationSizeMax = 0;
    outStats.UnusedRangeSizeMin = UINT64_MAX;
    outStats.UnusedRangeSizeMax = 0;
}

static void AddStatistics(Statistics& inoutStats, const Statistics& src)
{
    inoutStats.BlockCount += src.BlockCount;
    inoutStats.AllocationCount += src.AllocationCount;
    inoutStats.BlockBytes += src.BlockBytes;
    inoutStats.AllocationBytes += src.AllocationBytes;
}

static void AddDetailedStatistics(DetailedStatistics& inoutStats, const DetailedStatistics& src)
{
    AddStatistics(inoutStats.Stats, src.Stats);
    inoutStats.UnusedRangeCount += src.UnusedRangeCount;
    inoutStats.AllocationSizeMin = MA_MIN(inoutStats.AllocationSizeMin, src.AllocationSizeMin);
    inoutStats.AllocationSizeMax = MA_MAX(inoutStats.AllocationSizeMax, src.AllocationSizeMax);
    inoutStats.UnusedRangeSizeMin = MA_MIN(inoutStats.UnusedRangeSizeMin, src.UnusedRangeSizeMin);
    inoutStats.UnusedRangeSizeMax = MA_MAX(inoutStats.UnusedRangeSizeMax, src.UnusedRangeSizeMax);
}

static void AddDetailedStatisticsAllocation(DetailedStatistics& inoutStats, uint64_t size)
{
    inoutStats.Stats.AllocationCount++;
    inoutStats.Stats.AllocationBytes += size;
    inoutStats.AllocationSizeMin = MA_MIN(inoutStats.AllocationSizeMin, size);
    inoutStats.AllocationSizeMax = MA_MAX(inoutStats.AllocationSizeMax, size);
}

static void AddDetailedStatisticsUnusedRange(DetailedStatistics& inoutStats, uint64_t size)
{
    inoutStats.UnusedRangeCount++;
    inoutStats.UnusedRangeSizeMin = MA_MIN(inoutStats.UnusedRangeSizeMin, size);
    inoutStats.UnusedRangeSizeMax = MA_MAX(inoutStats.UnusedRangeSizeMax, size);
}

#endif // _MA_STATISTICS_FUNCTIONS

#ifndef _MA_MUTEX

#ifndef MA_MUTEX
class Mutex
{
public:
    void Lock() { m_Mutex.lock(); }
    void Unlock() { m_Mutex.unlock(); }

private:
    std::mutex m_Mutex;
};
#define MA_MUTEX Mutex
#endif

#ifndef MA_RW_MUTEX
#ifdef _WIN32
class RWMutex
{
public:
    RWMutex() { InitializeSRWLock(&m_Lock); }
    void LockRead() { AcquireSRWLockShared(&m_Lock); }
    void UnlockRead() { ReleaseSRWLockShared(&m_Lock); }
    void LockWrite() { AcquireSRWLockExclusive(&m_Lock); }
    void UnlockWrite() { ReleaseSRWLockExclusive(&m_Lock); }

private:
    SRWLOCK m_Lock;
};
#else // #ifdef _WIN32
class RWMutex
{
public:
    RWMutex() {}
    void LockRead() { m_Mutex.lock_shared(); }
    void UnlockRead() { m_Mutex.unlock_shared(); }
    void LockWrite() { m_Mutex.lock(); }
    void UnlockWrite() { m_Mutex.unlock(); }

private:
    std::shared_timed_mutex m_Mutex;
};
#endif // #ifdef _WIN32
#define MA_RW_MUTEX RWMutex
#endif // #ifndef MA_RW_MUTEX

// Helper RAII class to lock a mutex in constructor and unlock it in destructor (at the end of scope).
struct MutexLock
{
    MA_CLASS_NO_COPY(MutexLock);
public:
    MutexLock(MA_MUTEX& mutex, bool useMutex = true) :
        m_pMutex(useMutex ? &mutex : NULL)
    {
        if (m_pMutex) m_pMutex->Lock();
    }
    ~MutexLock() { if (m_pMutex) m_pMutex->Unlock(); }

private:
    MA_MUTEX* m_pMutex;
};

// Helper RAII class to lock a RW mutex in constructor and unlock it in destructor (at the end of scope), for reading.
struct MutexLockRead
{
    MA_CLASS_NO_COPY(MutexLockRead);
public:
    MutexLockRead(MA_RW_MUTEX& mutex, bool useMutex)
        : m_pMutex(useMutex ? &mutex : NULL)
    {
        if (m_pMutex)
        {
            m_pMutex->LockRead();
        }
    }
    ~MutexLockRead() { if (m_pMutex) m_pMutex->UnlockRead(); }

private:
    MA_RW_MUTEX* m_pMutex;
};

// Helper RAII class to lock a RW mutex in constructor and unlock it in destructor (at the end of scope), for writing.
struct MutexLockWrite
{
    MA_CLASS_NO_COPY(MutexLockWrite);
public:
    MutexLockWrite(MA_RW_MUTEX& mutex, bool useMutex)
        : m_pMutex(useMutex ? &mutex : NULL)
    {
        if (m_pMutex) m_pMutex->LockWrite();
    }
    ~MutexLockWrite() { if (m_pMutex) m_pMutex->UnlockWrite(); }

private:
    MA_RW_MUTEX* m_pMutex;
};

#if MA_DEBUG_GLOBAL_MUTEX
static MA_MUTEX g_DebugGlobalMutex;
#define MA_DEBUG_GLOBAL_MUTEX_LOCK MutexLock debugGlobalMutexLock(g_DebugGlobalMutex, true);
#else
#define MA_DEBUG_GLOBAL_MUTEX_LOCK
#endif
#endif // _MA_MUTEX

#ifndef _MA_VECTOR
/*
Dynamically resizing continuous array. Class with interface similar to std::vector.
T must be POD because constructors and destructors are not called and memcpy is
used for these objects.
*/
template<typename T>
class Vector
{
public:
    using value_type = T;
    using iterator = T*;
    using const_iterator = const T*;

    // allocationCallbacks externally owned, must outlive this object.
    Vector(const ALLOCATION_CALLBACKS& allocationCallbacks);
    Vector(size_t count, const ALLOCATION_CALLBACKS& allocationCallbacks);
    Vector(const Vector<T>& src);
    ~Vector();

    const ALLOCATION_CALLBACKS& GetAllocs() const { return m_AllocationCallbacks; }
    bool empty() const { return m_Count == 0; }
    size_t size() const { return m_Count; }
    T* data() { return m_pArray; }
    const T* data() const { return m_pArray; }
    void clear(bool freeMemory = false) { resize(0, freeMemory); }

    iterator begin() { return m_pArray; }
    iterator end() { return m_pArray + m_Count; }
    const_iterator cbegin() const { return m_pArray; }
    const_iterator cend() const { return m_pArray + m_Count; }
    const_iterator begin() const { return cbegin(); }
    const_iterator end() const { return cend(); }

    void push_front(const T& src) { insert(0, src); }
    void push_back(const T& src);
    void pop_front();
    void pop_back();

    T& front();
    T& back();
    const T& front() const;
    const T& back() const;

    void reserve(size_t newCapacity, bool freeMemory = false);
    void resize(size_t newCount, bool freeMemory = false);
    void insert(size_t index, const T& src);
    void remove(size_t index);

    template<typename CmpLess>
    size_t InsertSorted(const T& value, const CmpLess& cmp);
    template<typename CmpLess>
    bool RemoveSorted(const T& value, const CmpLess& cmp);

    Vector& operator=(const Vector<T>& rhs);
    T& operator[](size_t index);
    const T& operator[](size_t index) const;

private:
    const ALLOCATION_CALLBACKS& m_AllocationCallbacks;
    T* m_pArray;
    size_t m_Count;
    size_t m_Capacity;
};

#ifndef _MA_VECTOR_FUNCTIONS
template<typename T>
Vector<T>::Vector(const ALLOCATION_CALLBACKS& allocationCallbacks)
    : m_AllocationCallbacks(allocationCallbacks),
    m_pArray(NULL),
    m_Count(0),
    m_Capacity(0) {}

template<typename T>
Vector<T>::Vector(size_t count, const ALLOCATION_CALLBACKS& allocationCallbacks)
    : m_AllocationCallbacks(allocationCallbacks),
    m_pArray(count ? AllocateArray<T>(allocationCallbacks, count) : NULL),
    m_Count(count),
    m_Capacity(count) {}

template<typename T>
Vector<T>::Vector(const Vector<T>& src)
    : m_AllocationCallbacks(src.m_AllocationCallbacks),
    m_pArray(src.m_Count ? AllocateArray<T>(src.m_AllocationCallbacks, src.m_Count) : NULL),
    m_Count(src.m_Count),
    m_Capacity(src.m_Count)
{
    if (m_Count > 0)
    {
        memcpy(m_pArray, src.m_pArray, m_Count * sizeof(T));
    }
}

template<typename T>
Vector<T>::~Vector()
{
    Free(m_AllocationCallbacks, m_pArray);
}

template<typename T>
void Vector<T>::push_back(const T& src)
{
    const size_t newIndex = size();
    resize(newIndex + 1);
    m_pArray[newIndex] = src;
}

template<typename T>
void Vector<T>::pop_front()
{
    MA_HEAVY_ASSERT(m_Count > 0);
    remove(0);
}

template<typename T>
void Vector<T>::pop_back()
{
    MA_HEAVY_ASSERT(m_Count > 0);
    resize(size() - 1);
}

template<typename T>
T& Vector<T>::front()
{
    MA_HEAVY_ASSERT(m_Count > 0);
    return m_pArray[0];
}

template<typename T>
T& Vector<T>::back()
{
    MA_HEAVY_ASSERT(m_Count > 0);
    return m_pArray[m_Count - 1];
}

template<typename T>
const T& Vector<T>::front() const
{
    MA_HEAVY_ASSERT(m_Count > 0);
    return m_pArray[0];
}

template<typename T>
const T& Vector<T>::back() const
{
    MA_HEAVY_ASSERT(m_Count > 0);
    return m_pArray[m_Count - 1];
}

template<typename T>
void Vector<T>::reserve(size_t newCapacity, bool freeMemory)
{
    newCapacity = MA_MAX(newCapacity, m_Count);

    if ((newCapacity < m_Capacity) && !freeMemory)
    {
        newCapacity = m_Capacity;
    }

    if (newCapacity != m_Capacity)
    {
        T* const newArray = newCapacity ? AllocateArray<T>(m_AllocationCallbacks, newCapacity) : NULL;
        if (m_Count != 0)
        {
            memcpy(newArray, m_pArray, m_Count * sizeof(T));
        }
        Free(m_AllocationCallbacks, m_pArray);
        m_Capacity = newCapacity;
        m_pArray = newArray;
    }
}

template<typename T>
void Vector<T>::resize(size_t newCount, bool freeMemory)
{
    size_t newCapacity = m_Capacity;
    if (newCount > m_Capacity)
    {
        newCapacity = MA_MAX(newCount, MA_MAX(m_Capacity * 3 / 2, (size_t)8));
    }
    else if (freeMemory)
    {
        newCapacity = newCount;
    }

    if (newCapacity != m_Capacity)
    {
        T* const newArray = newCapacity ? AllocateArray<T>(m_AllocationCallbacks, newCapacity) : NULL;
        const size_t elementsToCopy = MA_MIN(m_Count, newCount);
        if (elementsToCopy != 0)
        {
            memcpy(newArray, m_pArray, elementsToCopy * sizeof(T));
        }
        Free(m_AllocationCallbacks, m_pArray);
        m_Capacity = newCapacity;
        m_pArray = newArray;
    }

    m_Count = newCount;
}

template<typename T>
void Vector<T>::insert(size_t index, const T& src)
{
    MA_HEAVY_ASSERT(index <= m_Count);
    const size_t oldCount = size();
    resize(oldCount + 1);
    if (index < oldCount)
    {
        memmove(m_pArray + (index + 1), m_pArray + index, (oldCount - index) * sizeof(T));
    }
    m_pArray[index] = src;
}

template<typename T>
void Vector<T>::remove(size_t index)
{
    MA_HEAVY_ASSERT(index < m_Count);
    const size_t oldCount = size();
    if (index < oldCount - 1)
    {
        memmove(m_pArray + index, m_pArray + (index + 1), (oldCount - index - 1) * sizeof(T));
    }
    resize(oldCount - 1);
}

template<typename T> template<typename CmpLess>
size_t Vector<T>::InsertSorted(const T& value, const CmpLess& cmp)
{
    const size_t indexToInsert = BinaryFindFirstNotLess<CmpLess, iterator, T>(
        m_pArray,
        m_pArray + m_Count,
        value,
        cmp) - m_pArray;
    insert(indexToInsert, value);
    return indexToInsert;
}

template<typename T> template<typename CmpLess>
bool Vector<T>::RemoveSorted(const T& value, const CmpLess& cmp)
{
    const iterator it = BinaryFindFirstNotLess(
        m_pArray,
        m_pArray + m_Count,
        value,
        cmp);
    if ((it != end()) && !cmp(*it, value) && !cmp(value, *it))
    {
        size_t indexToRemove = it - begin();
        remove(indexToRemove);
        return true;
    }
    return false;
}

template<typename T>
Vector<T>& Vector<T>::operator=(const Vector<T>& rhs)
{
    if (&rhs != this)
    {
        resize(rhs.m_Count);
        if (m_Count != 0)
        {
            memcpy(m_pArray, rhs.m_pArray, m_Count * sizeof(T));
        }
    }
    return *this;
}

template<typename T>
T& Vector<T>::operator[](size_t index)
{
    MA_HEAVY_ASSERT(index < m_Count);
    return m_pArray[index];
}

template<typename T>
const T& Vector<T>::operator[](size_t index) const
{
    MA_HEAVY_ASSERT(index < m_Count);
    return m_pArray[index];
}
#endif // _MA_VECTOR_FUNCTIONS
#endif // _MA_VECTOR

#ifndef _MA_STRING_BUILDER
class StringBuilder
{

public:
    StringBuilder(const ALLOCATION_CALLBACKS& allocationCallbacks) : m_Data(allocationCallbacks) {}

    size_t GetLength() const { return m_Data.size(); }
    const wchar_t* GetData() const { return m_Data.data(); }

    void Add(wchar_t ch) { m_Data.push_back(ch); }
    void Add(const wchar_t* str);
    void AddNewLine() { Add(L'\n'); }
    void AddNumber(uint32_t num);
    void AddNumber(uint64_t num);
    void AddPointer(const void* ptr);

private:
    Vector<wchar_t> m_Data;

};

#ifndef _MA_STRING_BUILDER_FUNCTIONS
void StringBuilder::Add(const wchar_t* str)
{
    const size_t len = wcslen(str);
    if (len > 0)
    {
        const size_t oldCount = m_Data.size();
        m_Data.resize(oldCount + len);
        memcpy(m_Data.data() + oldCount, str, len * sizeof(wchar_t));
    }
}

void StringBuilder::AddNumber(uint32_t num)
{
    wchar_t buf[11];
    buf[10] = L'\0';
    wchar_t* p = &buf[10];
    do
    {
        *--p = L'0' + (num % 10);
        num /= 10;
    } while (num);
    Add(p);
}

void StringBuilder::AddNumber(uint64_t num)
{
    wchar_t buf[21];
    buf[20] = L'\0';
    wchar_t* p = &buf[20];
    do
    {
        *--p = L'0' + (num % 10);
        num /= 10;
    } while (num);
    Add(p);
}

void StringBuilder::AddPointer(const void* ptr)
{
    wchar_t buf[21];
    uintptr_t num = (uintptr_t)ptr;
    buf[20] = L'\0';
    wchar_t* p = &buf[20];
    do
    {
        *--p = HexDigitToChar((uint8_t)(num & 0xF));
        num >>= 4;
    } while (num);
    Add(p);
}

#endif // _MA_STRING_BUILDER_FUNCTIONS
#endif // _MA_STRING_BUILDER

#ifndef _MA_POOL_ALLOCATOR
/*
Allocator for objects of type T using a list of arrays (pools) to speed up
allocation. Number of elements that can be allocated is not bounded because
allocator can create multiple blocks.
T should be POD because constructor and destructor is not called in Alloc or
Free.
*/
template<typename T>
class PoolAllocator
{
    MA_CLASS_NO_COPY(PoolAllocator)
public:
    // allocationCallbacks externally owned, must outlive this object.
    PoolAllocator(const ALLOCATION_CALLBACKS& allocationCallbacks, uint32_t firstBlockCapacity);
    ~PoolAllocator() { Clear(); }

    void Clear();
    template<typename... Types>
    T* Alloc(Types... args);
    void Free(T* ptr);

private:
    union Item
    {
        uint32_t NextFreeIndex; // UINT32_MAX means end of list.
        alignas(T) char Value[sizeof(T)];
    };

    struct ItemBlock
    {
        Item* pItems;
        uint32_t Capacity;
        uint32_t FirstFreeIndex;
    };

    const ALLOCATION_CALLBACKS& m_AllocationCallbacks;
    const uint32_t m_FirstBlockCapacity;
    Vector<ItemBlock> m_ItemBlocks;

    ItemBlock& CreateNewBlock();
};

#ifndef _MA_POOL_ALLOCATOR_FUNCTIONS
template<typename T>
PoolAllocator<T>::PoolAllocator(const ALLOCATION_CALLBACKS& allocationCallbacks, uint32_t firstBlockCapacity)
    : m_AllocationCallbacks(allocationCallbacks),
    m_FirstBlockCapacity(firstBlockCapacity),
    m_ItemBlocks(allocationCallbacks)
{
    SKTBD_ASSERT(m_FirstBlockCapacity > 1);
}

template<typename T>
void PoolAllocator<T>::Clear()
{
    for (size_t i = m_ItemBlocks.size(); i--; )
    {
        MA_DELETE_ARRAY(m_AllocationCallbacks, m_ItemBlocks[i].pItems, m_ItemBlocks[i].Capacity);
    }
    m_ItemBlocks.clear(true);
}

template<typename T> template<typename... Types>
T* PoolAllocator<T>::Alloc(Types... args)
{
    for (size_t i = m_ItemBlocks.size(); i--; )
    {
        ItemBlock& block = m_ItemBlocks[i];
        // This block has some free items: Use first one.
        if (block.FirstFreeIndex != UINT32_MAX)
        {
            Item* const pItem = &block.pItems[block.FirstFreeIndex];
            block.FirstFreeIndex = pItem->NextFreeIndex;
            T* result = (T*)&pItem->Value;
            new(result)T(std::forward<Types>(args)...); // Explicit constructor call.
            return result;
        }
    }

    // No block has free item: Create new one and use it.
    ItemBlock& newBlock = CreateNewBlock();
    Item* const pItem = &newBlock.pItems[0];
    newBlock.FirstFreeIndex = pItem->NextFreeIndex;
    T* result = (T*)pItem->Value;
    new(result)T(std::forward<Types>(args)...); // Explicit constructor call.
    return result;
}

template<typename T>
void PoolAllocator<T>::Free(T* ptr)
{
    // Search all memory blocks to find ptr.
    for (size_t i = m_ItemBlocks.size(); i--; )
    {
        ItemBlock& block = m_ItemBlocks[i];

        Item* pItemPtr;
        memcpy(&pItemPtr, &ptr, sizeof(pItemPtr));

        // Check if pItemPtr is in address range of this block.
        if ((pItemPtr >= block.pItems) && (pItemPtr < block.pItems + block.Capacity))
        {
            ptr->~T(); // Explicit destructor call.
            const uint32_t index = static_cast<uint32_t>(pItemPtr - block.pItems);
            pItemPtr->NextFreeIndex = block.FirstFreeIndex;
            block.FirstFreeIndex = index;
            return;
        }
    }
    SKTBD_ASSERT(0 , "Pointer doesn't belong to this memory pool.");
}

template<typename T>
typename PoolAllocator<T>::ItemBlock& PoolAllocator<T>::CreateNewBlock()
{
    const uint32_t newBlockCapacity = m_ItemBlocks.empty() ?
        m_FirstBlockCapacity : m_ItemBlocks.back().Capacity * 3 / 2;

    const ItemBlock newBlock = {
        MA_NEW_ARRAY(m_AllocationCallbacks, Item, newBlockCapacity),
        newBlockCapacity,
        0 };

    m_ItemBlocks.push_back(newBlock);

    // Setup singly-linked list of all free items in this block.
    for (uint32_t i = 0; i < newBlockCapacity - 1; ++i)
    {
        newBlock.pItems[i].NextFreeIndex = i + 1;
    }
    newBlock.pItems[newBlockCapacity - 1].NextFreeIndex = UINT32_MAX;
    return m_ItemBlocks.back();
}
#endif // _MA_POOL_ALLOCATOR_FUNCTIONS
#endif // _MA_POOL_ALLOCATOR

#ifndef _MA_LIST
/*
Doubly linked list, with elements allocated out of PoolAllocator.
Has custom interface, as well as STL-style interface, including iterator and
const_iterator.
*/
template<typename T>
class List
{
    MA_CLASS_NO_COPY(List)
public:
    struct Item
    {
        Item* pPrev;
        Item* pNext;
        T Value;
    };

    class reverse_iterator;
    class const_reverse_iterator;
    class iterator
    {
        friend class List<T>;
        friend class const_iterator;

    public:
        iterator() = default;
        iterator(const reverse_iterator& src)
            : m_pList(src.m_pList), m_pItem(src.m_pItem) {}

        T& operator*() const;
        T* operator->() const;

        iterator& operator++();
        iterator& operator--();
        iterator operator++(int);
        iterator operator--(int);

        bool operator==(const iterator& rhs) const;
        bool operator!=(const iterator& rhs) const;

    private:
        List<T>* m_pList = NULL;
        Item* m_pItem = NULL;

        iterator(List<T>* pList, Item* pItem) : m_pList(pList), m_pItem(pItem) {}
    };

    class reverse_iterator
    {
        friend class List<T>;
        friend class const_reverse_iterator;

    public:
        reverse_iterator() = default;
        reverse_iterator(const iterator& src)
            : m_pList(src.m_pList), m_pItem(src.m_pItem) {}

        T& operator*() const;
        T* operator->() const;

        reverse_iterator& operator++();
        reverse_iterator& operator--();
        reverse_iterator operator++(int);
        reverse_iterator operator--(int);

        bool operator==(const reverse_iterator& rhs) const;
        bool operator!=(const reverse_iterator& rhs) const;

    private:
        List<T>* m_pList = NULL;
        Item* m_pItem = NULL;

        reverse_iterator(List<T>* pList, Item* pItem)
            : m_pList(pList), m_pItem(pItem) {}
    };

    class const_iterator
    {
        friend class List<T>;

    public:
        const_iterator() = default;
        const_iterator(const iterator& src)
            : m_pList(src.m_pList), m_pItem(src.m_pItem) {}
        const_iterator(const reverse_iterator& src)
            : m_pList(src.m_pList), m_pItem(src.m_pItem) {}
        const_iterator(const const_reverse_iterator& src)
            : m_pList(src.m_pList), m_pItem(src.m_pItem) {}

        iterator dropConst() const;
        const T& operator*() const;
        const T* operator->() const;

        const_iterator& operator++();
        const_iterator& operator--();
        const_iterator operator++(int);
        const_iterator operator--(int);

        bool operator==(const const_iterator& rhs) const;
        bool operator!=(const const_iterator& rhs) const;

    private:
        const List<T>* m_pList = NULL;
        const Item* m_pItem = NULL;

        const_iterator(const List<T>* pList, const Item* pItem)
            : m_pList(pList), m_pItem(pItem) {}
    };

    class const_reverse_iterator
    {
        friend class List<T>;

    public:
        const_reverse_iterator() = default;
        const_reverse_iterator(const iterator& src)
            : m_pList(src.m_pList), m_pItem(src.m_pItem) {}
        const_reverse_iterator(const reverse_iterator& src)
            : m_pList(src.m_pList), m_pItem(src.m_pItem) {}
        const_reverse_iterator(const const_iterator& src)
            : m_pList(src.m_pList), m_pItem(src.m_pItem) {}

        reverse_iterator dropConst() const;
        const T& operator*() const;
        const T* operator->() const;

        const_reverse_iterator& operator++();
        const_reverse_iterator& operator--();
        const_reverse_iterator operator++(int);
        const_reverse_iterator operator--(int);

        bool operator==(const const_reverse_iterator& rhs) const;
        bool operator!=(const const_reverse_iterator& rhs) const;

    private:
        const List<T>* m_pList = NULL;
        const Item* m_pItem = NULL;

        const_reverse_iterator(const List<T>* pList, const Item* pItem)
            : m_pList(pList), m_pItem(pItem) {}
    };

    // allocationCallbacks externally owned, must outlive this object.
    List(const ALLOCATION_CALLBACKS& allocationCallbacks);
    // Intentionally not calling Clear, because that would be unnecessary
    // computations to return all items to m_ItemAllocator as free.
    ~List() = default;

    size_t GetCount() const { return m_Count; }
    bool IsEmpty() const { return m_Count == 0; }

    Item* Front() { return m_pFront; }
    const Item* Front() const { return m_pFront; }
    Item* Back() { return m_pBack; }
    const Item* Back() const { return m_pBack; }

    bool empty() const { return IsEmpty(); }
    size_t size() const { return GetCount(); }
    void push_back(const T& value) { PushBack(value); }
    iterator insert(iterator it, const T& value) { return iterator(this, InsertBefore(it.m_pItem, value)); }
    void clear() { Clear(); }
    void erase(iterator it) { Remove(it.m_pItem); }

    iterator begin() { return iterator(this, Front()); }
    iterator end() { return iterator(this, NULL); }
    reverse_iterator rbegin() { return reverse_iterator(this, Back()); }
    reverse_iterator rend() { return reverse_iterator(this, NULL); }

    const_iterator cbegin() const { return const_iterator(this, Front()); }
    const_iterator cend() const { return const_iterator(this, NULL); }
    const_iterator begin() const { return cbegin(); }
    const_iterator end() const { return cend(); }

    const_reverse_iterator crbegin() const { return const_reverse_iterator(this, Back()); }
    const_reverse_iterator crend() const { return const_reverse_iterator(this, NULL); }
    const_reverse_iterator rbegin() const { return crbegin(); }
    const_reverse_iterator rend() const { return crend(); }

    Item* PushBack();
    Item* PushFront();
    Item* PushBack(const T& value);
    Item* PushFront(const T& value);
    void PopBack();
    void PopFront();

    // Item can be null - it means PushBack.
    Item* InsertBefore(Item* pItem);
    // Item can be null - it means PushFront.
    Item* InsertAfter(Item* pItem);
    Item* InsertBefore(Item* pItem, const T& value);
    Item* InsertAfter(Item* pItem, const T& value);

    void Clear();
    void Remove(Item* pItem);

private:
    const ALLOCATION_CALLBACKS& m_AllocationCallbacks;
    PoolAllocator<Item> m_ItemAllocator;
    Item* m_pFront;
    Item* m_pBack;
    size_t m_Count;
};

#ifndef _MA_LIST_ITERATOR_FUNCTIONS
template<typename T>
T& List<T>::iterator::operator*() const
{
    MA_HEAVY_ASSERT(m_pItem != NULL);
    return m_pItem->Value;
}

template<typename T>
T* List<T>::iterator::operator->() const
{
    MA_HEAVY_ASSERT(m_pItem != NULL);
    return &m_pItem->Value;
}

template<typename T>
typename List<T>::iterator& List<T>::iterator::operator++()
{
    MA_HEAVY_ASSERT(m_pItem != NULL);
    m_pItem = m_pItem->pNext;
    return *this;
}

template<typename T>
typename List<T>::iterator& List<T>::iterator::operator--()
{
    if (m_pItem != NULL)
    {
        m_pItem = m_pItem->pPrev;
    }
    else
    {
        MA_HEAVY_ASSERT(!m_pList->IsEmpty());
        m_pItem = m_pList->Back();
    }
    return *this;
}

template<typename T>
typename List<T>::iterator List<T>::iterator::operator++(int)
{
    iterator result = *this;
    ++*this;
    return result;
}

template<typename T>
typename List<T>::iterator List<T>::iterator::operator--(int)
{
    iterator result = *this;
    --*this;
    return result;
}

template<typename T>
bool List<T>::iterator::operator==(const iterator& rhs) const
{
    MA_HEAVY_ASSERT(m_pList == rhs.m_pList);
    return m_pItem == rhs.m_pItem;
}

template<typename T>
bool List<T>::iterator::operator!=(const iterator& rhs) const
{
    MA_HEAVY_ASSERT(m_pList == rhs.m_pList);
    return m_pItem != rhs.m_pItem;
}
#endif // _MA_LIST_ITERATOR_FUNCTIONS

#ifndef _MA_LIST_REVERSE_ITERATOR_FUNCTIONS
template<typename T>
T& List<T>::reverse_iterator::operator*() const
{
    MA_HEAVY_ASSERT(m_pItem != NULL);
    return m_pItem->Value;
}

template<typename T>
T* List<T>::reverse_iterator::operator->() const
{
    MA_HEAVY_ASSERT(m_pItem != NULL);
    return &m_pItem->Value;
}

template<typename T>
typename List<T>::reverse_iterator& List<T>::reverse_iterator::operator++()
{
    MA_HEAVY_ASSERT(m_pItem != NULL);
    m_pItem = m_pItem->pPrev;
    return *this;
}

template<typename T>
typename List<T>::reverse_iterator& List<T>::reverse_iterator::operator--()
{
    if (m_pItem != NULL)
    {
        m_pItem = m_pItem->pNext;
    }
    else
    {
        MA_HEAVY_ASSERT(!m_pList->IsEmpty());
        m_pItem = m_pList->Front();
    }
    return *this;
}

template<typename T>
typename List<T>::reverse_iterator List<T>::reverse_iterator::operator++(int)
{
    reverse_iterator result = *this;
    ++*this;
    return result;
}

template<typename T>
typename List<T>::reverse_iterator List<T>::reverse_iterator::operator--(int)
{
    reverse_iterator result = *this;
    --*this;
    return result;
}

template<typename T>
bool List<T>::reverse_iterator::operator==(const reverse_iterator& rhs) const
{
    MA_HEAVY_ASSERT(m_pList == rhs.m_pList);
    return m_pItem == rhs.m_pItem;
}

template<typename T>
bool List<T>::reverse_iterator::operator!=(const reverse_iterator& rhs) const
{
    MA_HEAVY_ASSERT(m_pList == rhs.m_pList);
    return m_pItem != rhs.m_pItem;
}
#endif // _MA_LIST_REVERSE_ITERATOR_FUNCTIONS

#ifndef _MA_LIST_CONST_ITERATOR_FUNCTIONS
template<typename T>
typename List<T>::iterator List<T>::const_iterator::dropConst() const
{
    return iterator(const_cast<List<T>*>(m_pList), const_cast<Item*>(m_pItem));
}

template<typename T>
const T& List<T>::const_iterator::operator*() const
{
    MA_HEAVY_ASSERT(m_pItem != NULL);
    return m_pItem->Value;
}

template<typename T>
const T* List<T>::const_iterator::operator->() const
{
    MA_HEAVY_ASSERT(m_pItem != NULL);
    return &m_pItem->Value;
}

template<typename T>
typename List<T>::const_iterator& List<T>::const_iterator::operator++()
{
    MA_HEAVY_ASSERT(m_pItem != NULL);
    m_pItem = m_pItem->pNext;
    return *this;
}

template<typename T>
typename List<T>::const_iterator& List<T>::const_iterator::operator--()
{
    if (m_pItem != NULL)
    {
        m_pItem = m_pItem->pPrev;
    }
    else
    {
        MA_HEAVY_ASSERT(!m_pList->IsEmpty());
        m_pItem = m_pList->Back();
    }
    return *this;
}

template<typename T>
typename List<T>::const_iterator List<T>::const_iterator::operator++(int)
{
    const_iterator result = *this;
    ++*this;
    return result;
}

template<typename T>
typename List<T>::const_iterator List<T>::const_iterator::operator--(int)
{
    const_iterator result = *this;
    --*this;
    return result;
}

template<typename T>
bool List<T>::const_iterator::operator==(const const_iterator& rhs) const
{
    MA_HEAVY_ASSERT(m_pList == rhs.m_pList);
    return m_pItem == rhs.m_pItem;
}

template<typename T>
bool List<T>::const_iterator::operator!=(const const_iterator& rhs) const
{
    MA_HEAVY_ASSERT(m_pList == rhs.m_pList);
    return m_pItem != rhs.m_pItem;
}
#endif // _MA_LIST_CONST_ITERATOR_FUNCTIONS

#ifndef _MA_LIST_CONST_REVERSE_ITERATOR_FUNCTIONS
template<typename T>
typename List<T>::reverse_iterator List<T>::const_reverse_iterator::dropConst() const
{
    return reverse_iterator(const_cast<List<T>*>(m_pList), const_cast<Item*>(m_pItem));
}

template<typename T>
const T& List<T>::const_reverse_iterator::operator*() const
{
    MA_HEAVY_ASSERT(m_pItem != NULL);
    return m_pItem->Value;
}

template<typename T>
const T* List<T>::const_reverse_iterator::operator->() const
{
    MA_HEAVY_ASSERT(m_pItem != NULL);
    return &m_pItem->Value;
}

template<typename T>
typename List<T>::const_reverse_iterator& List<T>::const_reverse_iterator::operator++()
{
    MA_HEAVY_ASSERT(m_pItem != NULL);
    m_pItem = m_pItem->pPrev;
    return *this;
}

template<typename T>
typename List<T>::const_reverse_iterator& List<T>::const_reverse_iterator::operator--()
{
    if (m_pItem != NULL)
    {
        m_pItem = m_pItem->pNext;
    }
    else
    {
        MA_HEAVY_ASSERT(!m_pList->IsEmpty());
        m_pItem = m_pList->Front();
    }
    return *this;
}

template<typename T>
typename List<T>::const_reverse_iterator List<T>::const_reverse_iterator::operator++(int)
{
    const_reverse_iterator result = *this;
    ++*this;
    return result;
}

template<typename T>
typename List<T>::const_reverse_iterator List<T>::const_reverse_iterator::operator--(int)
{
    const_reverse_iterator result = *this;
    --*this;
    return result;
}

template<typename T>
bool List<T>::const_reverse_iterator::operator==(const const_reverse_iterator& rhs) const
{
    MA_HEAVY_ASSERT(m_pList == rhs.m_pList);
    return m_pItem == rhs.m_pItem;
}

template<typename T>
bool List<T>::const_reverse_iterator::operator!=(const const_reverse_iterator& rhs) const
{
    MA_HEAVY_ASSERT(m_pList == rhs.m_pList);
    return m_pItem != rhs.m_pItem;
}
#endif // _MA_LIST_CONST_REVERSE_ITERATOR_FUNCTIONS

#ifndef _MA_LIST_FUNCTIONS
template<typename T>
List<T>::List(const ALLOCATION_CALLBACKS& allocationCallbacks)
    : m_AllocationCallbacks(allocationCallbacks),
    m_ItemAllocator(allocationCallbacks, 128),
    m_pFront(NULL),
    m_pBack(NULL),
    m_Count(0) {}

template<typename T>
void List<T>::Clear()
{
    if (!IsEmpty())
    {
        Item* pItem = m_pBack;
        while (pItem != NULL)
        {
            Item* const pPrevItem = pItem->pPrev;
            m_ItemAllocator.Free(pItem);
            pItem = pPrevItem;
        }
        m_pFront = NULL;
        m_pBack = NULL;
        m_Count = 0;
    }
}

template<typename T>
typename List<T>::Item* List<T>::PushBack()
{
    Item* const pNewItem = m_ItemAllocator.Alloc();
    pNewItem->pNext = NULL;
    if (IsEmpty())
    {
        pNewItem->pPrev = NULL;
        m_pFront = pNewItem;
        m_pBack = pNewItem;
        m_Count = 1;
    }
    else
    {
        pNewItem->pPrev = m_pBack;
        m_pBack->pNext = pNewItem;
        m_pBack = pNewItem;
        ++m_Count;
    }
    return pNewItem;
}

template<typename T>
typename List<T>::Item* List<T>::PushFront()
{
    Item* const pNewItem = m_ItemAllocator.Alloc();
    pNewItem->pPrev = NULL;
    if (IsEmpty())
    {
        pNewItem->pNext = NULL;
        m_pFront = pNewItem;
        m_pBack = pNewItem;
        m_Count = 1;
    }
    else
    {
        pNewItem->pNext = m_pFront;
        m_pFront->pPrev = pNewItem;
        m_pFront = pNewItem;
        ++m_Count;
    }
    return pNewItem;
}

template<typename T>
typename List<T>::Item* List<T>::PushBack(const T& value)
{
    Item* const pNewItem = PushBack();
    pNewItem->Value = value;
    return pNewItem;
}

template<typename T>
typename List<T>::Item* List<T>::PushFront(const T& value)
{
    Item* const pNewItem = PushFront();
    pNewItem->Value = value;
    return pNewItem;
}

template<typename T>
void List<T>::PopBack()
{
    MA_HEAVY_ASSERT(m_Count > 0);
    Item* const pBackItem = m_pBack;
    Item* const pPrevItem = pBackItem->pPrev;
    if (pPrevItem != NULL)
    {
        pPrevItem->pNext = NULL;
    }
    m_pBack = pPrevItem;
    m_ItemAllocator.Free(pBackItem);
    --m_Count;
}

template<typename T>
void List<T>::PopFront()
{
    MA_HEAVY_ASSERT(m_Count > 0);
    Item* const pFrontItem = m_pFront;
    Item* const pNextItem = pFrontItem->pNext;
    if (pNextItem != NULL)
    {
        pNextItem->pPrev = NULL;
    }
    m_pFront = pNextItem;
    m_ItemAllocator.Free(pFrontItem);
    --m_Count;
}

template<typename T>
void List<T>::Remove(Item* pItem)
{
    MA_HEAVY_ASSERT(pItem != NULL);
    MA_HEAVY_ASSERT(m_Count > 0);

    if (pItem->pPrev != NULL)
    {
        pItem->pPrev->pNext = pItem->pNext;
    }
    else
    {
        MA_HEAVY_ASSERT(m_pFront == pItem);
        m_pFront = pItem->pNext;
    }

    if (pItem->pNext != NULL)
    {
        pItem->pNext->pPrev = pItem->pPrev;
    }
    else
    {
        MA_HEAVY_ASSERT(m_pBack == pItem);
        m_pBack = pItem->pPrev;
    }

    m_ItemAllocator.Free(pItem);
    --m_Count;
}

template<typename T>
typename List<T>::Item* List<T>::InsertBefore(Item* pItem)
{
    if (pItem != NULL)
    {
        Item* const prevItem = pItem->pPrev;
        Item* const newItem = m_ItemAllocator.Alloc();
        newItem->pPrev = prevItem;
        newItem->pNext = pItem;
        pItem->pPrev = newItem;
        if (prevItem != NULL)
        {
            prevItem->pNext = newItem;
        }
        else
        {
            MA_HEAVY_ASSERT(m_pFront == pItem);
            m_pFront = newItem;
        }
        ++m_Count;
        return newItem;
    }
    else
    {
        return PushBack();
    }
}

template<typename T>
typename List<T>::Item* List<T>::InsertAfter(Item* pItem)
{
    if (pItem != NULL)
    {
        Item* const nextItem = pItem->pNext;
        Item* const newItem = m_ItemAllocator.Alloc();
        newItem->pNext = nextItem;
        newItem->pPrev = pItem;
        pItem->pNext = newItem;
        if (nextItem != NULL)
        {
            nextItem->pPrev = newItem;
        }
        else
        {
            MA_HEAVY_ASSERT(m_pBack == pItem);
            m_pBack = newItem;
        }
        ++m_Count;
        return newItem;
    }
    else
        return PushFront();
}

template<typename T>
typename List<T>::Item* List<T>::InsertBefore(Item* pItem, const T& value)
{
    Item* const newItem = InsertBefore(pItem);
    newItem->Value = value;
    return newItem;
}

template<typename T>
typename List<T>::Item* List<T>::InsertAfter(Item* pItem, const T& value)
{
    Item* const newItem = InsertAfter(pItem);
    newItem->Value = value;
    return newItem;
}
#endif // _MA_LIST_FUNCTIONS
#endif // _MA_LIST

#ifndef _MA_SUBALLOCATION
/*
Represents a region of NormalBlock that is either assigned and returned as
allocated memory block or free.
*/
struct Suballocation
{
    uint64_t offset;
    uint64_t size;
    void* privateData;
    SuballocationType type;
};
using SuballocationList = List<Suballocation>;

// Comparator for offsets.
struct SuballocationOffsetLess
{
    bool operator()(const Suballocation& lhs, const Suballocation& rhs) const
    {
        return lhs.offset < rhs.offset;
    }
};

struct SuballocationOffsetGreater
{
    bool operator()(const Suballocation& lhs, const Suballocation& rhs) const
    {
        return lhs.offset > rhs.offset;
    }
};

struct SuballocationItemSizeLess
{
    bool operator()(const SuballocationList::iterator lhs, const SuballocationList::iterator rhs) const
    {
        return lhs->size < rhs->size;
    }
    bool operator()(const SuballocationList::iterator lhs, uint64_t rhsSize) const
    {
        return lhs->size < rhsSize;
    }
};
#endif // _MA_SUBALLOCATION

#ifndef _MA_ALLOCATION_REQUEST
/*
Parameters of planned allocation inside a NormalBlock.
*/
struct AllocationRequest
{
    AllocHandle allocHandle;
    uint64_t size;
    uint64_t algorithmData;
    uint64_t sumFreeSize; // Sum size of free items that overlap with proposed allocation.
    uint64_t sumItemSize; // Sum size of items to make lost that overlap with proposed allocation.
    SuballocationList::iterator item;
};
#endif // _MAs_ALLOCATION_REQUEST

#ifndef _MA_BLOCK_METADATA
/*
Data structure used for bookkeeping of allocations and unused ranges of memory
in a single ID3D12Heap memory block.
*/
class BlockMetadata
{
public:
    BlockMetadata(const ALLOCATION_CALLBACKS* allocationCallbacks, bool isVirtual);
    virtual ~BlockMetadata() = default;

    virtual void Init(uint64_t size) { m_Size = size; }
    // Validates all data structures inside this object. If not valid, returns false.
    virtual bool Validate() const = 0;
    uint64_t GetSize() const { return m_Size; }
    bool IsVirtual() const { return m_IsVirtual; }
    virtual size_t GetAllocationCount() const = 0;
    virtual size_t GetFreeRegionsCount() const = 0;
    virtual uint64_t GetSumFreeSize() const = 0;
    virtual uint64_t GetAllocationOffset(AllocHandle allocHandle) const = 0;
    // Returns true if this block is empty - contains only single free suballocation.
    virtual bool IsEmpty() const = 0;

    virtual void GetAllocationInfo(AllocHandle allocHandle, VIRTUAL_ALLOCATION_INFO& outInfo) const = 0;

    // Tries to find a place for suballocation with given parameters inside this block.
    // If succeeded, fills pAllocationRequest and returns true.
    // If failed, returns false.
    virtual bool CreateAllocationRequest(
        uint64_t allocSize,
        uint64_t allocAlignment,
        bool upperAddress,
        uint32_t strategy,
        AllocationRequest* pAllocationRequest) = 0;

    // Makes actual allocation based on request. Request must already be checked and valid.
    virtual void Alloc(
        const AllocationRequest& request,
        uint64_t allocSize,
        void* PrivateData) = 0;

    virtual void Free(AllocHandle allocHandle) = 0;
    // Frees all allocations.
    // Careful! Don't call it if there are Allocation objects owned by pPrivateData of of cleared allocations!
    virtual void Clear() = 0;

    virtual AllocHandle GetAllocationListBegin() const = 0;
    virtual AllocHandle GetNextAllocation(AllocHandle prevAlloc) const = 0;
    virtual uint64_t GetNextFreeRegionSize(AllocHandle alloc) const = 0;
    virtual void* GetAllocationPrivateData(AllocHandle allocHandle) const = 0;
    virtual void SetAllocationPrivateData(AllocHandle allocHandle, void* privateData) = 0;

    virtual void AddStatistics(Statistics& inoutStats) const = 0;
    virtual void AddDetailedStatistics(DetailedStatistics& inoutStats) const = 0;
    virtual void DebugLogAllAllocations() const = 0;

protected:
    const ALLOCATION_CALLBACKS* GetAllocs() const { return m_pAllocationCallbacks; }
    uint64_t GetDebugMargin() const { return IsVirtual() ? 0 : MA_DEBUG_MARGIN; }

    void DebugLogAllocation(uint64_t offset, uint64_t size, void* privateData) const;
private:
    uint64_t m_Size;
    bool m_IsVirtual;
    const ALLOCATION_CALLBACKS* m_pAllocationCallbacks;

    MA_CLASS_NO_COPY(BlockMetadata);
};

#ifndef _MA_BLOCK_METADATA_FUNCTIONS
BlockMetadata::BlockMetadata(const ALLOCATION_CALLBACKS* allocationCallbacks, bool isVirtual)
    : m_Size(0),
    m_IsVirtual(isVirtual),
    m_pAllocationCallbacks(allocationCallbacks)
{
    SKTBD_ASSERT(allocationCallbacks);
}

void BlockMetadata::DebugLogAllocation(uint64_t offset, uint64_t size, void* privateData) const
{
    if (IsVirtual())
    {
        MA_DEBUG_LOG(L"UNFREED VIRTUAL ALLOCATION; Offset: %llu; Size: %llu; PrivateData: %p", offset, size, privateData);
    }
}
#endif // _MA_BLOCK_METADATA_FUNCTIONS
#endif // _MA_BLOCK_METADATA

#ifndef _MA_BLOCK_METADATA_LINEAR
class BlockMetadata_Linear : public BlockMetadata
{
public:
    BlockMetadata_Linear(const ALLOCATION_CALLBACKS* allocationCallbacks, bool isVirtual);
    virtual ~BlockMetadata_Linear() = default;

    uint64_t GetSumFreeSize() const override { return m_SumFreeSize; }
    bool IsEmpty() const override { return GetAllocationCount() == 0; }
    uint64_t GetAllocationOffset(AllocHandle allocHandle) const override { return (uint64_t)allocHandle - 1; };

    void Init(uint64_t size) override;
    bool Validate() const override;
    size_t GetAllocationCount() const override;
    size_t GetFreeRegionsCount() const override;
    void GetAllocationInfo(AllocHandle allocHandle, VIRTUAL_ALLOCATION_INFO& outInfo) const override;

    bool CreateAllocationRequest(
        uint64_t allocSize,
        uint64_t allocAlignment,
        bool upperAddress,
        uint32_t strategy,
        AllocationRequest* pAllocationRequest) override;

    void Alloc(
        const AllocationRequest& request,
        uint64_t allocSize,
        void* privateData) override;

    void Free(AllocHandle allocHandle) override;
    void Clear() override;

    AllocHandle GetAllocationListBegin() const override;
    AllocHandle GetNextAllocation(AllocHandle prevAlloc) const override;
    uint64_t GetNextFreeRegionSize(AllocHandle alloc) const override;
    void* GetAllocationPrivateData(AllocHandle allocHandle) const override;
    void SetAllocationPrivateData(AllocHandle allocHandle, void* privateData) override;

    void AddStatistics(Statistics& inoutStats) const override;
    void AddDetailedStatistics(DetailedStatistics& inoutStats) const override;
    //void WriteAllocationInfoToJson(JsonWriter& json) const override;
    void DebugLogAllAllocations() const override;

private:
    /*
    There are two suballocation vectors, used in ping-pong way.
    The one with index m_1stVectorIndex is called 1st.
    The one with index (m_1stVectorIndex ^ 1) is called 2nd.
    2nd can be non-empty only when 1st is not empty.
    When 2nd is not empty, m_2ndVectorMode indicates its mode of operation.
    */
    typedef Vector<Suballocation> SuballocationVectorType;

    enum ALLOC_REQUEST_TYPE
    {
        ALLOC_REQUEST_UPPER_ADDRESS,
        ALLOC_REQUEST_END_OF_1ST,
        ALLOC_REQUEST_END_OF_2ND,
    };

    enum SECOND_VECTOR_MODE
    {
        SECOND_VECTOR_EMPTY,
        /*
        Suballocations in 2nd vector are created later than the ones in 1st, but they
        all have smaller offset.
        */
        SECOND_VECTOR_RING_BUFFER,
        /*
        Suballocations in 2nd vector are upper side of double stack.
        They all have offsets higher than those in 1st vector.
        Top of this stack means smaller offsets, but higher indices in this vector.
        */
        SECOND_VECTOR_DOUBLE_STACK,
    };

    uint64_t m_SumFreeSize;
    SuballocationVectorType m_Suballocations0, m_Suballocations1;
    uint32_t m_1stVectorIndex;
    SECOND_VECTOR_MODE m_2ndVectorMode;
    // Number of items in 1st vector with hAllocation = null at the beginning.
    size_t m_1stNullItemsBeginCount;
    // Number of other items in 1st vector with hAllocation = null somewhere in the middle.
    size_t m_1stNullItemsMiddleCount;
    // Number of items in 2nd vector with hAllocation = null.
    size_t m_2ndNullItemsCount;

    SuballocationVectorType& AccessSuballocations1st() { return m_1stVectorIndex ? m_Suballocations1 : m_Suballocations0; }
    SuballocationVectorType& AccessSuballocations2nd() { return m_1stVectorIndex ? m_Suballocations0 : m_Suballocations1; }
    const SuballocationVectorType& AccessSuballocations1st() const { return m_1stVectorIndex ? m_Suballocations1 : m_Suballocations0; }
    const SuballocationVectorType& AccessSuballocations2nd() const { return m_1stVectorIndex ? m_Suballocations0 : m_Suballocations1; }

    Suballocation& FindSuballocation(uint64_t offset) const;
    bool ShouldCompact1st() const;
    void CleanupAfterFree();

    bool CreateAllocationRequest_LowerAddress(
        uint64_t allocSize,
        uint64_t allocAlignment,
        AllocationRequest* pAllocationRequest);
    bool CreateAllocationRequest_UpperAddress(
        uint64_t allocSize,
        uint64_t allocAlignment,
        AllocationRequest* pAllocationRequest);

    MA_CLASS_NO_COPY(BlockMetadata_Linear)
};

#ifndef _MA_BLOCK_METADATA_LINEAR_FUNCTIONS
BlockMetadata_Linear::BlockMetadata_Linear(const ALLOCATION_CALLBACKS* allocationCallbacks, bool isVirtual)
    : BlockMetadata(allocationCallbacks, isVirtual),
    m_SumFreeSize(0),
    m_Suballocations0(*allocationCallbacks),
    m_Suballocations1(*allocationCallbacks),
    m_1stVectorIndex(0),
    m_2ndVectorMode(SECOND_VECTOR_EMPTY),
    m_1stNullItemsBeginCount(0),
    m_1stNullItemsMiddleCount(0),
    m_2ndNullItemsCount(0)
{
    SKTBD_ASSERT(allocationCallbacks);
}

void BlockMetadata_Linear::Init(uint64_t size)
{
    BlockMetadata::Init(size);
    m_SumFreeSize = size;
}

bool BlockMetadata_Linear::Validate() const
{
    MA_VALIDATE(GetSumFreeSize() <= GetSize());
    const SuballocationVectorType& suballocations1st = AccessSuballocations1st();
    const SuballocationVectorType& suballocations2nd = AccessSuballocations2nd();

    MA_VALIDATE(suballocations2nd.empty() == (m_2ndVectorMode == SECOND_VECTOR_EMPTY));
    MA_VALIDATE(!suballocations1st.empty() ||
        suballocations2nd.empty() ||
        m_2ndVectorMode != SECOND_VECTOR_RING_BUFFER);

    if (!suballocations1st.empty())
    {
        // Null item at the beginning should be accounted into m_1stNullItemsBeginCount.
        MA_VALIDATE(suballocations1st[m_1stNullItemsBeginCount].type != SUBALLOCATION_TYPE_FREE);
        // Null item at the end should be just pop_back().
        MA_VALIDATE(suballocations1st.back().type != SUBALLOCATION_TYPE_FREE);
    }
    if (!suballocations2nd.empty())
    {
        // Null item at the end should be just pop_back().
        MA_VALIDATE(suballocations2nd.back().type != SUBALLOCATION_TYPE_FREE);
    }

    MA_VALIDATE(m_1stNullItemsBeginCount + m_1stNullItemsMiddleCount <= suballocations1st.size());
    MA_VALIDATE(m_2ndNullItemsCount <= suballocations2nd.size());

    uint64_t sumUsedSize = 0;
    const size_t suballoc1stCount = suballocations1st.size();
    uint64_t offset = 0;

    if (m_2ndVectorMode == SECOND_VECTOR_RING_BUFFER)
    {
        const size_t suballoc2ndCount = suballocations2nd.size();
        size_t nullItem2ndCount = 0;
        for (size_t i = 0; i < suballoc2ndCount; ++i)
        {
            const Suballocation& suballoc = suballocations2nd[i];
            const bool currFree = (suballoc.type == SUBALLOCATION_TYPE_FREE);

            MA_VALIDATE(suballoc.offset >= offset);

            if (!currFree)
            {
                sumUsedSize += suballoc.size;
            }
            else
            {
                ++nullItem2ndCount;
            }

            offset = suballoc.offset + suballoc.size + GetDebugMargin();
        }

        MA_VALIDATE(nullItem2ndCount == m_2ndNullItemsCount);
    }

    for (size_t i = 0; i < m_1stNullItemsBeginCount; ++i)
    {
        const Suballocation& suballoc = suballocations1st[i];
        MA_VALIDATE(suballoc.type == SUBALLOCATION_TYPE_FREE &&
            suballoc.privateData == NULL);
    }

    size_t nullItem1stCount = m_1stNullItemsBeginCount;

    for (size_t i = m_1stNullItemsBeginCount; i < suballoc1stCount; ++i)
    {
        const Suballocation& suballoc = suballocations1st[i];
        const bool currFree = (suballoc.type == SUBALLOCATION_TYPE_FREE);

        MA_VALIDATE(suballoc.offset >= offset);
        MA_VALIDATE(i >= m_1stNullItemsBeginCount || currFree);

        if (!currFree)
        {
            sumUsedSize += suballoc.size;
        }
        else
        {
            ++nullItem1stCount;
        }

        offset = suballoc.offset + suballoc.size + GetDebugMargin();
    }
    MA_VALIDATE(nullItem1stCount == m_1stNullItemsBeginCount + m_1stNullItemsMiddleCount);

    if (m_2ndVectorMode == SECOND_VECTOR_DOUBLE_STACK)
    {
        const size_t suballoc2ndCount = suballocations2nd.size();
        size_t nullItem2ndCount = 0;
        for (size_t i = suballoc2ndCount; i--; )
        {
            const Suballocation& suballoc = suballocations2nd[i];
            const bool currFree = (suballoc.type == SUBALLOCATION_TYPE_FREE);

            MA_VALIDATE(suballoc.offset >= offset);

            if (!currFree)
            {
                sumUsedSize += suballoc.size;
            }
            else
            {
                ++nullItem2ndCount;
            }

            offset = suballoc.offset + suballoc.size + GetDebugMargin();
        }

        MA_VALIDATE(nullItem2ndCount == m_2ndNullItemsCount);
    }

    MA_VALIDATE(offset <= GetSize());
    MA_VALIDATE(m_SumFreeSize == GetSize() - sumUsedSize);

    return true;
}

size_t BlockMetadata_Linear::GetAllocationCount() const
{
    return AccessSuballocations1st().size() - m_1stNullItemsBeginCount - m_1stNullItemsMiddleCount +
        AccessSuballocations2nd().size() - m_2ndNullItemsCount;
}

size_t BlockMetadata_Linear::GetFreeRegionsCount() const
{
    // Function only used for defragmentation, which is disabled for this algorithm
    SKTBD_ASSERT(0);
    return SIZE_MAX;
}

void BlockMetadata_Linear::GetAllocationInfo(AllocHandle allocHandle, VIRTUAL_ALLOCATION_INFO& outInfo) const
{
    const Suballocation& suballoc = FindSuballocation((uint64_t)allocHandle - 1);
    outInfo.Offset = suballoc.offset;
    outInfo.Size = suballoc.size;
    outInfo.pPrivateData = suballoc.privateData;
}

bool BlockMetadata_Linear::CreateAllocationRequest(
    uint64_t allocSize,
    uint64_t allocAlignment,
    bool upperAddress,
    uint32_t strategy,
    AllocationRequest* pAllocationRequest)
{
    SKTBD_ASSERT(allocSize > 0 , "Cannot allocate empty block!");
    SKTBD_ASSERT(pAllocationRequest != NULL);
    MA_HEAVY_ASSERT(Validate());

    if (allocSize > GetSize())
        return false;

    pAllocationRequest->size = allocSize;
    return upperAddress ?
        CreateAllocationRequest_UpperAddress(
            allocSize, allocAlignment, pAllocationRequest) :
        CreateAllocationRequest_LowerAddress(
            allocSize, allocAlignment, pAllocationRequest);
}

void BlockMetadata_Linear::Alloc(
    const AllocationRequest& request,
    uint64_t allocSize,
    void* privateData)
{
    uint64_t offset = (uint64_t)request.allocHandle - 1;
    const Suballocation newSuballoc = { offset, request.size, privateData, SUBALLOCATION_TYPE_ALLOCATION };

    switch (request.algorithmData)
    {
    case ALLOC_REQUEST_UPPER_ADDRESS:
    {
        SKTBD_ASSERT(m_2ndVectorMode != SECOND_VECTOR_RING_BUFFER ,
            "CRITICAL ERROR: Trying to use linear allocator as double stack while it was already used as ring buffer.");
        SuballocationVectorType& suballocations2nd = AccessSuballocations2nd();
        suballocations2nd.push_back(newSuballoc);
        m_2ndVectorMode = SECOND_VECTOR_DOUBLE_STACK;
        break;
    }
    case ALLOC_REQUEST_END_OF_1ST:
    {
        SuballocationVectorType& suballocations1st = AccessSuballocations1st();

        SKTBD_ASSERT(suballocations1st.empty() ||
            offset >= suballocations1st.back().offset + suballocations1st.back().size);
        // Check if it fits before the end of the block.
        SKTBD_ASSERT(offset + request.size <= GetSize());

        suballocations1st.push_back(newSuballoc);
        break;
    }
    case ALLOC_REQUEST_END_OF_2ND:
    {
        SuballocationVectorType& suballocations1st = AccessSuballocations1st();
        // New allocation at the end of 2-part ring buffer, so before first allocation from 1st vector.
        SKTBD_ASSERT(!suballocations1st.empty() &&
            offset + request.size <= suballocations1st[m_1stNullItemsBeginCount].offset);
        SuballocationVectorType& suballocations2nd = AccessSuballocations2nd();

        switch (m_2ndVectorMode)
        {
        case SECOND_VECTOR_EMPTY:
            // First allocation from second part ring buffer.
            SKTBD_ASSERT(suballocations2nd.empty());
            m_2ndVectorMode = SECOND_VECTOR_RING_BUFFER;
            break;
        case SECOND_VECTOR_RING_BUFFER:
            // 2-part ring buffer is already started.
            SKTBD_ASSERT(!suballocations2nd.empty());
            break;
        case SECOND_VECTOR_DOUBLE_STACK:
            SKTBD_ASSERT(0 , "CRITICAL ERROR: Trying to use linear allocator as ring buffer while it was already used as double stack.");
            break;
        default:
            SKTBD_ASSERT(0);
        }

        suballocations2nd.push_back(newSuballoc);
        break;
    }
    default:
        SKTBD_ASSERT(0 , "CRITICAL INTERNAL ERROR.");
    }
    m_SumFreeSize -= newSuballoc.size;
}

void BlockMetadata_Linear::Free(AllocHandle allocHandle)
{
    SuballocationVectorType& suballocations1st = AccessSuballocations1st();
    SuballocationVectorType& suballocations2nd = AccessSuballocations2nd();
    uint64_t offset = (uint64_t)allocHandle - 1;

    if (!suballocations1st.empty())
    {
        // First allocation: Mark it as next empty at the beginning.
        Suballocation& firstSuballoc = suballocations1st[m_1stNullItemsBeginCount];
        if (firstSuballoc.offset == offset)
        {
            firstSuballoc.type = SUBALLOCATION_TYPE_FREE;
            firstSuballoc.privateData = NULL;
            m_SumFreeSize += firstSuballoc.size;
            ++m_1stNullItemsBeginCount;
            CleanupAfterFree();
            return;
        }
    }

    // Last allocation in 2-part ring buffer or top of upper stack (same logic).
    if (m_2ndVectorMode == SECOND_VECTOR_RING_BUFFER ||
        m_2ndVectorMode == SECOND_VECTOR_DOUBLE_STACK)
    {
        Suballocation& lastSuballoc = suballocations2nd.back();
        if (lastSuballoc.offset == offset)
        {
            m_SumFreeSize += lastSuballoc.size;
            suballocations2nd.pop_back();
            CleanupAfterFree();
            return;
        }
    }
    // Last allocation in 1st vector.
    else if (m_2ndVectorMode == SECOND_VECTOR_EMPTY)
    {
        Suballocation& lastSuballoc = suballocations1st.back();
        if (lastSuballoc.offset == offset)
        {
            m_SumFreeSize += lastSuballoc.size;
            suballocations1st.pop_back();
            CleanupAfterFree();
            return;
        }
    }

    Suballocation refSuballoc;
    refSuballoc.offset = offset;
    // Rest of members stays uninitialized intentionally for better performance.

    // Item from the middle of 1st vector.
    {
        const SuballocationVectorType::iterator it = BinaryFindSorted(
            suballocations1st.begin() + m_1stNullItemsBeginCount,
            suballocations1st.end(),
            refSuballoc,
            SuballocationOffsetLess());
        if (it != suballocations1st.end())
        {
            it->type = SUBALLOCATION_TYPE_FREE;
            it->privateData = NULL;
            ++m_1stNullItemsMiddleCount;
            m_SumFreeSize += it->size;
            CleanupAfterFree();
            return;
        }
    }

    if (m_2ndVectorMode != SECOND_VECTOR_EMPTY)
    {
        // Item from the middle of 2nd vector.
        const SuballocationVectorType::iterator it = m_2ndVectorMode == SECOND_VECTOR_RING_BUFFER ?
            BinaryFindSorted(suballocations2nd.begin(), suballocations2nd.end(), refSuballoc, SuballocationOffsetLess()) :
            BinaryFindSorted(suballocations2nd.begin(), suballocations2nd.end(), refSuballoc, SuballocationOffsetGreater());
        if (it != suballocations2nd.end())
        {
            it->type = SUBALLOCATION_TYPE_FREE;
            it->privateData = NULL;
            ++m_2ndNullItemsCount;
            m_SumFreeSize += it->size;
            CleanupAfterFree();
            return;
        }
    }

    SKTBD_ASSERT(0 , "Allocation to free not found in linear allocator!");
}

void BlockMetadata_Linear::Clear()
{
    m_SumFreeSize = GetSize();
    m_Suballocations0.clear();
    m_Suballocations1.clear();
    // Leaving m_1stVectorIndex unchanged - it doesn't matter.
    m_2ndVectorMode = SECOND_VECTOR_EMPTY;
    m_1stNullItemsBeginCount = 0;
    m_1stNullItemsMiddleCount = 0;
    m_2ndNullItemsCount = 0;
}

AllocHandle BlockMetadata_Linear::GetAllocationListBegin() const
{
    // Function only used for defragmentation, which is disabled for this algorithm
    SKTBD_ASSERT(0);
    return (AllocHandle)0;
}

AllocHandle BlockMetadata_Linear::GetNextAllocation(AllocHandle prevAlloc) const
{
    // Function only used for defragmentation, which is disabled for this algorithm
    SKTBD_ASSERT(0);
    return (AllocHandle)0;
}

uint64_t BlockMetadata_Linear::GetNextFreeRegionSize(AllocHandle alloc) const
{
    // Function only used for defragmentation, which is disabled for this algorithm
    SKTBD_ASSERT(0);
    return 0;
}

void* BlockMetadata_Linear::GetAllocationPrivateData(AllocHandle allocHandle) const
{
    return FindSuballocation((uint64_t)allocHandle - 1).privateData;
}

void BlockMetadata_Linear::SetAllocationPrivateData(AllocHandle allocHandle, void* privateData)
{
    Suballocation& suballoc = FindSuballocation((uint64_t)allocHandle - 1);
    suballoc.privateData = privateData;
}

void BlockMetadata_Linear::AddStatistics(Statistics& inoutStats) const
{
    inoutStats.BlockCount++;
    inoutStats.AllocationCount += (uint32_t)GetAllocationCount();
    inoutStats.BlockBytes += GetSize();
    inoutStats.AllocationBytes += GetSize() - m_SumFreeSize;
}

void BlockMetadata_Linear::AddDetailedStatistics(DetailedStatistics& inoutStats) const
{
    inoutStats.Stats.BlockCount++;
    inoutStats.Stats.BlockBytes += GetSize();

    const uint64_t size = GetSize();
    const SuballocationVectorType& suballocations1st = AccessSuballocations1st();
    const SuballocationVectorType& suballocations2nd = AccessSuballocations2nd();
    const size_t suballoc1stCount = suballocations1st.size();
    const size_t suballoc2ndCount = suballocations2nd.size();

    uint64_t lastOffset = 0;
    if (m_2ndVectorMode == SECOND_VECTOR_RING_BUFFER)
    {
        const uint64_t freeSpace2ndTo1stEnd = suballocations1st[m_1stNullItemsBeginCount].offset;
        size_t nextAlloc2ndIndex = 0;
        while (lastOffset < freeSpace2ndTo1stEnd)
        {
            // Find next non-null allocation or move nextAllocIndex to the end.
            while (nextAlloc2ndIndex < suballoc2ndCount &&
                suballocations2nd[nextAlloc2ndIndex].privateData == NULL)
            {
                ++nextAlloc2ndIndex;
            }

            // Found non-null allocation.
            if (nextAlloc2ndIndex < suballoc2ndCount)
            {
                const Suballocation& suballoc = suballocations2nd[nextAlloc2ndIndex];

                // 1. Process free space before this allocation.
                if (lastOffset < suballoc.offset)
                {
                    // There is free space from lastOffset to suballoc.offset.
                    const uint64_t unusedRangeSize = suballoc.offset - lastOffset;
                    AddDetailedStatisticsUnusedRange(inoutStats, unusedRangeSize);
                }

                // 2. Process this allocation.
                // There is allocation with suballoc.offset, suballoc.size.
                AddDetailedStatisticsAllocation(inoutStats, suballoc.size);

                // 3. Prepare for next iteration.
                lastOffset = suballoc.offset + suballoc.size;
                ++nextAlloc2ndIndex;
            }
            // We are at the end.
            else
            {
                // There is free space from lastOffset to freeSpace2ndTo1stEnd.
                if (lastOffset < freeSpace2ndTo1stEnd)
                {
                    const uint64_t unusedRangeSize = freeSpace2ndTo1stEnd - lastOffset;
                    AddDetailedStatisticsUnusedRange(inoutStats, unusedRangeSize);
                }

                // End of loop.
                lastOffset = freeSpace2ndTo1stEnd;
            }
        }
    }

    size_t nextAlloc1stIndex = m_1stNullItemsBeginCount;
    const uint64_t freeSpace1stTo2ndEnd =
        m_2ndVectorMode == SECOND_VECTOR_DOUBLE_STACK ? suballocations2nd.back().offset : size;
    while (lastOffset < freeSpace1stTo2ndEnd)
    {
        // Find next non-null allocation or move nextAllocIndex to the end.
        while (nextAlloc1stIndex < suballoc1stCount &&
            suballocations1st[nextAlloc1stIndex].privateData == NULL)
        {
            ++nextAlloc1stIndex;
        }

        // Found non-null allocation.
        if (nextAlloc1stIndex < suballoc1stCount)
        {
            const Suballocation& suballoc = suballocations1st[nextAlloc1stIndex];

            // 1. Process free space before this allocation.
            if (lastOffset < suballoc.offset)
            {
                // There is free space from lastOffset to suballoc.offset.
                const uint64_t unusedRangeSize = suballoc.offset - lastOffset;
                AddDetailedStatisticsUnusedRange(inoutStats, unusedRangeSize);
            }

            // 2. Process this allocation.
            // There is allocation with suballoc.offset, suballoc.size.
            AddDetailedStatisticsAllocation(inoutStats, suballoc.size);

            // 3. Prepare for next iteration.
            lastOffset = suballoc.offset + suballoc.size;
            ++nextAlloc1stIndex;
        }
        // We are at the end.
        else
        {
            // There is free space from lastOffset to freeSpace1stTo2ndEnd.
            if (lastOffset < freeSpace1stTo2ndEnd)
            {
                const uint64_t unusedRangeSize = freeSpace1stTo2ndEnd - lastOffset;
                AddDetailedStatisticsUnusedRange(inoutStats, unusedRangeSize);
            }

            // End of loop.
            lastOffset = freeSpace1stTo2ndEnd;
        }
    }

    if (m_2ndVectorMode == SECOND_VECTOR_DOUBLE_STACK)
    {
        size_t nextAlloc2ndIndex = suballocations2nd.size() - 1;
        while (lastOffset < size)
        {
            // Find next non-null allocation or move nextAllocIndex to the end.
            while (nextAlloc2ndIndex != SIZE_MAX &&
                suballocations2nd[nextAlloc2ndIndex].privateData == NULL)
            {
                --nextAlloc2ndIndex;
            }

            // Found non-null allocation.
            if (nextAlloc2ndIndex != SIZE_MAX)
            {
                const Suballocation& suballoc = suballocations2nd[nextAlloc2ndIndex];

                // 1. Process free space before this allocation.
                if (lastOffset < suballoc.offset)
                {
                    // There is free space from lastOffset to suballoc.offset.
                    const uint64_t unusedRangeSize = suballoc.offset - lastOffset;
                    AddDetailedStatisticsUnusedRange(inoutStats, unusedRangeSize);
                }

                // 2. Process this allocation.
                // There is allocation with suballoc.offset, suballoc.size.
                AddDetailedStatisticsAllocation(inoutStats, suballoc.size);

                // 3. Prepare for next iteration.
                lastOffset = suballoc.offset + suballoc.size;
                --nextAlloc2ndIndex;
            }
            // We are at the end.
            else
            {
                // There is free space from lastOffset to size.
                if (lastOffset < size)
                {
                    const uint64_t unusedRangeSize = size - lastOffset;
                    AddDetailedStatisticsUnusedRange(inoutStats, unusedRangeSize);
                }

                // End of loop.
                lastOffset = size;
            }
        }
    }
}

void BlockMetadata_Linear::DebugLogAllAllocations() const
{
    const SuballocationVectorType& suballocations1st = AccessSuballocations1st();
    for (auto it = suballocations1st.begin() + m_1stNullItemsBeginCount; it != suballocations1st.end(); ++it)
        if (it->type != SUBALLOCATION_TYPE_FREE)
            DebugLogAllocation(it->offset, it->size, it->privateData);

    const SuballocationVectorType& suballocations2nd = AccessSuballocations2nd();
    for (auto it = suballocations2nd.begin(); it != suballocations2nd.end(); ++it)
        if (it->type != SUBALLOCATION_TYPE_FREE)
            DebugLogAllocation(it->offset, it->size, it->privateData);
}

Suballocation& BlockMetadata_Linear::FindSuballocation(uint64_t offset) const
{
    const SuballocationVectorType& suballocations1st = AccessSuballocations1st();
    const SuballocationVectorType& suballocations2nd = AccessSuballocations2nd();

    Suballocation refSuballoc;
    refSuballoc.offset = offset;
    // Rest of members stays uninitialized intentionally for better performance.

    // Item from the 1st vector.
    {
        const SuballocationVectorType::const_iterator it = BinaryFindSorted(
            suballocations1st.begin() + m_1stNullItemsBeginCount,
            suballocations1st.end(),
            refSuballoc,
            SuballocationOffsetLess());
        if (it != suballocations1st.end())
        {
            return const_cast<Suballocation&>(*it);
        }
    }

    if (m_2ndVectorMode != SECOND_VECTOR_EMPTY)
    {
        // Rest of members stays uninitialized intentionally for better performance.
        const SuballocationVectorType::const_iterator it = m_2ndVectorMode == SECOND_VECTOR_RING_BUFFER ?
            BinaryFindSorted(suballocations2nd.begin(), suballocations2nd.end(), refSuballoc, SuballocationOffsetLess()) :
            BinaryFindSorted(suballocations2nd.begin(), suballocations2nd.end(), refSuballoc, SuballocationOffsetGreater());
        if (it != suballocations2nd.end())
        {
            return const_cast<Suballocation&>(*it);
        }
    }

    SKTBD_ASSERT(0 , "Allocation not found in linear allocator!");
    return const_cast<Suballocation&>(suballocations1st.back()); // Should never occur.
}

bool BlockMetadata_Linear::ShouldCompact1st() const
{
    const size_t nullItemCount = m_1stNullItemsBeginCount + m_1stNullItemsMiddleCount;
    const size_t suballocCount = AccessSuballocations1st().size();
    return suballocCount > 32 && nullItemCount * 2 >= (suballocCount - nullItemCount) * 3;
}

void BlockMetadata_Linear::CleanupAfterFree()
{
    SuballocationVectorType& suballocations1st = AccessSuballocations1st();
    SuballocationVectorType& suballocations2nd = AccessSuballocations2nd();

    if (IsEmpty())
    {
        suballocations1st.clear();
        suballocations2nd.clear();
        m_1stNullItemsBeginCount = 0;
        m_1stNullItemsMiddleCount = 0;
        m_2ndNullItemsCount = 0;
        m_2ndVectorMode = SECOND_VECTOR_EMPTY;
    }
    else
    {
        const size_t suballoc1stCount = suballocations1st.size();
        const size_t nullItem1stCount = m_1stNullItemsBeginCount + m_1stNullItemsMiddleCount;
        SKTBD_ASSERT(nullItem1stCount <= suballoc1stCount);

        // Find more null items at the beginning of 1st vector.
        while (m_1stNullItemsBeginCount < suballoc1stCount &&
            suballocations1st[m_1stNullItemsBeginCount].type == SUBALLOCATION_TYPE_FREE)
        {
            ++m_1stNullItemsBeginCount;
            --m_1stNullItemsMiddleCount;
        }

        // Find more null items at the end of 1st vector.
        while (m_1stNullItemsMiddleCount > 0 &&
            suballocations1st.back().type == SUBALLOCATION_TYPE_FREE)
        {
            --m_1stNullItemsMiddleCount;
            suballocations1st.pop_back();
        }

        // Find more null items at the end of 2nd vector.
        while (m_2ndNullItemsCount > 0 &&
            suballocations2nd.back().type == SUBALLOCATION_TYPE_FREE)
        {
            --m_2ndNullItemsCount;
            suballocations2nd.pop_back();
        }

        // Find more null items at the beginning of 2nd vector.
        while (m_2ndNullItemsCount > 0 &&
            suballocations2nd[0].type == SUBALLOCATION_TYPE_FREE)
        {
            --m_2ndNullItemsCount;
            suballocations2nd.remove(0);
        }

        if (ShouldCompact1st())
        {
            const size_t nonNullItemCount = suballoc1stCount - nullItem1stCount;
            size_t srcIndex = m_1stNullItemsBeginCount;
            for (size_t dstIndex = 0; dstIndex < nonNullItemCount; ++dstIndex)
            {
                while (suballocations1st[srcIndex].type == SUBALLOCATION_TYPE_FREE)
                {
                    ++srcIndex;
                }
                if (dstIndex != srcIndex)
                {
                    suballocations1st[dstIndex] = suballocations1st[srcIndex];
                }
                ++srcIndex;
            }
            suballocations1st.resize(nonNullItemCount);
            m_1stNullItemsBeginCount = 0;
            m_1stNullItemsMiddleCount = 0;
        }

        // 2nd vector became empty.
        if (suballocations2nd.empty())
        {
            m_2ndVectorMode = SECOND_VECTOR_EMPTY;
        }

        // 1st vector became empty.
        if (suballocations1st.size() - m_1stNullItemsBeginCount == 0)
        {
            suballocations1st.clear();
            m_1stNullItemsBeginCount = 0;

            if (!suballocations2nd.empty() && m_2ndVectorMode == SECOND_VECTOR_RING_BUFFER)
            {
                // Swap 1st with 2nd. Now 2nd is empty.
                m_2ndVectorMode = SECOND_VECTOR_EMPTY;
                m_1stNullItemsMiddleCount = m_2ndNullItemsCount;
                while (m_1stNullItemsBeginCount < suballocations2nd.size() &&
                    suballocations2nd[m_1stNullItemsBeginCount].type == SUBALLOCATION_TYPE_FREE)
                {
                    ++m_1stNullItemsBeginCount;
                    --m_1stNullItemsMiddleCount;
                }
                m_2ndNullItemsCount = 0;
                m_1stVectorIndex ^= 1;
            }
        }
    }

    MA_HEAVY_ASSERT(Validate());
}

bool BlockMetadata_Linear::CreateAllocationRequest_LowerAddress(
    uint64_t allocSize,
    uint64_t allocAlignment,
    AllocationRequest* pAllocationRequest)
{
    const uint64_t blockSize = GetSize();
    SuballocationVectorType& suballocations1st = AccessSuballocations1st();
    SuballocationVectorType& suballocations2nd = AccessSuballocations2nd();

    if (m_2ndVectorMode == SECOND_VECTOR_EMPTY || m_2ndVectorMode == SECOND_VECTOR_DOUBLE_STACK)
    {
        // Try to allocate at the end of 1st vector.

        uint64_t resultBaseOffset = 0;
        if (!suballocations1st.empty())
        {
            const Suballocation& lastSuballoc = suballocations1st.back();
            resultBaseOffset = lastSuballoc.offset + lastSuballoc.size + GetDebugMargin();
        }

        // Start from offset equal to beginning of free space.
        uint64_t resultOffset = resultBaseOffset;
        // Apply alignment.
        resultOffset = AlignUp(resultOffset, allocAlignment);

        const uint64_t freeSpaceEnd = m_2ndVectorMode == SECOND_VECTOR_DOUBLE_STACK ?
            suballocations2nd.back().offset : blockSize;

        // There is enough free space at the end after alignment.
        if (resultOffset + allocSize + GetDebugMargin() <= freeSpaceEnd)
        {
            // All tests passed: Success.
            pAllocationRequest->allocHandle = (AllocHandle)(resultOffset + 1);
            // pAllocationRequest->item, customData unused.
            pAllocationRequest->algorithmData = ALLOC_REQUEST_END_OF_1ST;
            return true;
        }
    }

    // Wrap-around to end of 2nd vector. Try to allocate there, watching for the
    // beginning of 1st vector as the end of free space.
    if (m_2ndVectorMode == SECOND_VECTOR_EMPTY || m_2ndVectorMode == SECOND_VECTOR_RING_BUFFER)
    {
        SKTBD_ASSERT(!suballocations1st.empty());

        uint64_t resultBaseOffset = 0;
        if (!suballocations2nd.empty())
        {
            const Suballocation& lastSuballoc = suballocations2nd.back();
            resultBaseOffset = lastSuballoc.offset + lastSuballoc.size + GetDebugMargin();
        }

        // Start from offset equal to beginning of free space.
        uint64_t resultOffset = resultBaseOffset;

        // Apply alignment.
        resultOffset = AlignUp(resultOffset, allocAlignment);

        size_t index1st = m_1stNullItemsBeginCount;
        // There is enough free space at the end after alignment.
        if ((index1st == suballocations1st.size() && resultOffset + allocSize + GetDebugMargin() <= blockSize) ||
            (index1st < suballocations1st.size() && resultOffset + allocSize + GetDebugMargin() <= suballocations1st[index1st].offset))
        {
            // All tests passed: Success.
            pAllocationRequest->allocHandle = (AllocHandle)(resultOffset + 1);
            pAllocationRequest->algorithmData = ALLOC_REQUEST_END_OF_2ND;
            // pAllocationRequest->item, customData unused.
            return true;
        }
    }
    return false;
}

bool BlockMetadata_Linear::CreateAllocationRequest_UpperAddress(
    uint64_t allocSize,
    uint64_t allocAlignment,
    AllocationRequest* pAllocationRequest)
{
    const uint64_t blockSize = GetSize();
    SuballocationVectorType& suballocations1st = AccessSuballocations1st();
    SuballocationVectorType& suballocations2nd = AccessSuballocations2nd();

    if (m_2ndVectorMode == SECOND_VECTOR_RING_BUFFER)
    {
        SKTBD_ASSERT(0 , "Trying to use pool with linear algorithm as double stack, while it is already being used as ring buffer.");
        return false;
    }

    // Try to allocate before 2nd.back(), or end of block if 2nd.empty().
    if (allocSize > blockSize)
    {
        return false;
    }
    uint64_t resultBaseOffset = blockSize - allocSize;
    if (!suballocations2nd.empty())
    {
        const Suballocation& lastSuballoc = suballocations2nd.back();
        resultBaseOffset = lastSuballoc.offset - allocSize;
        if (allocSize > lastSuballoc.offset)
        {
            return false;
        }
    }

    // Start from offset equal to end of free space.
    uint64_t resultOffset = resultBaseOffset;
    // Apply debugMargin at the end.
    if (GetDebugMargin() > 0)
    {
        if (resultOffset < GetDebugMargin())
        {
            return false;
        }
        resultOffset -= GetDebugMargin();
    }

    // Apply alignment.
    resultOffset = AlignDown(resultOffset, allocAlignment);
    // There is enough free space.
    const uint64_t endOf1st = !suballocations1st.empty() ?
        suballocations1st.back().offset + suballocations1st.back().size : 0;

    if (endOf1st + GetDebugMargin() <= resultOffset)
    {
        // All tests passed: Success.
        pAllocationRequest->allocHandle = (AllocHandle)(resultOffset + 1);
        // pAllocationRequest->item unused.
        pAllocationRequest->algorithmData = ALLOC_REQUEST_UPPER_ADDRESS;
        return true;
    }
    return false;
}
#endif // _MA_BLOCK_METADATA_LINEAR_FUNCTIONS
#endif // _MA_BLOCK_METADATA_LINEAR

#ifndef _MA_BLOCK_METADATA_TLSF
class BlockMetadata_TLSF : public BlockMetadata
{
public:
    BlockMetadata_TLSF(const ALLOCATION_CALLBACKS* allocationCallbacks, bool isVirtual);
    virtual ~BlockMetadata_TLSF();

    size_t GetAllocationCount() const override { return m_AllocCount; }
    size_t GetFreeRegionsCount() const override { return m_BlocksFreeCount + 1; }
    uint64_t GetSumFreeSize() const override { return m_BlocksFreeSize + m_NullBlock->size; }
    bool IsEmpty() const override { return m_NullBlock->offset == 0; }
    uint64_t GetAllocationOffset(AllocHandle allocHandle) const override { return ((Block*)allocHandle)->offset; };

    void Init(uint64_t size) override;
    bool Validate() const override;
    void GetAllocationInfo(AllocHandle allocHandle, VIRTUAL_ALLOCATION_INFO& outInfo) const override;

    bool CreateAllocationRequest(
        uint64_t allocSize,
        uint64_t allocAlignment,
        bool upperAddress,
        uint32_t strategy,
        AllocationRequest* pAllocationRequest) override;

    void Alloc(
        const AllocationRequest& request,
        uint64_t allocSize,
        void* privateData) override;

    void Free(AllocHandle allocHandle) override;
    void Clear() override;

    AllocHandle GetAllocationListBegin() const override;
    AllocHandle GetNextAllocation(AllocHandle prevAlloc) const override;
    uint64_t GetNextFreeRegionSize(AllocHandle alloc) const override;
    void* GetAllocationPrivateData(AllocHandle allocHandle) const override;
    void SetAllocationPrivateData(AllocHandle allocHandle, void* privateData) override;

    void AddStatistics(Statistics& inoutStats) const override;
    void AddDetailedStatistics(DetailedStatistics& inoutStats) const override;
   // void WriteAllocationInfoToJson(JsonWriter& json) const override;
    void DebugLogAllAllocations() const override;

private:
    // According to original paper it should be preferable 4 or 5:
    // M. Masmano, I. Ripoll, A. Crespo, and J. Real "TLSF: a New Dynamic Memory Allocator for Real-Time Systems"
    // http://www.gii.upv.es/tlsf/files/ecrts04_tlsf.pdf
    static const uint8_t SECOND_LEVEL_INDEX = 5;
    static const uint16_t SMALL_BUFFER_SIZE = 256;
    static const uint32_t INITIAL_BLOCK_ALLOC_COUNT = 16;
    static const uint8_t MEMORY_CLASS_SHIFT = 7;
    static const uint8_t MAX_MEMORY_CLASSES = 65 - MEMORY_CLASS_SHIFT;

    class Block
    {
    public:
        uint64_t offset;
        uint64_t size;
        Block* prevPhysical;
        Block* nextPhysical;

        void MarkFree() { prevFree = NULL; }
        void MarkTaken() { prevFree = this; }
        bool IsFree() const { return prevFree != this; }
        void*& PrivateData() { MA_HEAVY_ASSERT(!IsFree()); return privateData; }
        Block*& PrevFree() { return prevFree; }
        Block*& NextFree() { MA_HEAVY_ASSERT(IsFree()); return nextFree; }

    private:
        Block* prevFree; // Address of the same block here indicates that block is taken
        union
        {
            Block* nextFree;
            void* privateData;
        };
    };

    size_t m_AllocCount = 0;
    // Total number of free blocks besides null block
    size_t m_BlocksFreeCount = 0;
    // Total size of free blocks excluding null block
    uint64_t m_BlocksFreeSize = 0;
    uint32_t m_IsFreeBitmap = 0;
    uint8_t m_MemoryClasses = 0;
    uint32_t m_InnerIsFreeBitmap[MAX_MEMORY_CLASSES];
    uint32_t m_ListsCount = 0;
    /*
    * 0: 0-3 lists for small buffers
    * 1+: 0-(2^SLI-1) lists for normal buffers
    */
    Block** m_FreeList = NULL;
    PoolAllocator<Block> m_BlockAllocator;
    Block* m_NullBlock = NULL;

    uint8_t SizeToMemoryClass(uint64_t size) const;
    uint16_t SizeToSecondIndex(uint64_t size, uint8_t memoryClass) const;
    uint32_t GetListIndex(uint8_t memoryClass, uint16_t secondIndex) const;
    uint32_t GetListIndex(uint64_t size) const;

    void RemoveFreeBlock(Block* block);
    void InsertFreeBlock(Block* block);
    void MergeBlock(Block* block, Block* prev);

    Block* FindFreeBlock(uint64_t size, uint32_t& listIndex) const;
    bool CheckBlock(
        Block& block,
        uint32_t listIndex,
        uint64_t allocSize,
        uint64_t allocAlignment,
        AllocationRequest* pAllocationRequest);

    MA_CLASS_NO_COPY(BlockMetadata_TLSF)
};

#ifndef _MA_BLOCK_METADATA_TLSF_FUNCTIONS
BlockMetadata_TLSF::BlockMetadata_TLSF(const ALLOCATION_CALLBACKS* allocationCallbacks, bool isVirtual)
    : BlockMetadata(allocationCallbacks, isVirtual),
    m_BlockAllocator(*allocationCallbacks, INITIAL_BLOCK_ALLOC_COUNT)
{
    SKTBD_ASSERT(allocationCallbacks);
}

BlockMetadata_TLSF::~BlockMetadata_TLSF()
{
    MA_DELETE_ARRAY(*GetAllocs(), m_FreeList, m_ListsCount);
}

void BlockMetadata_TLSF::Init(uint64_t size)
{
    BlockMetadata::Init(size);

    m_NullBlock = m_BlockAllocator.Alloc();
    m_NullBlock->size = size;
    m_NullBlock->offset = 0;
    m_NullBlock->prevPhysical = NULL;
    m_NullBlock->nextPhysical = NULL;
    m_NullBlock->MarkFree();
    m_NullBlock->NextFree() = NULL;
    m_NullBlock->PrevFree() = NULL;
    uint8_t memoryClass = SizeToMemoryClass(size);
    uint16_t sli = SizeToSecondIndex(size, memoryClass);
    m_ListsCount = (memoryClass == 0 ? 0 : (memoryClass - 1) * (1UL << SECOND_LEVEL_INDEX) + sli) + 1;
    if (IsVirtual())
        m_ListsCount += 1UL << SECOND_LEVEL_INDEX;
    else
        m_ListsCount += 4;

    m_MemoryClasses = memoryClass + 2;
    memset(m_InnerIsFreeBitmap, 0, MAX_MEMORY_CLASSES * sizeof(uint32_t));

    m_FreeList = MA_NEW_ARRAY(*GetAllocs(), Block*, m_ListsCount);
    memset(m_FreeList, 0, m_ListsCount * sizeof(Block*));
}

bool BlockMetadata_TLSF::Validate() const
{
    MA_VALIDATE(GetSumFreeSize() <= GetSize());

    uint64_t calculatedSize = m_NullBlock->size;
    uint64_t calculatedFreeSize = m_NullBlock->size;
    size_t allocCount = 0;
    size_t freeCount = 0;

    // Check integrity of free lists
    for (uint32_t list = 0; list < m_ListsCount; ++list)
    {
        Block* block = m_FreeList[list];
        if (block != NULL)
        {
            MA_VALIDATE(block->IsFree());
            MA_VALIDATE(block->PrevFree() == NULL);
            while (block->NextFree())
            {
                MA_VALIDATE(block->NextFree()->IsFree());
                MA_VALIDATE(block->NextFree()->PrevFree() == block);
                block = block->NextFree();
            }
        }
    }

    MA_VALIDATE(m_NullBlock->nextPhysical == NULL);
    if (m_NullBlock->prevPhysical)
    {
        MA_VALIDATE(m_NullBlock->prevPhysical->nextPhysical == m_NullBlock);
    }

    // Check all blocks
    uint64_t nextOffset = m_NullBlock->offset;
    for (Block* prev = m_NullBlock->prevPhysical; prev != NULL; prev = prev->prevPhysical)
    {
        MA_VALIDATE(prev->offset + prev->size == nextOffset);
        nextOffset = prev->offset;
        calculatedSize += prev->size;

        uint32_t listIndex = GetListIndex(prev->size);
        if (prev->IsFree())
        {
            ++freeCount;
            // Check if free block belongs to free list
            Block* freeBlock = m_FreeList[listIndex];
            MA_VALIDATE(freeBlock != NULL);

            bool found = false;
            do
            {
                if (freeBlock == prev)
                    found = true;

                freeBlock = freeBlock->NextFree();
            } while (!found && freeBlock != NULL);

            MA_VALIDATE(found);
            calculatedFreeSize += prev->size;
        }
        else
        {
            ++allocCount;
            // Check if taken block is not on a free list
            Block* freeBlock = m_FreeList[listIndex];
            while (freeBlock)
            {
                MA_VALIDATE(freeBlock != prev);
                freeBlock = freeBlock->NextFree();
            }
        }

        if (prev->prevPhysical)
        {
            MA_VALIDATE(prev->prevPhysical->nextPhysical == prev);
        }
    }

    MA_VALIDATE(nextOffset == 0);
    MA_VALIDATE(calculatedSize == GetSize());
    MA_VALIDATE(calculatedFreeSize == GetSumFreeSize());
    MA_VALIDATE(allocCount == m_AllocCount);
    MA_VALIDATE(freeCount == m_BlocksFreeCount);

    return true;
}

void BlockMetadata_TLSF::GetAllocationInfo(AllocHandle allocHandle, VIRTUAL_ALLOCATION_INFO& outInfo) const
{
    Block* block = (Block*)allocHandle;
    SKTBD_ASSERT(!block->IsFree() , "Cannot get allocation info for free block!");
    outInfo.Offset = block->offset;
    outInfo.Size = block->size;
    outInfo.pPrivateData = block->PrivateData();
}

bool BlockMetadata_TLSF::CreateAllocationRequest(
    uint64_t allocSize,
    uint64_t allocAlignment,
    bool upperAddress,
    uint32_t strategy,
    AllocationRequest* pAllocationRequest)
{
    SKTBD_ASSERT(allocSize > 0 , "Cannot allocate empty block!");
    SKTBD_ASSERT(!upperAddress , "ALLOCATION_FLAG_UPPER_ADDRESS can be used only with linear algorithm.");
    SKTBD_ASSERT(pAllocationRequest != NULL);
    MA_HEAVY_ASSERT(Validate());

    allocSize += GetDebugMargin();
    // Quick check for too small pool
    if (allocSize > GetSumFreeSize())
        return false;

    // If no free blocks in pool then check only null block
    if (m_BlocksFreeCount == 0)
        return CheckBlock(*m_NullBlock, m_ListsCount, allocSize, allocAlignment, pAllocationRequest);

    // Round up to the next block
    uint64_t sizeForNextList = allocSize;
    uint16_t smallSizeStep = SMALL_BUFFER_SIZE / (IsVirtual() ? 1 << SECOND_LEVEL_INDEX : 4);
    if (allocSize > SMALL_BUFFER_SIZE)
    {
        sizeForNextList += (1ULL << (BitScanMSB(allocSize) - SECOND_LEVEL_INDEX));
    }
    else if (allocSize > SMALL_BUFFER_SIZE - smallSizeStep)
        sizeForNextList = SMALL_BUFFER_SIZE + 1;
    else
        sizeForNextList += smallSizeStep;

    uint32_t nextListIndex = 0;
    uint32_t prevListIndex = 0;
    Block* nextListBlock = NULL;
    Block* prevListBlock = NULL;

    // Check blocks according to strategies
    if (strategy & VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_TIME)
    {
        // Quick check for larger block first
        nextListBlock = FindFreeBlock(sizeForNextList, nextListIndex);
        if (nextListBlock != NULL && CheckBlock(*nextListBlock, nextListIndex, allocSize, allocAlignment, pAllocationRequest))
            return true;

        // If not fitted then null block
        if (CheckBlock(*m_NullBlock, m_ListsCount, allocSize, allocAlignment, pAllocationRequest))
            return true;

        // Null block failed, search larger bucket
        while (nextListBlock)
        {
            if (CheckBlock(*nextListBlock, nextListIndex, allocSize, allocAlignment, pAllocationRequest))
                return true;
            nextListBlock = nextListBlock->NextFree();
        }

        // Failed again, check best fit bucket
        prevListBlock = FindFreeBlock(allocSize, prevListIndex);
        while (prevListBlock)
        {
            if (CheckBlock(*prevListBlock, prevListIndex, allocSize, allocAlignment, pAllocationRequest))
                return true;
            prevListBlock = prevListBlock->NextFree();
        }
    }
    else if (strategy & VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_MEMORY)
    {
        // Check best fit bucket
        prevListBlock = FindFreeBlock(allocSize, prevListIndex);
        while (prevListBlock)
        {
            if (CheckBlock(*prevListBlock, prevListIndex, allocSize, allocAlignment, pAllocationRequest))
                return true;
            prevListBlock = prevListBlock->NextFree();
        }

        // If failed check null block
        if (CheckBlock(*m_NullBlock, m_ListsCount, allocSize, allocAlignment, pAllocationRequest))
            return true;

        // Check larger bucket
        nextListBlock = FindFreeBlock(sizeForNextList, nextListIndex);
        while (nextListBlock)
        {
            if (CheckBlock(*nextListBlock, nextListIndex, allocSize, allocAlignment, pAllocationRequest))
                return true;
            nextListBlock = nextListBlock->NextFree();
        }
    }
    else if (strategy & VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_OFFSET)
    {
        // Perform search from the start
        Vector<Block*> blockList(m_BlocksFreeCount, *GetAllocs());

        size_t i = m_BlocksFreeCount;
        for (Block* block = m_NullBlock->prevPhysical; block != NULL; block = block->prevPhysical)
        {
            if (block->IsFree() && block->size >= allocSize)
                blockList[--i] = block;
        }

        for (; i < m_BlocksFreeCount; ++i)
        {
            Block& block = *blockList[i];
            if (CheckBlock(block, GetListIndex(block.size), allocSize, allocAlignment, pAllocationRequest))
                return true;
        }

        // If failed check null block
        if (CheckBlock(*m_NullBlock, m_ListsCount, allocSize, allocAlignment, pAllocationRequest))
            return true;

        // Whole range searched, no more memory
        return false;
    }
    else
    {
        // Check larger bucket
        nextListBlock = FindFreeBlock(sizeForNextList, nextListIndex);
        while (nextListBlock)
        {
            if (CheckBlock(*nextListBlock, nextListIndex, allocSize, allocAlignment, pAllocationRequest))
                return true;
            nextListBlock = nextListBlock->NextFree();
        }

        // If failed check null block
        if (CheckBlock(*m_NullBlock, m_ListsCount, allocSize, allocAlignment, pAllocationRequest))
            return true;

        // Check best fit bucket
        prevListBlock = FindFreeBlock(allocSize, prevListIndex);
        while (prevListBlock)
        {
            if (CheckBlock(*prevListBlock, prevListIndex, allocSize, allocAlignment, pAllocationRequest))
                return true;
            prevListBlock = prevListBlock->NextFree();
        }
    }

    // Worst case, full search has to be done
    while (++nextListIndex < m_ListsCount)
    {
        nextListBlock = m_FreeList[nextListIndex];
        while (nextListBlock)
        {
            if (CheckBlock(*nextListBlock, nextListIndex, allocSize, allocAlignment, pAllocationRequest))
                return true;
            nextListBlock = nextListBlock->NextFree();
        }
    }

    // No more memory sadly
    return false;
}

void BlockMetadata_TLSF::Alloc(
    const AllocationRequest& request,
    uint64_t allocSize,
    void* privateData)
{
    // Get block and pop it from the free list
    Block* currentBlock = (Block*)request.allocHandle;
    uint64_t offset = request.algorithmData;
    SKTBD_ASSERT(currentBlock != NULL);
    SKTBD_ASSERT(currentBlock->offset <= offset);

    if (currentBlock != m_NullBlock)
        RemoveFreeBlock(currentBlock);

    // Append missing alignment to prev block or create new one
    uint64_t misssingAlignment = offset - currentBlock->offset;
    if (misssingAlignment)
    {
        Block* prevBlock = currentBlock->prevPhysical;
        SKTBD_ASSERT(prevBlock != NULL , "There should be no missing alignment at offset 0!");

        if (prevBlock->IsFree() && prevBlock->size != GetDebugMargin())
        {
            uint32_t oldList = GetListIndex(prevBlock->size);
            prevBlock->size += misssingAlignment;
            // Check if new size crosses list bucket
            if (oldList != GetListIndex(prevBlock->size))
            {
                prevBlock->size -= misssingAlignment;
                RemoveFreeBlock(prevBlock);
                prevBlock->size += misssingAlignment;
                InsertFreeBlock(prevBlock);
            }
            else
                m_BlocksFreeSize += misssingAlignment;
        }
        else
        {
            Block* newBlock = m_BlockAllocator.Alloc();
            currentBlock->prevPhysical = newBlock;
            prevBlock->nextPhysical = newBlock;
            newBlock->prevPhysical = prevBlock;
            newBlock->nextPhysical = currentBlock;
            newBlock->size = misssingAlignment;
            newBlock->offset = currentBlock->offset;
            newBlock->MarkTaken();

            InsertFreeBlock(newBlock);
        }

        currentBlock->size -= misssingAlignment;
        currentBlock->offset += misssingAlignment;
    }

    uint64_t size = request.size + GetDebugMargin();
    if (currentBlock->size == size)
    {
        if (currentBlock == m_NullBlock)
        {
            // Setup new null block
            m_NullBlock = m_BlockAllocator.Alloc();
            m_NullBlock->size = 0;
            m_NullBlock->offset = currentBlock->offset + size;
            m_NullBlock->prevPhysical = currentBlock;
            m_NullBlock->nextPhysical = NULL;
            m_NullBlock->MarkFree();
            m_NullBlock->PrevFree() = NULL;
            m_NullBlock->NextFree() = NULL;
            currentBlock->nextPhysical = m_NullBlock;
            currentBlock->MarkTaken();
        }
    }
    else
    {
        SKTBD_ASSERT(currentBlock->size > size , "Proper block already found, shouldn't find smaller one!");

        // Create new free block
        Block* newBlock = m_BlockAllocator.Alloc();
        newBlock->size = currentBlock->size - size;
        newBlock->offset = currentBlock->offset + size;
        newBlock->prevPhysical = currentBlock;
        newBlock->nextPhysical = currentBlock->nextPhysical;
        currentBlock->nextPhysical = newBlock;
        currentBlock->size = size;

        if (currentBlock == m_NullBlock)
        {
            m_NullBlock = newBlock;
            m_NullBlock->MarkFree();
            m_NullBlock->NextFree() = NULL;
            m_NullBlock->PrevFree() = NULL;
            currentBlock->MarkTaken();
        }
        else
        {
            newBlock->nextPhysical->prevPhysical = newBlock;
            newBlock->MarkTaken();
            InsertFreeBlock(newBlock);
        }
    }
    currentBlock->PrivateData() = privateData;

    if (GetDebugMargin() > 0)
    {
        currentBlock->size -= GetDebugMargin();
        Block* newBlock = m_BlockAllocator.Alloc();
        newBlock->size = GetDebugMargin();
        newBlock->offset = currentBlock->offset + currentBlock->size;
        newBlock->prevPhysical = currentBlock;
        newBlock->nextPhysical = currentBlock->nextPhysical;
        newBlock->MarkTaken();
        currentBlock->nextPhysical->prevPhysical = newBlock;
        currentBlock->nextPhysical = newBlock;
        InsertFreeBlock(newBlock);
    }
    ++m_AllocCount;
}

void BlockMetadata_TLSF::Free(AllocHandle allocHandle)
{
    Block* block = (Block*)allocHandle;
    Block* next = block->nextPhysical;
    SKTBD_ASSERT(!block->IsFree() , "Block is already free!");

    --m_AllocCount;
    if (GetDebugMargin() > 0)
    {
        RemoveFreeBlock(next);
        MergeBlock(next, block);
        block = next;
        next = next->nextPhysical;
    }

    // Try merging
    Block* prev = block->prevPhysical;
    if (prev != NULL && prev->IsFree() && prev->size != GetDebugMargin())
    {
        RemoveFreeBlock(prev);
        MergeBlock(block, prev);
    }

    if (!next->IsFree())
        InsertFreeBlock(block);
    else if (next == m_NullBlock)
        MergeBlock(m_NullBlock, block);
    else
    {
        RemoveFreeBlock(next);
        MergeBlock(next, block);
        InsertFreeBlock(next);
    }
}

void BlockMetadata_TLSF::Clear()
{
    m_AllocCount = 0;
    m_BlocksFreeCount = 0;
    m_BlocksFreeSize = 0;
    m_IsFreeBitmap = 0;
    m_NullBlock->offset = 0;
    m_NullBlock->size = GetSize();
    Block* block = m_NullBlock->prevPhysical;
    m_NullBlock->prevPhysical = NULL;
    while (block)
    {
        Block* prev = block->prevPhysical;
        m_BlockAllocator.Free(block);
        block = prev;
    }
    memset(m_FreeList, 0, m_ListsCount * sizeof(Block*));
    memset(m_InnerIsFreeBitmap, 0, m_MemoryClasses * sizeof(uint32_t));
}

AllocHandle BlockMetadata_TLSF::GetAllocationListBegin() const
{
    if (m_AllocCount == 0)
        return (AllocHandle)0;

    for (Block* block = m_NullBlock->prevPhysical; block; block = block->prevPhysical)
    {
        if (!block->IsFree())
            return (AllocHandle)block;
    }
    SKTBD_ASSERT(false , "If m_AllocCount > 0 then should find any allocation!");
    return (AllocHandle)0;
}

AllocHandle BlockMetadata_TLSF::GetNextAllocation(AllocHandle prevAlloc) const
{
    Block* startBlock = (Block*)prevAlloc;
    SKTBD_ASSERT(!startBlock->IsFree(),"Incorrect block!");

    for (Block* block = startBlock->prevPhysical; block; block = block->prevPhysical)
    {
        if (!block->IsFree())
            return (AllocHandle)block;
    }
    return (AllocHandle)0;
}

uint64_t BlockMetadata_TLSF::GetNextFreeRegionSize(AllocHandle alloc) const
{
    Block* block = (Block*)alloc;
    SKTBD_ASSERT(!block->IsFree() , "Incorrect block!");

    if (block->prevPhysical)
        return block->prevPhysical->IsFree() ? block->prevPhysical->size : 0;
    return 0;
}

void* BlockMetadata_TLSF::GetAllocationPrivateData(AllocHandle allocHandle) const
{
    Block* block = (Block*)allocHandle;
    SKTBD_ASSERT(!block->IsFree() , "Cannot get user data for free block!");
    return block->PrivateData();
}

void BlockMetadata_TLSF::SetAllocationPrivateData(AllocHandle allocHandle, void* privateData)
{
    Block* block = (Block*)allocHandle;
    SKTBD_ASSERT(!block->IsFree() , "Trying to set user data for not allocated block!");
    block->PrivateData() = privateData;
}

void BlockMetadata_TLSF::AddStatistics(Statistics& inoutStats) const
{
    inoutStats.BlockCount++;
    inoutStats.AllocationCount += static_cast<uint32_t>(m_AllocCount);
    inoutStats.BlockBytes += GetSize();
    inoutStats.AllocationBytes += GetSize() - GetSumFreeSize();
}

void BlockMetadata_TLSF::AddDetailedStatistics(DetailedStatistics& inoutStats) const
{
    inoutStats.Stats.BlockCount++;
    inoutStats.Stats.BlockBytes += GetSize();

    for (Block* block = m_NullBlock->prevPhysical; block != NULL; block = block->prevPhysical)
    {
        if (block->IsFree())
            AddDetailedStatisticsUnusedRange(inoutStats, block->size);
        else
            AddDetailedStatisticsAllocation(inoutStats, block->size);
    }

    if (m_NullBlock->size > 0)
        AddDetailedStatisticsUnusedRange(inoutStats, m_NullBlock->size);
}

void BlockMetadata_TLSF::DebugLogAllAllocations() const
{
    for (Block* block = m_NullBlock->prevPhysical; block != NULL; block = block->prevPhysical)
    {
        if (!block->IsFree())
        {
            DebugLogAllocation(block->offset, block->size, block->PrivateData());
        }
    }
}

uint8_t BlockMetadata_TLSF::SizeToMemoryClass(uint64_t size) const
{
    if (size > SMALL_BUFFER_SIZE)
        return BitScanMSB(size) - MEMORY_CLASS_SHIFT;
    return 0;
}

uint16_t BlockMetadata_TLSF::SizeToSecondIndex(uint64_t size, uint8_t memoryClass) const
{
    if (memoryClass == 0)
    {
        if (IsVirtual())
            return static_cast<uint16_t>((size - 1) / 8);
        else
            return static_cast<uint16_t>((size - 1) / 64);
    }
    return static_cast<uint16_t>((size >> (memoryClass + MEMORY_CLASS_SHIFT - SECOND_LEVEL_INDEX)) ^ (1U << SECOND_LEVEL_INDEX));
}

uint32_t BlockMetadata_TLSF::GetListIndex(uint8_t memoryClass, uint16_t secondIndex) const
{
    if (memoryClass == 0)
        return secondIndex;

    const uint32_t index = static_cast<uint32_t>(memoryClass - 1) * (1 << SECOND_LEVEL_INDEX) + secondIndex;
    if (IsVirtual())
        return index + (1 << SECOND_LEVEL_INDEX);
    else
        return index + 4;
}

uint32_t BlockMetadata_TLSF::GetListIndex(uint64_t size) const
{
    uint8_t memoryClass = SizeToMemoryClass(size);
    return GetListIndex(memoryClass, SizeToSecondIndex(size, memoryClass));
}

void BlockMetadata_TLSF::RemoveFreeBlock(Block* block)
{
    SKTBD_ASSERT(block != m_NullBlock);
    SKTBD_ASSERT(block->IsFree());

    if (block->NextFree() != NULL)
        block->NextFree()->PrevFree() = block->PrevFree();
    if (block->PrevFree() != NULL)
        block->PrevFree()->NextFree() = block->NextFree();
    else
    {
        uint8_t memClass = SizeToMemoryClass(block->size);
        uint16_t secondIndex = SizeToSecondIndex(block->size, memClass);
        uint32_t index = GetListIndex(memClass, secondIndex);
        m_FreeList[index] = block->NextFree();
        if (block->NextFree() == NULL)
        {
            m_InnerIsFreeBitmap[memClass] &= ~(1U << secondIndex);
            if (m_InnerIsFreeBitmap[memClass] == 0)
                m_IsFreeBitmap &= ~(1UL << memClass);
        }
    }
    block->MarkTaken();
    block->PrivateData() = NULL;
    --m_BlocksFreeCount;
    m_BlocksFreeSize -= block->size;
}

void BlockMetadata_TLSF::InsertFreeBlock(Block* block)
{
    SKTBD_ASSERT(block != m_NullBlock);
    SKTBD_ASSERT(!block->IsFree() , "Cannot insert block twice!");

    uint8_t memClass = SizeToMemoryClass(block->size);
    uint16_t secondIndex = SizeToSecondIndex(block->size, memClass);
    uint32_t index = GetListIndex(memClass, secondIndex);
    block->PrevFree() = NULL;
    block->NextFree() = m_FreeList[index];
    m_FreeList[index] = block;
    if (block->NextFree() != NULL)
        block->NextFree()->PrevFree() = block;
    else
    {
        m_InnerIsFreeBitmap[memClass] |= 1U << secondIndex;
        m_IsFreeBitmap |= 1UL << memClass;
    }
    ++m_BlocksFreeCount;
    m_BlocksFreeSize += block->size;
}

void BlockMetadata_TLSF::MergeBlock(Block* block, Block* prev)
{
    SKTBD_ASSERT(block->prevPhysical == prev , "Cannot merge seperate physical regions!");
    SKTBD_ASSERT(!prev->IsFree() , "Cannot merge block that belongs to free list!");

    block->offset = prev->offset;
    block->size += prev->size;
    block->prevPhysical = prev->prevPhysical;
    if (block->prevPhysical)
        block->prevPhysical->nextPhysical = block;
    m_BlockAllocator.Free(prev);
}

BlockMetadata_TLSF::Block* BlockMetadata_TLSF::FindFreeBlock(uint64_t size, uint32_t& listIndex) const
{
    uint8_t memoryClass = SizeToMemoryClass(size);
    uint32_t innerFreeMap = m_InnerIsFreeBitmap[memoryClass] & (~0U << SizeToSecondIndex(size, memoryClass));
    if (!innerFreeMap)
    {
        // Check higher levels for avaiable blocks
        uint32_t freeMap = m_IsFreeBitmap & (~0UL << (memoryClass + 1));
        if (!freeMap)
            return NULL; // No more memory avaible

        // Find lowest free region
        memoryClass = BitScanLSB(freeMap);
        innerFreeMap = m_InnerIsFreeBitmap[memoryClass];
        SKTBD_ASSERT(innerFreeMap != 0);
    }
    // Find lowest free subregion
    listIndex = GetListIndex(memoryClass, BitScanLSB(innerFreeMap));
    return m_FreeList[listIndex];
}

bool BlockMetadata_TLSF::CheckBlock(
    Block& block,
    uint32_t listIndex,
    uint64_t allocSize,
    uint64_t allocAlignment,
    AllocationRequest* pAllocationRequest)
{
    SKTBD_ASSERT(block.IsFree() , "Block is already taken!");

    uint64_t alignedOffset = AlignUp(block.offset, allocAlignment);
    if (block.size < allocSize + alignedOffset - block.offset)
        return false;

    // Alloc successful
    pAllocationRequest->allocHandle = (AllocHandle)&block;
    pAllocationRequest->size = allocSize - GetDebugMargin();
    pAllocationRequest->algorithmData = alignedOffset;

    // Place block at the start of list if it's normal block
    if (listIndex != m_ListsCount && block.PrevFree())
    {
        block.PrevFree()->NextFree() = block.NextFree();
        if (block.NextFree())
            block.NextFree()->PrevFree() = block.PrevFree();
        block.PrevFree() = NULL;
        block.NextFree() = m_FreeList[listIndex];
        m_FreeList[listIndex] = &block;
        if (block.NextFree())
            block.NextFree()->PrevFree() = &block;
    }

    return true;
}
#endif // _MA_BLOCK_METADATA_TLSF_FUNCTIONS
#endif // _MA_BLOCK_METADATA_TLSF

#ifndef _MA_VIRTUAL_BLOCK_PIMPL
class VirtualBlockPimpl
{
public:
    const ALLOCATION_CALLBACKS m_AllocationCallbacks;
    const uint64_t m_Size;
    BlockMetadata* m_Metadata;

    VirtualBlockPimpl(const ALLOCATION_CALLBACKS& allocationCallbacks, const VIRTUAL_BLOCK_DESC& desc);
    ~VirtualBlockPimpl();
};

#ifndef _MA_VIRTUAL_BLOCK_PIMPL_FUNCTIONS
VirtualBlockPimpl::VirtualBlockPimpl(const ALLOCATION_CALLBACKS& allocationCallbacks, const VIRTUAL_BLOCK_DESC& desc)
    : m_AllocationCallbacks(allocationCallbacks), m_Size(desc.Size)
{
    switch (desc.Flags & VIRTUAL_BLOCK_FLAG_ALGORITHM_MASK)
    {
    case VIRTUAL_BLOCK_FLAG_ALGORITHM_LINEAR:
        m_Metadata = MA_NEW(allocationCallbacks, BlockMetadata_Linear)(&m_AllocationCallbacks, true);
        break;
    default:
        SKTBD_ASSERT(0);
    case 0:
        m_Metadata = MA_NEW(allocationCallbacks, BlockMetadata_TLSF)(&m_AllocationCallbacks, true);
        break;
    }
    m_Metadata->Init(m_Size);
}

VirtualBlockPimpl::~VirtualBlockPimpl()
{
    MA_DELETE(m_AllocationCallbacks, m_Metadata);
}
#endif // _MA_VIRTUAL_BLOCK_PIMPL_FUNCTIONS
#endif // _MA_VIRTUAL_BLOCK_PIMPL

#ifndef _MA_VIRTUAL_BLOCK_FUNCTIONS
bool BlockAllocator::IsEmpty() const
{
    MA_DEBUG_GLOBAL_MUTEX_LOCK
        return m_Pimpl->m_Metadata->IsEmpty() ? true : false;
}

void BlockAllocator::GetAllocationInfo(VirtualAllocation allocation, VIRTUAL_ALLOCATION_INFO* pInfo) const
{
    SKTBD_ASSERT(allocation.AllocHandle != (AllocHandle)0 && pInfo);

    MA_DEBUG_GLOBAL_MUTEX_LOCK
        m_Pimpl->m_Metadata->GetAllocationInfo(allocation.AllocHandle, *pInfo);
}

SKTBDR BlockAllocator::Allocate(const VIRTUAL_ALLOCATION_DESC* pDesc, VirtualAllocation* pAllocation, uint64_t* pOffset)
{
    if (!pDesc || !pAllocation || pDesc->Size == 0 || !IsPow2(pDesc->Alignment))
    {
        SKTBD_ASSERT(0 , "Invalid arguments passed to Allocator::Allocate.");
        return OOPS_INVALIDARG;
    }

    MA_DEBUG_GLOBAL_MUTEX_LOCK

        const uint64_t alignment = pDesc->Alignment != 0 ? pDesc->Alignment : 1;
    AllocationRequest allocRequest = {};
    if (m_Pimpl->m_Metadata->CreateAllocationRequest(
        pDesc->Size,
        alignment,
        pDesc->Flags & VIRTUAL_ALLOCATION_FLAG_UPPER_ADDRESS,
        pDesc->Flags & VIRTUAL_ALLOCATION_FLAG_STRATEGY_MASK,
        &allocRequest))
    {
        m_Pimpl->m_Metadata->Alloc(allocRequest, pDesc->Size, pDesc->pPrivateData);
        MA_HEAVY_ASSERT(m_Pimpl->m_Metadata->Validate());
        pAllocation->AllocHandle = allocRequest.allocHandle;

        if (pOffset)
            *pOffset = m_Pimpl->m_Metadata->GetAllocationOffset(allocRequest.allocHandle);
        return OKEYDOKEY;
    }

    pAllocation->AllocHandle = (AllocHandle)0;
    if (pOffset)
        *pOffset = UINT64_MAX;

    return OOPS_OUTOFMEMORY;
}

void BlockAllocator::FreeAllocation(VirtualAllocation allocation)
{
    if (allocation.AllocHandle == (AllocHandle)0)
        return;

    MA_DEBUG_GLOBAL_MUTEX_LOCK

    m_Pimpl->m_Metadata->Free(allocation.AllocHandle);
    MA_HEAVY_ASSERT(m_Pimpl->m_Metadata->Validate());
}

void BlockAllocator::Clear()
{
    MA_DEBUG_GLOBAL_MUTEX_LOCK

    m_Pimpl->m_Metadata->Clear();
    MA_HEAVY_ASSERT(m_Pimpl->m_Metadata->Validate());
}

void BlockAllocator::SetAllocationPrivateData(VirtualAllocation allocation, void* pPrivateData)
{
    SKTBD_ASSERT(allocation.AllocHandle != (AllocHandle)0);

    MA_DEBUG_GLOBAL_MUTEX_LOCK
        m_Pimpl->m_Metadata->SetAllocationPrivateData(allocation.AllocHandle, pPrivateData);
}

void BlockAllocator::GetStatistics(Statistics* pStats) const
{
    SKTBD_ASSERT(pStats);
    MA_DEBUG_GLOBAL_MUTEX_LOCK
        MA_HEAVY_ASSERT(m_Pimpl->m_Metadata->Validate());
    ClearStatistics(*pStats);
    m_Pimpl->m_Metadata->AddStatistics(*pStats);
}

void BlockAllocator::CalculateStatistics(DetailedStatistics* pStats) const
{
    SKTBD_ASSERT(pStats);
    MA_DEBUG_GLOBAL_MUTEX_LOCK
        MA_HEAVY_ASSERT(m_Pimpl->m_Metadata->Validate());
    ClearDetailedStatistics(*pStats);
    m_Pimpl->m_Metadata->AddDetailedStatistics(*pStats);
}

void BlockAllocator::BuildStatsString(wchar_t** ppStatsString) const
{
    SKTBD_ASSERT(ppStatsString);

    MA_DEBUG_GLOBAL_MUTEX_LOCK

    StringBuilder sb(m_Pimpl->m_AllocationCallbacks);


    const size_t length = sb.GetLength();
    wchar_t* result = AllocateArray<wchar_t>(m_Pimpl->m_AllocationCallbacks, length + 1);
    memcpy(result, sb.GetData(), length * sizeof(wchar_t));
    result[length] = L'\0';
    *ppStatsString = result;
}

void BlockAllocator::FreeStatsString(wchar_t* pStatsString) const
{
    if (pStatsString != NULL)
    {
        MA_DEBUG_GLOBAL_MUTEX_LOCK
            Free(m_Pimpl->m_AllocationCallbacks, pStatsString);
    }
}

void BlockAllocator::ReleaseThis()
{
    // Copy is needed because otherwise we would call destructor and invalidate the structure with callbacks before using it to free memory.
    const ALLOCATION_CALLBACKS allocationCallbacksCopy = m_Pimpl->m_AllocationCallbacks;
    MA_DELETE(allocationCallbacksCopy, this);
}

BlockAllocator::BlockAllocator(const ALLOCATION_CALLBACKS& allocationCallbacks, const VIRTUAL_BLOCK_DESC& desc)
    : m_Pimpl(new(::Skateboard::MemoryUtils::Allocate<VirtualBlockPimpl>(allocationCallbacks))(VirtualBlockPimpl)(allocationCallbacks, desc)) {}

BlockAllocator::~BlockAllocator()
{
    // THIS IS AN IMPORTANT ASSERT!
    // Hitting it means you have some memory leak - unreleased allocations in this virtual block.
    SKTBD_ASSERT(m_Pimpl->m_Metadata->IsEmpty() , "Some allocations were not freed before destruction of this virtual block!");

    MA_DELETE(m_Pimpl->m_AllocationCallbacks, m_Pimpl);
}
#endif // _MA_VIRTUAL_BLOCK_FUNCTIONS

void CreateBlock(const VIRTUAL_BLOCK_DESC* pDesc, BlockAllocator** ppVirtualBlock)
{
    if (!pDesc || !ppVirtualBlock)
    {
        SKTBD_LOG_ASSERT(0, "Invalid arguments passed to CreateVirtualBlock.");
     //   return E_INVALIDARG;
    }

    MA_DEBUG_GLOBAL_MUTEX_LOCK

        ALLOCATION_CALLBACKS allocationCallbacks;
    SetupAllocationCallbacks(allocationCallbacks, pDesc->pAllocationCallbacks);

    *ppVirtualBlock = MA_NEW(allocationCallbacks, BlockAllocator)(allocationCallbacks, *pDesc);
    ///return S_OK;
}

};