#include "../hpp/exchange_rates.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <iterator>

std::chrono::system_clock::time_point exchange_rates::make_date(int year, int month, int day) {
    std::tm tm = {};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

int exchange_rates::get_year(const std::chrono::system_clock::time_point& date) {
    std::time_t time = std::chrono::system_clock::to_time_t(date);
    std::tm* tm = std::localtime(&time);
    return (*tm).tm_year + 1900; 
}

int exchange_rates::get_month(const std::chrono::system_clock::time_point& date) {
    std::time_t time = std::chrono::system_clock::to_time_t(date);
    std::tm* tm = std::localtime(&time);
    return (*tm).tm_mon + 1;
}

int exchange_rates::get_day(const std::chrono::system_clock::time_point& date) {
    std::time_t time = std::chrono::system_clock::to_time_t(date);
    std::tm* tm = std::localtime(&time);
    return (*tm).tm_mday;
}

exchange_rates::exchange_rates() : base_currency_(""), currencies_(), data_() {
}

exchange_rates::exchange_rates(const char* path, const char* base_currency) 
    : base_currency_(""), currencies_(), data_() {
    from_csv(path, base_currency);
}

exchange_rates::exchange_rates(const exchange_rates& other) 
    : base_currency_(other.base_currency_),
      currencies_(other.currencies_),
      data_(other.data_) {
}

exchange_rates& exchange_rates::operator=(const exchange_rates& other) {
    if (this != &other) {
        base_currency_ = other.base_currency_;
        currencies_ = other.currencies_;
        data_ = other.data_;
    }
    return *this;
}

exchange_rates::~exchange_rates() {}

std::string exchange_rates::to_upper(const std::string& str) const {
    std::string result = str;
    for (int i = 0; i < (int)result.length(); ++i) {
        result[i] = std::toupper(result[i]);
    }
    return result;
}

std::chrono::system_clock::time_point exchange_rates::parse_date(const std::string& date_str) const {
    if (date_str.length() != 10 || date_str[4] != '-' || date_str[7] != '-') {
        throw std::runtime_error("Invalid date format: " + date_str);
    }
    
    int year = std::stoi(date_str.substr(0, 4));
    int month = std::stoi(date_str.substr(5, 2));
    int day = std::stoi(date_str.substr(8, 2));
    
    return make_date(year, month, day);
}

double exchange_rates::parse_rate(const std::string& rate_str) const {
    if (rate_str == "N/A" || rate_str == "") {
        return 0.0 / 0.0;
    }
    return std::stod(rate_str);
}

int exchange_rates::find_currency_index(const char* currency) const {
    std::string currency_str;
    for (const char* p = currency; *p != '\0'; ++p) {
        char c = *p;
        if (c >= 'a' && c <= 'z') {
            c = c - 'a' + 'A';
        }
        currency_str += c;
    }
    
    int left = 0;
    int right = (int)currencies_.size();
    
    while (left < right) {
        int mid = left + (right - left) / 2;
        if (currencies_[mid] < currency_str) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    
    if (left < (int)currencies_.size() && currencies_[left] == currency_str) {
        return left;
    }
    
    return -1;
}

void exchange_rates::from_csv(const char* path, const char* base_currency) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + std::string(path));
    }
    
    std::string temp_base_currency = to_upper(base_currency);
    std::vector<std::string> temp_currencies;
    std::vector<DateData> temp_data;
    
    try {
        std::string line;
        
        if (!std::getline(file, line)) {
            throw std::runtime_error("Empty file or cannot read header");
        }
        
        std::istringstream header_stream(line);
        std::string cell;
        bool first = true;
        while (std::getline(header_stream, cell, ',')) {
            if (first) {
                first = false;
                continue;
            }
            
            std::string currency_code = to_upper(cell);
            if (!currency_code.empty()) {
                temp_currencies.push_back(currency_code);
            }
        }
        
        if (temp_currencies.empty()) {
            throw std::runtime_error("No currencies found in file header");
        }
        
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            
            std::istringstream row_stream(line);
            std::string cell;
            
            if (!std::getline(row_stream, cell, ',')) {
                continue;
            }
            
            try {
                std::chrono::system_clock::time_point date = parse_date(cell);
                DateData date_data(date, temp_currencies.size());
                
                int currency_idx = 0;
                while (std::getline(row_stream, cell, ',') && currency_idx < (int)temp_currencies.size()) {
                    try {
                        date_data.rates[currency_idx] = parse_rate(cell);
                    } catch (const std::exception&) {
                    }
                    currency_idx++;
                }
                
                temp_data.push_back(date_data);
            } catch (const std::exception&) {
                continue;
            }
        }
        
        std::vector<int> sort_order(temp_currencies.size());
        for (int i = 0; i < (int)temp_currencies.size(); ++i) {
            sort_order[i] = i;
        }
        
        std::sort(sort_order.begin(), sort_order.end(), 
                  [&temp_currencies](int a, int b) {
                      return temp_currencies[a] < temp_currencies[b];
                  });
        
        std::vector<std::string> sorted_currencies(temp_currencies.size());
        for (int i = 0; i < (int)temp_currencies.size(); ++i) {
            sorted_currencies[i] = temp_currencies[sort_order[i]];
        }
        
        for (int row = 0; row < (int)temp_data.size(); ++row) {
            std::vector<double> old_rates = temp_data[row].rates;
            for (int i = 0; i < (int)temp_currencies.size(); ++i) {
                temp_data[row].rates[i] = old_rates[sort_order[i]];
            }
        }
        
        std::sort(temp_data.begin(), temp_data.end(), 
                  [](const DateData& a, const DateData& b) {
                      return a.date > b.date;
                  });
        
        base_currency_ = temp_base_currency;
        currencies_ = sorted_currencies;
        data_ = temp_data;
        
    } catch (const std::bad_alloc&) {
        file.close();
        throw;
    } catch (const std::exception&) {
        file.close();
        throw std::runtime_error("Error reading file: " + std::string(path));
    }
    
    file.close();
}

