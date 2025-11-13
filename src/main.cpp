#include "hpp/exchange_rates.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <cmath>

std::chrono::system_clock::time_point parse_date(const std::string& date_str) {
    if (date_str.length() != 10 || date_str[4] != '-' || date_str[7] != '-') {
        throw std::runtime_error("Invalid date format");
    }
    
    int year = std::stoi(date_str.substr(0, 4));
    int month = std::stoi(date_str.substr(5, 2));
    int day = std::stoi(date_str.substr(8, 2));
    
    return exchange_rates::make_date(year, month, day);
}

int main() {
    try {
        exchange_rates rates("../src/eurofxref-hist.csv", "EUR");
        std::cout << "Exchange rates loaded successfully." << std::endl << std::endl;
        
        std::cout << "Please, enter a query, or \"exit\" to leave." << std::endl << std::endl;
        
        std::string line;
        while (true) {
            std::cout << "> ";
            if (!std::getline(std::cin, line)) {
                break;
            }
            
            if (line == "exit" || line == "Exit") {
                std::cout << "OK, bye!" << std::endl;
                break;
            }
            
            try {
                std::istringstream iss(line);
                std::string date_str, currency1, currency2;
                
                if (!(iss >> date_str >> currency1 >> currency2)) {
                    std::cout << "Invalid input format. Expected: YYYY-MM-DD CURRENCY1 CURRENCY2" << std::endl;
                    continue;
                }
                
                std::chrono::system_clock::time_point query_date = parse_date(date_str);
                
                double rate1, rate2;
                
                try {
                    rate1 = rates.for_date(query_date, currency1.c_str());
                } catch (const std::runtime_error&) {
                    std::cout << "Unsupported currency." << std::endl;
                    continue;
                }
                
                try {
                    rate2 = rates.for_date(query_date, currency2.c_str());
                } catch (const std::runtime_error&) {
                    std::cout << "Unsupported currency." << std::endl;
                    continue;
                }
                
                if (std::isnan(rate1) || std::isnan(rate2)) {
                    std::cout << "There is no information for those currencies on the specified date." << std::endl;
                    continue;
                }
                
                double exchange_rate;
                if (currency1 == "EUR" || currency1 == "eur") {
                    exchange_rate = rate2;
                } else if (currency2 == "EUR" || currency2 == "eur") {
                    exchange_rate = 1.0 / rate1;
                } else {
                    exchange_rate = rate2 / rate1;
                }
                
                std::cout.precision(4);
                std::cout << std::fixed << exchange_rate << std::endl;
                
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cout << "Error loading exchange rates: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
