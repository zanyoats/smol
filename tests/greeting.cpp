#include <smol/greeting.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("The greeting is Hello, World", "[greeting]") {
    REQUIRE(smol::greeting == "Hello, World");
}
