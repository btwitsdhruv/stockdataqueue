

#include <iostream>
#include <hiredis/hiredis.h>
#include <iomanip>
#include <sstream>
#include <vector>
#include <chrono>
#include <string>

const std::string STOCK_LIST_KEY = "stock:queue";

void retrieveStockData(redisContext *c, const std::string &stockSymbol)
{
    redisReply *reply = (redisReply *)redisCommand(c, "LRANGE %s 0 -1", STOCK_LIST_KEY.c_str());

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

void displayAllStockData(redisContext *c)
{
    redisReply *reply = (redisReply *)redisCommand(c, "LRANGE %s 0 -1", STOCK_LIST_KEY.c_str());

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

void insertStockData(redisContext *c, const std::string &stockSymbol, double price, int quantity, double &totalPreparationTime, double &totalInsertionTime)
{
    for (int i = 1; i <= quantity; ++i)
    {
        // Prepare data
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

        // Insert into Redis
        start = std::chrono::high_resolution_clock::now();
        redisReply *reply = (redisReply *)redisCommand(c, "RPUSH %s %s", STOCK_LIST_KEY.c_str(), jsonData.c_str());
        end = std::chrono::high_resolution_clock::now();
        totalInsertionTime += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        if (reply)
            freeReplyObject(reply);
    }
}

void deleteAllStockData(redisContext *c)
{
    redisReply *reply = (redisReply *)redisCommand(c, "DEL %s", STOCK_LIST_KEY.c_str());
    if (reply && reply->type == REDIS_REPLY_INTEGER && reply->integer > 0)
    {
        std::cout << "Deleted list key: " << STOCK_LIST_KEY << "\n";
    }
    else
    {
        std::cout << "No stock data found to delete.\n";
    }

    if (reply)
        freeReplyObject(reply);
}

int main()
{
    redisContext *c = redisConnect("127.0.0.1", 6379);

    if (c == nullptr || c->err)
    {
        std::cerr << "Redis connection error: " << (c ? c->errstr : "unknown") << "\n";
        return 1;
    }

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

            insertStockData(c, symbol, price, quantity, totalPreparationTime, totalInsertionTime);

            std::cout << "\nInserted " << quantity << " entries for symbol: " << symbol << "\n";
            // std::cout << "Total preparation time: " << totalPreparationTime << " microseconds\n";
            std::cout << "Total insertion time: " << totalInsertionTime << " microseconds\n";

            break;
        }
        case 2:
        {
            std::string symbol;
            std::cout << "Enter stock symbol to retrieve: ";
            std::cin >> symbol;
            retrieveStockData(c, symbol);
            break;
        }
        case 3:
        {
            displayAllStockData(c);
            break;
        }
        case 4:
        {
            deleteAllStockData(c);
            break;
        }
        case 5:
        {
            std::cout << "Exiting program.\n";
            break;
        }
        default:
            std::cout << "Invalid choice. Please try again.\n";
        }

    } while (choice != 5);

    redisFree(c);
    return 0;
}
