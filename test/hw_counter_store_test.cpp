

#include <catch.hpp>

#include "../hw_counters_store.hpp"
#include "../error_handling.hpp"

TEST_CASE("HW_COUNTER_STORE", "[HW_COUNTER_STORE]") {
    HwCounterStore store(5);

    for (std::size_t i = 0; i < 5; i++) {
        store.append(i);
    }

    SECTION("CONTENT") {
        REQUIRE(0 == store[0]);
        REQUIRE(1 == store[1]);
        REQUIRE(2 == store[2]);
        REQUIRE(3 == store[3]);
        REQUIRE(4 == store[4]);
    }

    SECTION("OVERFLOW") {
        bool b = false;
        setDefaultErrorHandler([&b](char const* msg) { b = true; });
        store.append(5);

        REQUIRE(b);
        resetDefaultErrorHandler();
    }

    SECTION("SIZE") { REQUIRE(5 == store.size()); }
}

