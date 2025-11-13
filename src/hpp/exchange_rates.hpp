#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <ctime>

class exchange_rates {
public:
    static std::chrono::system_clock::time_point make_date(int year, int month, int day);
    static int get_year(const std::chrono::system_clock::time_point& date);
    static int get_month(const std::chrono::system_clock::time_point& date);
    static int get_day(const std::chrono::system_clock::time_point& date);

private:
    struct DateData {
        std::chrono::system_clock::time_point date;
        std::vector<double> rates;
        
        DateData(const std::chrono::system_clock::time_point& d, size_t currency_count) 
            : date(d), rates(currency_count, 0.0 / 0.0) {}
    };
      
    std::string base_currency_;
    std::vector<std::string> currencies_;
    std::vector<DateData> data_;
    
    int find_date_index(const std::chrono::system_clock::time_point& date) const;
    int find_currency_index(const char* currency) const;
    std::string to_upper(const std::string& str) const;
    std::chrono::system_clock::time_point parse_date(const std::string& date_str) const;
    double parse_rate(const std::string& rate_str) const;

public:
    exchange_rates();
    exchange_rates(const char* path, const char* base_currency);
    exchange_rates(const exchange_rates& other);
    exchange_rates& operator=(const exchange_rates& other);
    ~exchange_rates();

    void from_csv(const char* path, const char* base_currency);
    double for_date(const std::chrono::system_clock::time_point& date, const char* currency) const;
    const char* base_currency() const;
    bool is_supported(const char* currency) const;
};
