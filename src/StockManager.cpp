#include "StockManager.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>

StockManager::StockManager(const std::string &host, int port)
{
    _context = redisConnect(host.c_str(), port);
    if (_context == nullptr || _context->err)
    {
        std::cerr << "Redis connection error: " << (_context ? _context->errstr : "unknown") << std::endl;
        exit(1);
    }
}

StockManager::~StockManager()
{
    if (_context)
    {
        redisFree(_context);
    }
}

void StockManager::insertStockData(const std::string &stockSymbol, double price, int quantity,
                                   double &totalPreparationTime, double &totalInsertionTime)
{
    for (int i = 1; i <= quantity; ++i)
    {
        auto start = std::chrono::high_resolution_clock::now();

        std::ostringstream dataStream;
        dataStream << std::fixed << std::setprecision(2);
        dataStream << "{"
                   << "\"symbol\":\"" << stockSymbol << "\","
                   << "\"price\":" << price << ","
                   << "\"quantity\":" << i << "}";

        std::string jsonData = dataStream.str();
        auto end = std::chrono::high_resolution_clock::now();
        totalPreparationTime += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        start = std::chrono::high_resolution_clock::now();
        redisReply *reply = (redisReply *)redisCommand(_context, "RPUSH %s %s", _stockListKey.c_str(), jsonData.c_str());
        end = std::chrono::high_resolution_clock::now();
        totalInsertionTime += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        if (reply)
            freeReplyObject(reply);
    }
}

void StockManager::retrieveStockData(const std::string &stockSymbol)
{
    redisReply *reply = (redisReply *)redisCommand(_context, "LRANGE %s 0 -1", _stockListKey.c_str());

    if (reply && reply->type == REDIS_REPLY_ARRAY && reply->elements > 0)
    {
        std::cout << "\nStock Data for " << stockSymbol << ":\n";
        bool found = false;
        for (size_t i = 0; i < reply->elements; ++i)
        {
            std::string entry = reply->element[i]->str;
            if (entry.find("\"symbol\":\"" + stockSymbol + "\"") != std::string::npos)
            {
                std::cout << entry << "\n";
                found = true;
            }
        }
        if (!found)
        {
            std::cout << "No data found for " << stockSymbol << "\n";
        }
    }
    else
    {
        std::cout << "No stock data found.\n";
    }

    if (reply)
        freeReplyObject(reply);
}

void StockManager::displayAllStockData()
{
    redisReply *reply = (redisReply *)redisCommand(_context, "LRANGE %s 0 -1", _stockListKey.c_str());

    if (!reply || reply->type != REDIS_REPLY_ARRAY || reply->elements == 0)
    {
        std::cout << "No stock data found in Redis.\n";
        if (reply)
            freeReplyObject(reply);
        return;
    }

    std::cout << "\n========== All Stock Entries ==========\n";
    for (size_t i = 0; i < reply->elements; ++i)
    {
        std::cout << "Entry [" << i + 1 << "]: " << reply->element[i]->str << "\n";
    }
    std::cout << "========== Total Entries: " << reply->elements << " ==========\n";

    if (reply)
        freeReplyObject(reply);
}

void StockManager::deleteAllStockData()
{
    redisReply *reply = (redisReply *)redisCommand(_context, "DEL %s", _stockListKey.c_str());

    if (reply && reply->type == REDIS_REPLY_INTEGER && reply->integer > 0)
    {
        std::cout << "Deleted list key: " << _stockListKey << "\n";
    }
    else
    {
        std::cout << "No stock data found to delete.\n";
    }

    if (reply)
        freeReplyObject(reply);
}



#include "StockManager.hpp"
#include <iostream>

void runStockApp()
{
    StockManager manager("127.0.0.1", 6379);

    int choice;
    double totalPreparationTime = 0.0;
    double totalInsertionTime = 0.0;

    do
    {
        std::cout << "\nPlease choose an option:\n";
        std::cout << "1. Insert stock data\n";
        std::cout << "2. Retrieve stock data by symbol\n";
        std::cout << "3. Display all stock data\n";
        std::cout << "4. Delete all stock data\n";
        std::cout << "5. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice)
        {
        case 1:
        {
            std::string symbol;
            double price;
            int quantity;

            std::cout << "Enter stock symbol: ";
            std::cin >> symbol;
            std::cout << "Enter price: ";
            std::cin >> price;
            std::cout << "Enter quantity: ";
            std::cin >> quantity;

            manager.insertStockData(symbol, price, quantity, totalPreparationTime, totalInsertionTime);
            std::cout << "\nInserted " << quantity << " entries for symbol: " << symbol << "\n";
            std::cout << "Total insertion time: " << totalInsertionTime << " microseconds\n";
            break;
        }
        case 2:
        {
            std::string symbol;
            std::cout << "Enter stock symbol to retrieve: ";
            std::cin >> symbol;
            manager.retrieveStockData(symbol);
            break;
        }
        case 3:
            manager.displayAllStockData();
            break;
        case 4:
            manager.deleteAllStockData();
            break;
        case 5:
            std::cout << "Exiting program.\n";
            break;
        default:
            std::cout << "Invalid choice. Please try again.\n";
        }

    } while (choice != 5);
}
