#pragma once

#include <cstdlib>
#include <stdint.h>
#include <cstring>
#include <memory>

namespace MFA
{

    class BaseBlob
    {
    public:
        
        template <typename U>
        [[nodiscard]]
    	U * As()
    	{
    		return reinterpret_cast<U*>(_ptr);
    	}

        [[nodiscard]]
        uint8_t * Ptr() const
    	{
            return _ptr;
    	}

        [[nodiscard]]
        size_t Len() const
    	{
            return _len;
    	}

        [[nodiscard]]
        bool IsValid() const
    	{
            return _ptr != nullptr && _len > 0;
    	}

    protected:

        uint8_t * _ptr = nullptr;
        size_t _len = 0;

    };

    class Alias : public BaseBlob
    {
    public:

        template<typename T>
        explicit Alias(T * ptr, size_t const count)
    	{
            _len = sizeof(T) * count;
            _ptr = reinterpret_cast<uint8_t *>(ptr);
        }

        template<typename T>
        explicit Alias(T const * ptr, size_t const count)
        {
            _len = sizeof(T) * count;
            _ptr = reinterpret_cast<uint8_t*>(const_cast<T *>(ptr));
        }

        template<typename T>
        explicit Alias(T & data)
        {
            _len = sizeof(T);
            _ptr = reinterpret_cast<uint8_t *>(&data);
        }

        template<typename T>
        explicit Alias(T const & data)
        {
            _len = sizeof(T);
            _ptr = reinterpret_cast<uint8_t*>(const_cast<T *>(&data));
        }

        ~Alias() = default;
    };

    class Blob : public BaseBlob
    {
    public:

    	explicit Blob(size_t const len)
    	{
            _ptr = new uint8_t[len];
            _len = len;
    	}

        explicit Blob(BaseBlob const & blob)
        {
            _len = blob.Len();
            _ptr = new uint8_t[_len];
            std::memcpy(_ptr, blob.Ptr(), _len);
        }

        // Creates a copy from buffer
        template<typename T>
        explicit Blob(T * ptr, size_t const count)
    	{
            _len = sizeof(T) * count;
            _ptr = new uint8_t[_len];
            std::memcpy(_ptr, ptr, _len);
    	}

        template<typename T>
        explicit Blob(T const & data)
        {
            _len = sizeof(T);
            _ptr = new uint8_t[_len];
            std::memcpy(_ptr, &data, _len);
        }

        ~Blob()
    	{
            std::free(_ptr);
    	}

        operator Alias() const {
            return Alias(_ptr, _len);
        }

    };

    namespace Memory
    {
        [[nodiscard]]
        inline std::unique_ptr<Blob> AllocSize(size_t const len)
        {
            return std::make_unique<Blob>(len);
        }

        template<typename T>
        [[nodiscard]]
        inline std::unique_ptr<Blob> Alloc(T * ptr, size_t const count)
        {
            return std::make_unique<Blob>(ptr, count);
        }

        template<typename T>
        [[nodiscard]]
        inline std::unique_ptr<Blob> Alloc(T const & data)
        {
            return std::make_unique<Blob>(data, true);
        }

        template<uint32_t Count, typename B, typename A>
        constexpr void Copy(B* dst, A const* src)
        {
            static_assert(sizeof(B) == sizeof(A));
            memcpy(dst, src, Count * sizeof(B));
        }

        template<uint32_t Count, typename  T>
        constexpr void Copy(T* dst, std::initializer_list<T> items)
        {
            memcpy(dst, items.begin(), Count * sizeof(T));
        }

        template<typename T, typename B>
        constexpr void Copy(T* dst, B const* src, uint32_t const count)
        {
            static_assert(sizeof(T) == sizeof(B));
            memcpy(dst, src, count * sizeof(T));
        }

        template<typename A, typename B>
        constexpr void Copy(A& dst, B const& src)
        {
            if constexpr (sizeof(A) > sizeof(B))
            {
                memcpy(&dst, &src, sizeof(B));
            }
            else
            {
                memcpy(&dst, &src, sizeof(A));
            }
        }

        template<uint32_t Count, typename A, typename B>
        constexpr void Copy(A& dst, B const* src)
        {
            static_assert(sizeof(A) >= sizeof(B) * Count);
            memcpy(&dst, src, sizeof(B) * Count);
        }

        template<uint32_t Count, typename A, typename B>
        constexpr void Copy(A* dst, B const& src)
        {
            static_assert(sizeof(A) * Count <= sizeof(B));
            memcpy(dst, &src, sizeof(A) * Count);
        }

        template<typename A, typename B>
        constexpr A Copy(B const& src)
        {
            A dst{};
            Copy(dst, src);
            return dst;
        }

        template<uint32_t Count, typename A, typename B>
        constexpr A Copy(B const* src)
        {
            A dst{};
            Copy<Count, A, B>(dst, src);
            return dst;
        }

        template<uint32_t Count, typename T>
        constexpr bool IsEqual(T const* memory1, T const* memory2)
        {
            return memcmp(memory1, memory2, Count * sizeof(T)) == 0;
        }

        template<uint32_t Count, typename A, typename B>
        constexpr bool IsEqual(A const& memory1, B const* memory2)
        {
            static_assert(sizeof(A) >= sizeof(B) * Count);
            return memcmp(&memory1, memory2, sizeof(B) * Count) == 0;
        }

        template<uint32_t Count, typename A, typename B>
        constexpr bool IsEqual(A const* memory1, B const& memory2)
        {
            static_assert(sizeof(A) * Count <= sizeof(B));
            return memcmp(memory1, &memory2, sizeof(A) * Count) == 0;
        }

        template<typename A, typename B>
        constexpr bool IsEqual(A const& memory1, B const& memory2)
        {
            if constexpr (sizeof(A) > sizeof(B))
            {
                return memcmp(&memory1, &memory2, sizeof(B)) == 0;
            }
            return memcmp(&memory1, &memory2, sizeof(A)) == 0;
        }

        template<typename T>
        constexpr void SetZero(T const* memory, uint32_t length)
        {
            memset(memory, 0, sizeof(T) * length);
        }

    }

}