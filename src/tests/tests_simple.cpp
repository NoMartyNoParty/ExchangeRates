#define CATCH_CONFIG_MAIN
#include "../catch.hpp"

#include "../hpp/exchange_rates.hpp"
#include <fstream>
#include <cmath>

TEST_CASE("Date Equality Test") {
    auto d1 = exchange_rates::make_date(2025, 1, 15);
    auto d2 = exchange_rates::make_date(2025, 1, 15);
    
    REQUIRE(d1 == d2);
}

TEST_CASE("Date Comparison Test") {
    auto d1 = exchange_rates::make_date(2025, 1, 15);
    auto d2 = exchange_rates::make_date(2025, 1, 16);
    
    REQUIRE(d1 < d2);
    REQUIRE(d2 > d1);
}

TEST_CASE("Default Constructor Test") {
    exchange_rates rates;
    
    REQUIRE(std::string(rates.base_currency()) == "");
    REQUIRE(!rates.is_supported("USD"));
}

TEST_CASE("Load CSV File Test") {
    std::ofstream file("test.csv");
    file << "Date,USD,GBP\n";
    file << "2025-01-15,1.1000,0.8500\n";
    file.close();
    
    exchange_rates rates;
    rates.from_csv("test.csv", "EUR");
    
    REQUIRE(std::string(rates.base_currency()) == "EUR");
    REQUIRE(rates.is_supported("USD"));
    REQUIRE(rates.is_supported("GBP"));
}

TEST_CASE("Get Exchange Rate Test") {
    std::ofstream file("test2.csv");
    file << "Date,USD\n";
    file << "2025-01-15,1.1000\n";
    file.close();    
    exchange_rates rates;
    rates.from_csv("test2.csv", "EUR");
    
    auto test_date = exchange_rates::make_date(2025, 1, 15);
    double usd_rate = rates.for_date(test_date, "USD");
    
    REQUIRE(std::abs(usd_rate - 1.1000) < 0.0001);
}

TEST_CASE("Case Insensitive Currency Test") {
    std::ofstream file("test4.csv");
    file << "Date,USD\n";
    file << "2025-01-15,1.1000\n";
    file.close();
    
    exchange_rates rates;
    rates.from_csv("test4.csv", "EUR");
    
    REQUIRE(rates.is_supported("USD"));
    REQUIRE(rates.is_supported("usd"));
    REQUIRE(rates.is_supported("Usd"));
}

TEST_CASE("Copy Constructor Test") {
    std::ofstream file("test5.csv");
    file << "Date,USD\n";
    file << "2025-01-15,1.1000\n";
    file.close();
    
    exchange_rates rates1;
    rates1.from_csv("test5.csv", "EUR");
    
    exchange_rates rates2(rates1);
    
    REQUIRE(std::string(rates2.base_currency()) == "EUR");
    REQUIRE(rates2.is_supported("USD"));
}

TEST_CASE("Unsupported Currency Test") {
    std::ofstream file("test8.csv");
    file << "Date,USD\n";
    file << "2025-01-15,1.1000\n";
    file.close();
    
    exchange_rates rates;
    rates.from_csv("test8.csv", "EUR");
    
    REQUIRE(rates.is_supported("USD"));
    REQUIRE(!rates.is_supported("JPY"));
}
