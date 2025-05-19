#ifndef STOCK_MANAGER_HPP
#define STOCK_MANAGER_HPP

#include <string>
#include <hiredis/hiredis.h>

class StockManager
{
public:
    StockManager(const std::string &host, int port);
    ~StockManager();

    void insertStockData(const std::string &stockSymbol, double price, int quantity,
                         double &totalPreparationTime, double &totalInsertionTime);
    void retrieveStockData(const std::string &stockSymbol);
    void displayAllStockData();
    void deleteAllStockData();

private:
    redisContext *_context;
    const std::string _stockListKey = "stock:queue";
};

// Clean main interface
void runStockApp();

#endif
