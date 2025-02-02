#include "database_connection.h"

/**
 * @brief Проверка подключения к базе данных.
 * 
 * Этот конструктор пытается открыть подключение к базе данных с использованием
 * переданной строки подключения `conn_info`. Если подключение не удается, генерируется
 * исключение `std::runtime_error`.
 * 
 * @param conn_info Строка подключения для базы данных PostgreSQL.
 * @throws std::runtime_error Если подключение не удалось открыть.
 */
DatabaseConnection::DatabaseConnection(const std::string& conn_info)
    : conn_(conn_info) {
    if (!conn_.is_open()) {
        throw std::runtime_error("Failed to open the database connection");
    }
}

/**
 * @brief Метод для сохранения пары короткий URL / длинный URL в базе данных.
 * 
 * Этот метод выполняет SQL-запрос для добавления новой пары короткий и длинный URL в таблицу `urls`.
 * Используется транзакция для обеспечения целостности данных.
 * 
 * @param short_url Короткий URL, который нужно сохранить.
 * @param long_url Оригинальный длинный URL, который будет связан с коротким URL.
 */
void DatabaseConnection::save_url(const std::string& short_url, const std::string& long_url) {
    pqxx::work txn(conn_);

    std::string query = "INSERT INTO urls (short_url, long_url) VALUES ('" + short_url + "', '" + long_url + "')";
    
    txn.exec(query);  
    txn.commit();
}

/**
 * @brief Метод для получения оригинального URL по короткому URL.
 * 
 * Этот метод выполняет SQL-запрос для поиска оригинального URL по переданному короткому URL.
 * Если результат не найден, возвращает пустую строку.
 * 
 * @param short_url Короткий URL, для которого необходимо найти оригинальный URL.
 * @return Оригинальный URL, если найден, или пустую строку, если результат не найден.
 */
std::string DatabaseConnection::get_original_url(const std::string& short_url) {
    pqxx::work txn(conn_);

    std::string query = "SELECT long_url FROM urls WHERE short_url = '" + short_url + "'";
    
    auto result = txn.exec(query);  
    if (result.empty()) {
        return "";
    }
    return result[0][0].as<std::string>();  
}
