/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 6.00.00.38-00.00.00.0.1
* Copyright (C) 2022 Sony Interactive Entertainment Inc.
* 
*/

#ifndef PACK_FILE_ARRAY_H
#define PACK_FILE_ARRAY_H

#include <cstddef> // size_t / nullptr_t
#include <cstdint> // uint8_t
#include <cstring> // memcpy
#include <initializer_list>
#include <new>

#if __clang__
#define RESTRICT_PTR restrict
#else
#define RESTRICT_PTR
#endif

namespace PackFile
{

template <typename T>
class Array
{
public:
    using value_t = T;
    explicit Array(size_t size) noexcept
    {
        if (size > 0)
        {
            elems = new (std::nothrow) T[size];
            sentinel = elems + size;
            last = elems + size;
        }
    }
    Array(std::initializer_list<T> list) noexcept
        : Array(list.size())
    {
        size_t idx = 0;
        for (auto const& elem : list)
        {
            elems[idx] = elem;
            idx++;
        }
    }
    ~Array()
    {
        delete[] elems;
    }
    Array() = default;
    Array(Array const& to_copy) = delete;
    Array& operator=(Array const& to_assign) = delete;
    Array(Array&& to_move)
       : elems(to_move.elems)
       , sentinel(to_move.sentinel)
       , last(to_move.last)
    {
       to_move.elems    = nullptr;
       to_move.sentinel = nullptr;
       to_move.last     = nullptr;
    }
    Array& operator=(Array&& to_assign)
    {
       if (this != &to_assign)
       {
          delete[] elems;
          elems    = to_assign.elems;
          sentinel = to_assign.sentinel;
          last     = to_assign.last;
          to_assign.elems = nullptr;
          to_assign.sentinel = nullptr;
          to_assign.last = nullptr;
       }
       return *this;
    }

    T* data() { return elems; }
    const T* data() const { return elems; }

    T* begin() { return elems; }
    T* end() { return sentinel; }

    T const* begin() const { return elems; }
    T const* end() const { return sentinel; }

    size_t getCount() const { return size(); }
    size_t size() const { return sentinel - elems; }

    void push_back(T const& x)
    {
        if (sentinel == last)
        {
            size_t old_size = size();
            // 8 12 19 30 48 77 127 200 323 522
            size_t new_size = (old_size == 0) ? 8 : old_size * 2584/1597; // Golden ratio
            T* temp = new (std::nothrow) T[new_size];
            T* cursor = temp; // should use std::copy but I don`t want to include any std lib.
            for(auto e : *this) *(cursor++) = e;
            delete[] elems;
            elems = temp;
            sentinel = elems + old_size;
            last = elems + new_size;
        }
        *sentinel = x;
        sentinel++;
    }

    const T &operator[] (size_t index) const
    {
       return elems[index];
    }

    T &operator[] (size_t index)
    {
       return elems[index];
    }

    bool operator==(nullptr_t) const
    {
       return elems == nullptr;
    }
    bool operator!=(nullptr_t) const
    {
       return elems != nullptr;
    }
private:
    T *elems = nullptr;       // The array elements.
    T *sentinel = nullptr;    // Pointer to the element after last valid
    T *last = nullptr;        // Pointer to the element after last allocated
};

struct String : public Array<char>
{
   String() : Array<char>(){}
   // Stores extra byte for null terminator
   explicit String(size_t size) : Array<char>(size + 1){}

   operator char *()
   {
      return data();
   }
   operator const char *() const
   {
      return data();
   }
};

class Stream
{
public:
    enum class SeekFrom
    {
        k_begin,
        k_current,
        k_end
    };
    ~Stream() = default;
    virtual bool skip(size_t ) = 0;
    virtual bool seek(int64_t offset, SeekFrom start) = 0;
    virtual size_t read(void* RESTRICT_PTR out_buffer, size_t to_read) = 0;
    virtual size_t position() const = 0;

    bool good() const {return !error; }
    bool bad() const {return error; }

protected:
    mutable bool error = false; // Error flag, is mutable because errors can happen in const function as well.
};

struct BufferFile : public Stream
{
    uint8_t const* const buffer;
    uint8_t const* const sentinel;
    uint8_t const* cursor;

    BufferFile(uint8_t const* begin, uint8_t const* end)
      : buffer { begin }
      , sentinel { end }
      , cursor{ begin }
    {
    }

    BufferFile(Array<uint8_t> const& datas)
      : BufferFile { datas.begin(), datas.end() }
    {
    }

    bool skip(size_t to_skip) override
    {
        if (bad()) return false;
        uint8_t const* temp = cursor + to_skip;
        if (temp > sentinel)
            error = true;
        else
            cursor = temp;
        return good();
    }

    bool seek(int64_t offset, SeekFrom start) override
    {
        if (bad()) return false;
        uintptr_t temp = 0;
        switch (start)
        {
        case Stream::SeekFrom::k_begin:
            temp = (uintptr_t)buffer + offset;
            break;
        case Stream::SeekFrom::k_current:
            temp = (uintptr_t)cursor + offset;
            break;
        case Stream::SeekFrom::k_end:
            temp = (uintptr_t)sentinel + offset;
            break;
        }
        if ((temp >= (uintptr_t)buffer) && (temp <= (uintptr_t)sentinel))
        {
            cursor = (uint8_t const*)temp;
            return true;
        }
        else
        {
            error = true;
            return false;
        }
    }

    size_t read(void* RESTRICT_PTR out_buffer, size_t to_read) override
    {
        if (bad()) return 0;
        if (cursor + to_read > sentinel)
        {
            error = true;
            return 0;
        }
        memcpy(out_buffer, cursor, to_read);
        cursor += to_read;
        return to_read;
    }

    size_t position() const override
    {
        return cursor - buffer;
    }
};

}
#endif //! PACK_FILE_ARRAY_H
