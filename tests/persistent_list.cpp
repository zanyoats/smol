#include <smol/persistent_list.hpp>

#include <catch2/catch_test_macros.hpp>
#include<stdexcept>
#include <cstddef>
#include <ranges>
#include <sstream>
#include <algorithm>

// empty construction
TEST_CASE("a default-constructed list is empty", "[persistent_list]") {
    const smol::persistent_list<int> xs;

    REQUIRE(xs.is_empty());
}

// non-empty construction and head()
TEST_CASE("head() returns the first element", "[persistent_list]") {
    const smol::persistent_list<int> xs{42};

    REQUIRE(xs.head() == 42);
}

// empty construction and head()
TEST_CASE("head() throws on an empty list", "[persistent_list]") {
    const smol::persistent_list<int> xs{};

    REQUIRE_THROWS_AS(xs.head(), std::out_of_range);
}

// non-empty construction and tail()
TEST_CASE("tail() returns the first element", "[persistent_list]") {
    const smol::persistent_list<int> xs{42};

    REQUIRE(xs.tail().is_empty());
}

// empty construction and tail()
TEST_CASE("tail() throws on an empty list", "[persistent_list]") {
    const smol::persistent_list<int> xs{};

    REQUIRE_THROWS_AS(xs.tail(), std::out_of_range);
}

// prepend element
TEST_CASE("prepend an element to existing list", "[persistent_list]") {
    const smol::persistent_list<int> xs;
    auto ys = 42 + xs;

    REQUIRE(ys.head() == 42);
}

// iteration
TEST_CASE("iteration visits each element in order", "[persistent_list]") {
    const smol::persistent_list<int> xs{42, 12};
    const int expected[]{42, 12};

    REQUIRE(std::ranges::equal(xs, expected));
}

// from range builder
TEST_CASE("from_range reads values from an input stream", "[persistent_list]") {
    std::istringstream input{"42 12"};
    auto values = std::ranges::istream_view<int>(input);

    const auto xs = smol::persistent_list<int>::from_range(values);

    const int expected[]{42,12};
    REQUIRE(std::ranges::equal(xs, expected));
}

// structural sharing, prepending
TEST_CASE("prepending shares the original list as its tail", "[persistent_list]") {
    const smol::persistent_list<int> xs{12};
    const auto ys = 42 + xs;

    REQUIRE(ys.tail().same(xs));
    REQUIRE_FALSE(ys.same(xs));
}

// structural sharing, copying
TEST_CASE("copying a list shares its root", "[persistent_list]") {
    const smol::persistent_list<int> xs{42};
    const auto ys = xs; // NOLINT(performance-unnecessary-copy-initialization)

    REQUIRE(ys.same(xs));
}

// value equality
TEST_CASE("list equality compares elements in order", "[persistent_list]") {
    const smol::persistent_list<int> empty;
    const smol::persistent_list<int> xs{42, 12};
    const smol::persistent_list<int> equal{42, 12};
    const smol::persistent_list<int> reordered{12, 42};
    const smol::persistent_list<int> prefix{42};

    REQUIRE(empty == empty);
    REQUIRE(xs == equal);
    REQUIRE_FALSE(xs == reordered);
    REQUIRE_FALSE(xs == prefix);
    REQUIRE_FALSE(prefix == xs);
}