int exchange_rates::find_date_index(const std::chrono::system_clock::time_point& date) const {
    if (data_.empty()) {
        return -1;
    }
    
    int left = 0;
    int right = (int)data_.size();
    int result = (int)data_.size() - 1;
    
    while (left < right) {
        int mid = left + (right - left) / 2;
        
        if (data_[mid].date <= date) {
            result = mid;
            right = mid;
        } else {
            left = mid + 1;
        }
    }
    
    return result;
}

double exchange_rates::for_date(const std::chrono::system_clock::time_point& date, const char* currency) const {
    if (data_.empty()) {
        return 0.0 / 0.0;
    }
    
    std::string currency_upper = to_upper(currency);
    
    if (currency_upper == base_currency_) {
        return 1.0;
    }

    int currency_idx = find_currency_index(currency);
    if (currency_idx == -1) {
        throw std::runtime_error("Unsupported currency: " + std::string(currency));
    }
    
    int date_idx = find_date_index(date);
    if (date_idx == -1) {
        return 0.0 / 0.0;
    }
    
    return data_[date_idx].rates[currency_idx];
}

const char* exchange_rates::base_currency() const {
    return base_currency_.c_str();
}

bool exchange_rates::is_supported(const char* currency) const {
    std::string currency_str;
    for (const char* p = currency; *p != '\0'; ++p) {
        char c = *p;
        if (c >= 'a' && c <= 'z') {
            c = c - 'a' + 'A';
        }
        currency_str += c;
    }
    
    if (currency_str == base_currency_) {
        return true;
    }
    
    int left = 0;
    int right = (int)currencies_.size();
    
    while (left < right) {
        int mid = left + (right - left) / 2;
        if (currencies_[mid] < currency_str) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    
    return (left < (int)currencies_.size() && currencies_[left] == currency_str);
}

size_t exchange_rates::entry_count() const {
    return data_.size();
}
