#pragma once
#ifndef PCH
    #include <mutex>
    #include <type_traits>
#endif

namespace stdext
{
    template <typename T>
    class secured: public T
    {
    public:
        mutable std::mutex mutex;

        secured() = default;

        template <typename U>
        secured(const U& item): T(item) {}

        template <typename U>
        secured(U&& item): T(std::move(item)) {}

        template <typename ...Args>
        secured(Args... args): T(std::forward(args)...) {}

        void operator = (const T& item)
        {
            *static_cast<T*>(this) = item;
        }

        void operator = (T&& item)
        {
            *static_cast<T*>(this) = std::move(item);
        }
    };
}
