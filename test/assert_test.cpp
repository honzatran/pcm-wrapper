

#include <catch.hpp>
#include "../error_handling.hpp"

#include <iostream>

using namespace std;

TEST_CASE("TEST", "[TEST]")
{
    REQUIRE(1 + 1 == 2);
}

TEST_CASE("ASSERT", "[TEST]")
{
    bool fatalErrorHit = false;
    setDefaultErrorHandler(
        [&fatalErrorHit](char const* data) { fatalErrorHit = true; });

    // this is our assert not assert of the test framework
    //
    ASSERT(1 == 2, "AA");
    REQUIRE(fatalErrorHit);

    resetDefaultErrorHandler();
}
