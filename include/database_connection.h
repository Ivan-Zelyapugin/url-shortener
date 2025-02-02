#pragma once
#include <pqxx/pqxx>
#include <stdexcept>

/**
 * @class DatabaseConnection
 * @brief Класс для работы с подключением к базе данных PostgreSQL с использованием библиотеки libpqxx.
 * 
 * Этот класс инкапсулирует соединение с базой данных PostgreSQL, позволяя сохранять и извлекать URL-адреса.
 */
class DatabaseConnection {
public:
    /**
     * @brief Конструктор, создающий подключение к базе данных.
     * 
     * Этот конструктор инициализирует объект `DatabaseConnection` с использованием строки
     * подключения, переданной в параметре `conn_info`. Использует библиотеку pqxx для создания
     * подключения к базе данных.
     * 
     * @param conn_info Строка подключения для установления соединения с PostgreSQL.
     */
    DatabaseConnection(const std::string& conn_info);

    /**
     * @brief Метод для сохранения короткой и длинной версии URL в базе данных.
     * 
     * Этот метод сохраняет пару короткий URL / оригинальный URL в базе данных.
     * 
     * @param short_url Короткая версия URL, которая будет использоваться для доступа.
     * @param long_url Оригинальный URL, который связан с коротким URL.
     */
    void save_url(const std::string& short_url, const std::string& long_url);

    /**
     * @brief Метод для извлечения оригинального URL по короткому URL.
     * 
     * Этот метод возвращает оригинальный URL, связанный с коротким URL.
     * 
     * @param short_url Короткий URL, для которого необходимо получить оригинальный URL.
     * @return Оригинальный URL.
     */
    std::string get_original_url(const std::string& short_url);

private:
/**
     * @brief Приватная переменная для хранения подключения к базе данных.
     * 
     * Этот объект типа `pqxx::connection` используется для установления и управления
     * подключением к базе данных PostgreSQL. Он выполняет роль интерфейса для взаимодействия
     * с базой данных и выполнения SQL-запросов.
     * 
     */
    pqxx::connection conn_;
};
