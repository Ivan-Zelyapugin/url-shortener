#include "storage.h"
#include <random>
#include <stdexcept>
#include <iostream>

/**
 * @brief Конструктор для инициализации экземпляра класса Storage.
 * 
 * Конструктор принимает объект конфигурации, который используется для настройки
 * подключения к базе данных.
 * 
 * @param config Конфигурация для инициализации базы данных.
 */
Storage::Storage(const Config& config)
    : db_connection_(config.get_db_url()) { }

/**
 * @brief Получение экземпляра класса Storage (Одиночка).
 * 
 * Этот метод реализует паттерн "Одиночка", обеспечивая создание единственного экземпляра
 * класса `Storage` и его повторное использование в приложении.
 * 
 * @param config Конфигурация для создания экземпляра Storage.
 * @return Ссылку на единственный экземпляр объекта Storage.
 */
Storage& Storage::get_instance(const Config& config) {
    static Storage instance(config);
    return instance;
}

/**
 * @brief Простая хеш-функция для строк.
 * 
 * Эта хеш-функция реализует алгоритм DJB2 для вычисления хеша строки. Хеш используется
 * для генерации коротких URL.
 * 
 * @param str Строка, для которой нужно вычислить хеш.
 * @return Возвращает хеш в виде числа типа unsigned long.
 */
unsigned long Storage::simple_hash(const std::string& str) {
    unsigned long hash = 5381;
    for (char c : str) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

/**
 * @brief Сокращение URL.
 * 
 * Этот метод принимает длинный URL и сокращает его, либо используя указанный короткий путь,
 * либо генерируя новый уникальный короткий URL. Если короткий путь уже существует в базе данных,
 * выбрасывается исключение.
 * 
 * @param long_url Длинный URL, который нужно сократить.
 * @param short_path Опциональный короткий путь, если он передан.
 * @return Сокращенный URL.
 * @throws std::runtime_error Если указанный короткий путь уже существует.
 */
std::string Storage::shorten_url(const std::string& long_url, const std::string& short_path) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!short_path.empty()) {
        // Проверим, существует ли уже такой короткий URL в базе данных
        std::string existing_url = db_connection_.get_original_url(short_path);
        if (!existing_url.empty()) {
            throw std::runtime_error("The specified short path already exists.");
        }
        // Сохраняем новый короткий URL
        db_connection_.save_url(short_path, long_url);
        return short_path;
    }

    // Генерация нового короткого URL
    std::random_device rd;                              // Используем случайное число для генерации
    std::mt19937 gen(rd());                             // Инициализируем генератор случайных чисел
    std::uniform_int_distribution<> dis(1000, 9999);    // Диапазон случайных чисел
    std::string salt = std::to_string(dis(gen));        // Генерируем случайную строку для уникальности
    std::string input = long_url + salt;                

    unsigned long hash = simple_hash(input);
    std::ostringstream oss;
    oss << std::hex << hash;                            // Преобразуем хеш в шестнадцатеричную строку
    std::string short_url = oss.str().substr(0, 8);     // Обрезаем хеш до 8 символов

    db_connection_.save_url(short_url, long_url);
    return short_url;
}

/**
 * @brief Получение оригинального URL по короткому URL.
 * 
 * Этот метод извлекает оригинальный длинный URL по короткому URL из базы данных.
 * 
 * @param short_url Короткий URL, для которого нужно получить оригинальный.
 * @return Оригинальный длинный URL.
 */
std::string Storage::get_original_url(const std::string& short_url) {
    std::lock_guard<std::mutex> lock(mutex_);
    // Получаем оригинальный URL из базы данных
    return db_connection_.get_original_url(short_url);
}
