#include <smol/forward_list.hpp>

#include <catch2/catch_test_macros.hpp>
#include <algorithm>

// empty construction
TEST_CASE("a default-constructed list is empty", "[forward_list]") {
    const smol::forward_list<int> xs;

    REQUIRE(xs.is_empty());
    REQUIRE(xs.begin() == xs.end());
}

// non-empty construction and front()
TEST_CASE("front() returns the first element", "[forward_list]") {
    const smol::forward_list<int> xs{42, 12};

    REQUIRE(xs.front() == 42);
}

// iteration
TEST_CASE("iteration visits each element in order", "[forward_list]") {
    const smol::forward_list<int> xs{42, 12};
    const int expected[]{42, 12};

    REQUIRE(std::ranges::equal(xs, expected));
}

// copy construction
TEST_CASE("copy construction creates an independent list", "[forward_list]") {
    const smol::forward_list<int> source{42, 12};
    smol::forward_list<int> copy(source);

    const int original[]{42, 12};
    REQUIRE(std::ranges::equal(copy, original));

    copy.front() = 99;

    const int modified[]{99, 12};
    REQUIRE(std::ranges::equal(copy, modified));
    REQUIRE(std::ranges::equal(source, original));
}

// copy assignment
TEST_CASE("copy assignment replaces the destination with an independent copy", "[forward_list]") {
    const smol::forward_list<int> source{42, 12};
    smol::forward_list<int> destination{7, 8, 9};

    destination = source;

    const int original[]{42, 12};
    REQUIRE(std::ranges::equal(destination, original));

    destination.front() = 99;

    const int modified[]{99, 12};
    REQUIRE(std::ranges::equal(destination, modified));
    REQUIRE(std::ranges::equal(source, original));
}

// prepend element
TEST_CASE("inserting after before_begin() prepends an element", "[forward_list]") {
    smol::forward_list<int> xs{42};
    auto it = xs.insert_after(xs.before_begin(), 12);
    REQUIRE(it != xs.end());
    REQUIRE(*it == 12);

    const int expected[]{12, 42};
    REQUIRE(std::ranges::equal(xs, expected));
}

// append element
TEST_CASE("inserting after the last element appends an element", "[forward_list]") {
    smol::forward_list<int> xs;
    auto first = xs.insert_after(xs.before_begin(), 42);
    REQUIRE(first != xs.end());
    REQUIRE(*first == 42);

    auto second = xs.insert_after(first, 12);
    REQUIRE(second != xs.end());
    REQUIRE(*second == 12);

    const int expected[]{42, 12};
    REQUIRE(std::ranges::equal(xs, expected));
}

// erase after, first element
TEST_CASE("erasing the first element returns its successor", "[forward_list]") {
    smol::forward_list<int> xs{12, 42, 100};

    auto successor = xs.erase_after(xs.before_begin());
    REQUIRE(successor != xs.end());
    REQUIRE(*successor == 42);

    const int expected[]{42, 100};
    REQUIRE(std::ranges::equal(xs, expected));
}

// erase after, middle element
TEST_CASE("erasing the middle element returns its successor", "[forward_list]") {
    smol::forward_list<int> xs{12, 42, 100};

    auto successor = xs.erase_after(xs.begin());
    REQUIRE(successor != xs.end());
    REQUIRE(*successor == 100);

    const int expected[]{12, 100};
    REQUIRE(std::ranges::equal(xs, expected));
}


// erase after, last element
TEST_CASE("erasing the last element returns end()", "[forward_list]") {
    smol::forward_list<int> xs{12, 42, 100};

    auto predecessor = xs.begin();
    ++predecessor;
    auto successor = xs.erase_after(predecessor);
    REQUIRE(successor == xs.end());

    const int expected[]{12, 42};
    REQUIRE(std::ranges::equal(xs, expected));
}

// erase after, empty list
TEST_CASE("erasing the only element leaves an empty list", "[forward_list]") {
    smol::forward_list<int> xs{42};
    REQUIRE(!xs.is_empty());

    auto successor = xs.erase_after(xs.before_begin());

    REQUIRE(successor == xs.end());
    REQUIRE(xs.begin() == xs.end());
    REQUIRE(xs.is_empty());
}
