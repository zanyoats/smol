// An immutable singly linked list with shared tails (a.k.a. persisten lists).
// Prepending allocates one node. Copying a list shares its root without copying
// any elements.
//
// Elements are accessed through const references. Objects referenced by an
// element may still be mutable. A reference returned by head() stays valid
// only while its node is alive.
//
// Releasing the last owner of a long, unshared chain can destroy nodes
// recursively and exhaust the call stack.

#pragma once

#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <utility>
#include <cstddef>
#include <iterator>
#include <concepts>
#include <ranges>
#include <vector> // TODO: remove when smol::array available
#include <ostream>

namespace smol {

template<class T>
class persistent_list {
private:
    struct node {
        T value_;
        std::shared_ptr<const node> next_;

        node(T&& value, std::shared_ptr<const node> next)
            :value_(std::move(value)), next_(std::move(next))
            {}
    };

    std::shared_ptr<const node> ptr_;

    explicit persistent_list(std::shared_ptr<const node> ptr) noexcept
        :ptr_(std::move(ptr))
        {}

public:
    persistent_list() noexcept = default; // empty list construction

    template<class U = T>
        requires std::copy_constructible<U>
    persistent_list(std::initializer_list<T> values) {
        auto it = values.end();

        while (it != values.begin()) {
            --it;
            *this = *it + *this;
        }
    }

    // `+` is always left-associative so an expression such as:
    //   1 + 2 + xs
    // will be parsed as:
    //   (1 + 2) + xs
    // This unfortunately can't be configured and is a design tradeoff. To
    // get the correct expression use parethesis:
    //   1 + (2 + xs)
    friend persistent_list operator+(T x, const persistent_list& xs) {
        return persistent_list(
            std::make_shared<const node>(
                std::move(x),
                xs.ptr_
            )
        );
    }

    bool is_empty() const noexcept {
        return !ptr_;
    }

    const T& head() const {
        if (!ptr_) {
            throw std::out_of_range("head() on empty list");
        }

        return ptr_->value_;
    }

    persistent_list tail() const {
        if (!ptr_) {
            throw std::out_of_range("tail() on empty list");
        }

        return persistent_list(ptr_->next_);
    }

    // iterator
    class const_iterator {
    private:
        std::shared_ptr<const node> current_;

        friend class persistent_list;

        explicit const_iterator(std::shared_ptr<const node> current) noexcept
            :current_(std::move(current))
            {}

    public:
        // Help generic code work with this iterator
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using reference = const T&;
        using pointer = const T*;

        using iterator_category = std::forward_iterator_tag;
        using iterator_concept = std::forward_iterator_tag;

        const_iterator() noexcept = default;

        reference operator*() const noexcept {
            return current_->value_;
        }

        pointer operator->() const noexcept {
            return std::addressof(operator*());
        }

        const_iterator& operator++() noexcept {
            current_ = current_->next_;
            return *this;
        }

        const_iterator operator++(int) noexcept {
            auto previous = *this;
            ++(*this);
            return previous;
        }

        bool operator==(const const_iterator&) const noexcept = default;
    };

    const_iterator begin() const noexcept {
        return const_iterator(ptr_);
    }

    const_iterator end() const noexcept {
        return const_iterator{};
    }

    // construct from range
    template<std::ranges::input_range R>
        requires std::constructible_from<T, std::ranges::range_reference_t<R>>
            && std::move_constructible<T>
    static persistent_list from_range(R&& values) {
        // Alternatives to using std::vector as a buffer:
        //
        // 1. Require a bidirectional range. Walk the input backward and prepend
        //    directly. This needs no temporary buffer, but excludes forward-only
        //    and single-pass ranges.
        //
        // 2. Prepend into a temporary list, then prepend from it into the result.
        //    This builds two list nodes per input element. The second pass reads
        //    each element through const T&, so it requires copyable elements.
        //
        // 3. Use a future growable smol::array as the buffer. Like std::vector,
        //    it can support move-only T by moving elements during growth and
        //    allowing them to be moved out during backward traversal.
        //
        // Alternative 3 is the ideal choice since we don't want to depend on
        // the standard container library while building smol ;)
        std::vector<T> buffer;

        for (auto&& value : values) {
            buffer.emplace_back(
                std::forward<decltype(value)>(value)
            );
        }

        persistent_list result;

        for (auto it = buffer.rbegin(); it != buffer.rend(); ++it) {
            result = std::move(*it) + result;
        }

        return result;
    }

    // equality
    bool same(const persistent_list& ys) const noexcept {
        return ptr_ == ys.ptr_;
    }

    bool operator==(const persistent_list& ys) const
        requires std::equality_comparable<T>
    {
        auto* left = ptr_.get();
        auto* right = ys.ptr_.get();

        while (left && right) {
            if (!(left->value_ == right->value_)) return false;

            left = left->next_.get();
            right = right->next_.get();
        }

        return left == right;
    }

    // output
    friend std::ostream& operator<<(std::ostream& out, const persistent_list& xs)
        requires requires(const T& value) { out << value; }
    {
        out << '(';

        bool first = true;
        for (const T& x : xs) {
            if (!first) {
                out << ' ';
            }
            out << x;
            first = false;
        }

        return out << ')';
    }
}; // class persistent_list

} // namespace smol
