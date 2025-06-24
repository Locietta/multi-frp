#pragma once
#include <memory>

#include "trait.hpp"

template <typename Derived>
struct PimplBase : Unique {
    using Deleter = void (*)(void *);
    std::unique_ptr<void, Deleter> ptr_;

    // Constructor for derived classes
    explicit PimplBase(void *ptr, Deleter deleter = [](void *p) { delete static_cast<typename Derived::Impl *>(p); })
        : ptr_(ptr, deleter) {}
};

template <typename Derived>
struct Pimpl : Unique {
    PimplBase<Derived> impl_;

    // delay instantiation of the implementation
    template <typename... Args>
    Pimpl(Args &&...args)
        : impl_(new typename Derived::Impl(std::forward<Args>(args)...)) {}

    template <typename U>
    void reset(std::unique_ptr<U> new_impl) noexcept {
        impl_.ptr_.reset(new_impl.release());
    }

    // Accessor for the implementation
    template <typename ImplType>
    ImplType *impl() noexcept {
        return static_cast<ImplType *>(impl_.ptr_.get());
    }
    template <typename ImplType>
    const ImplType *impl() const noexcept {
        return static_cast<const ImplType *>(impl_.ptr_.get());
    }
};