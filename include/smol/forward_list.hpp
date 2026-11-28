#pragma once

#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>
#include <cstddef>
#include <iterator>

namespace smol {

template<class T>
class forward_list {
private:
    struct node;

    struct node_base {
        std::unique_ptr<node> next_;
    };

    struct node : node_base {
        T value_;

        explicit node(const T& value) :value_(value) {}

        explicit node(T&& value) :value_(std::move(value)) {}

        // Iteratively remove all nodes this node can reach, then clean up of
        // this node is ready (this->next_ is null).
        // The default destructor would make recusive calls taking up O(n) stack
        // space, possibly exhausting the stack for long lists.
        ~node() {
            while (this->next_) {
                auto removed = std::move(this->next_);
                this->next_ = std::move(removed->next_);
            }
        }
    };

    node_base before_head_{};

public:
    forward_list() noexcept = default;

    forward_list(std::initializer_list<T> values) {
        auto it = before_begin();

        for (const auto& value : values) {
            it = insert_after(it, value);
        }
    }

    forward_list(const forward_list& other) {
        auto it = before_begin();

        for (const auto& value : other) {
            it = insert_after(it, value);
        }
    }

    forward_list& operator=(const forward_list& other) {
        forward_list copy(other);
        return *this = std::move(copy);
    }

    forward_list(forward_list&& other) noexcept = default;

    forward_list& operator=(forward_list&& other) noexcept = default;

    template<bool IsConst>
    class basic_iterator {
    private:
        using node_base_pointer = std::conditional_t<IsConst, const node_base*, node_base*>;
        using node_pointer = std::conditional_t<IsConst, const node*, node*>;

        friend class forward_list;

        template<bool>
        friend class basic_iterator;

        node_base_pointer current_ = nullptr;

        explicit basic_iterator(node_base_pointer position)
            :current_(position)
            {}

    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using reference = std::conditional_t<IsConst, const T&, T&>;
        using pointer = std::conditional_t<IsConst, const T*, T*>;

        using iterator_category = std::forward_iterator_tag;
        using iterator_concept = std::forward_iterator_tag;

        basic_iterator() = default;

        // Implicitly convert iterator to const_iterator by copying the node pointer.
        // Both still point to the same node. The reverse conversion would grant write access.
        // Making it non-explicit allows the compiler to use it like:
        //      iterator a = it;
        //      const_iterator b = it;
        // in addition to normal usage:
        //      const_iterator b{it};

        template<bool OtherConst>
            requires (IsConst && !OtherConst)
        basic_iterator(const basic_iterator<OtherConst>& other) noexcept
            :current_(other.current_)
            {}

        basic_iterator& operator++() {
            current_ = current_->next_.get();
            return *this;
        }

        basic_iterator operator++(int) {
            auto previous = *this;
            ++(*this);
            return previous;
        }

        reference operator*() const {
            auto* element = static_cast<node_pointer>(current_);
            return element->value_;
        }

        pointer operator->() const {
            return std::addressof(operator*());
        }

        bool operator==(const basic_iterator&) const = default;
    };

    using iterator = basic_iterator<false>;
    using const_iterator = basic_iterator<true>;

    iterator before_begin() noexcept {
        return iterator{std::addressof(before_head_)};
    }

    const_iterator before_begin() const noexcept {
        return const_iterator{std::addressof(before_head_)};
    }

    const_iterator cbefore_begin() const noexcept {
        return before_begin();
    }

    iterator begin() noexcept {
        return iterator{before_head_.next_.get()};
    }

    const_iterator begin() const noexcept {
        return const_iterator{before_head_.next_.get()};
    }

    const_iterator cbegin() const noexcept {
        return begin();
    }

    iterator end() noexcept {
        return iterator{};
    }

    const_iterator end() const noexcept {
        return const_iterator{};
    }

    const_iterator cend() const noexcept {
        return end();
    }

    // precondition:
    //   called on sentinel or element position in list; calling with end() is
    //   undefined behavior.
    iterator insert_after(const_iterator it, const T& value) {
        return insert_after_aux(it, value);
    }

    iterator insert_after(const_iterator it, T&& value) {
        return insert_after_aux(it, std::move(value));
    }

private:
    template<class U>
    iterator insert_after_aux(const_iterator it, U&& value) {
        auto new_node = std::make_unique<node>(std::forward<U>(value));
        auto& next = const_cast<node_base*>(it.current_)->next_;

        new_node->next_ = std::move(next);
        next = std::move(new_node);

        return iterator{next.get()};
    }

public:
    bool is_empty() const noexcept {
        return before_head_.next_ == nullptr;
    }

    // precondition:
    //   list must be non-empty
    T& front() noexcept {
        return before_head_.next_->value_;
    }

    const T& front() const noexcept {
        return before_head_.next_->value_;
    }

    // precondition:
    //   iterator in list must have a node afterwards
    iterator erase_after(const_iterator it) {
        auto& next = const_cast<node_base*>(it.current_)->next_;
        auto erased = std::move(next);

        next = std::move(erased->next_);

        return iterator{next.get()};
    }
}; // class forward_list

} // namespace smol
