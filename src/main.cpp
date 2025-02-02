#include "server.h"
#include "config.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <pqxx/pqxx>

/**
 * @brief Главная функция для запуска сервера.
 * 
 * Эта функция проверяет аргументы командной строки, устанавливает строку подключения к базе данных,
 * ожидает несколько секунд, чтобы убедиться, что база данных доступна, и затем запускает сервер.
 * 
 * @param argc Количество аргументов командной строки.
 * @param argv Массив строк с аргументами командной строки.
 * @return Возвращает 0 при успешном завершении, или 1, если возникла ошибка в аргументах командной строки.
 */
int main(int argc, char* argv[]) {
    // Проверяем наличие аргументов командной строки
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <address> <port>\n";
        return 1;
    }

    std::string address = argv[1];      // IP-адрес сервера
    int port = std::stoi(argv[2]);     // Порт для сервера

    // Строка подключения к базе данных PostgreSQL
    std::string db_url = "postgres://postgres:root@db:5432/url_shortener";

    // Ожидание, пока база данных не станет доступной
    std::this_thread::sleep_for(std::chrono::seconds(4));

    Config config(db_url);
    
    Server server(address, port, config);

    // Запускаем сервер в отдельном потоке
    std::thread server_thread([&]() {
        server.run();  
    });

    std::cout << "Server is running" << std::endl;

    server_thread.join();  

    return 0;
}


