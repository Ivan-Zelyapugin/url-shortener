#include "server.h"
#include "storage.h"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * @brief Инициализация сервера.
 * 
 * Конструктор инициализирует объект сервера, устанавливая адрес и порт, на котором сервер будет слушать запросы.
 * Также инициализируется объект для работы с асинхронным вводом-выводом.
 * 
 * @param address Адрес, на котором сервер будет слушать соединения.
 * @param port Порт для соединений.
 * @param config Конфигурация сервера.
 */
Server::Server(const std::string& address, int port, const Config& config)
    : io_context_(), 
    acceptor_(io_context_, {boost::asio::ip::make_address(address), static_cast<unsigned short>(port)}), config_(config) {}

/**
 * @brief Запуск сервера.
 * 
 * Эта функция запускает сервер, начинает принимать соединения и инициирует асинхронную обработку запросов.
 */
void Server::run() {
    do_accept();
    io_context_.run();
}

/**
 * @brief Формирование JSON-ответа.
 * 
 * Эта функция создает и возвращает строку JSON, содержащую статус и пару "ключ-значение".
 * 
 * @param status Статус ответа (например, "success").
 * @param key Ключ для данных.
 * @param value Значение, связанное с ключом.
 * @return Строка, представляющая JSON-ответ.
 */
std::string Server::make_json_response(const std::string& status, const std::string& key, const std::string& value) {
    // Создаем объект JSON и заполняем его
    json response;
    response["status"] = status;
    response[key] = value;

    // Преобразуем объект JSON в строку и возвращаем её
    return response.dump();
}

/**
 * @brief Формирование JSON-ответа об ошибке.
 * 
 * Эта функция создает и возвращает строку JSON с ошибкой, содержащую статус "error" и сообщение об ошибке.
 * 
 * @param message Сообщение об ошибке.
 * @return Строка, представляющая JSON-ответ об ошибке.
 */
std::string Server::make_json_error(const std::string& message) {
    // Создаем объект JSON для ошибки
    json error_response;
    error_response["status"] = "error";
    error_response["message"] = message;

    // Преобразуем объект JSON в строку и возвращаем её
    return error_response.dump();
}

/**
 * @brief Извлечение значения из строки JSON.
 * 
 * Эта функция извлекает значение по указанному ключу из строки JSON. Если ключ не найден или
 * возникает ошибка при парсинге, возвращается пустая строка.
 * 
 * @param json_str Строка в формате JSON.
 * @param key Ключ, по которому нужно извлечь значение.
 * @return Значение, связанное с ключом, или пустая строка, если ключ не найден.
 */
std::string Server::extract_json_value(const std::string& json_str, const std::string& key) {
    try {
        json parsed_json = json::parse(json_str);

        if (parsed_json.contains(key)) {
            return parsed_json[key].get<std::string>();  
        } else {
            std::cerr << "Key not found: " << key << std::endl;
            return "";  
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return "";  
    }
}

/**
 * @brief Ожидание и принятие входящих соединений.
 * 
 * Эта функция асинхронно ожидает входящих соединений и передает их на обработку, вызывая `handle_request`.
 * После каждого принятого соединения, функция снова ожидает следующее подключение.
 */
void Server::do_accept() {
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
    acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec) {
        if (!ec) {
            std::cout << "Connection accepted" << std::endl;
            handle_request(socket);
        } else {
            std::cerr << "Error while accepting connection: " << ec.message() << std::endl;
        }
        do_accept();
    });
}

/**
 * @brief Обработка HTTP-запроса.
 * 
 * Эта функция читает запрос от клиента, обрабатывает его и отправляет ответ. В зависимости от метода запроса
 * (POST или GET) и пути, она вызывает соответствующие действия: сокращение URL или редирект.
 * 
 * @param socket Сокет, через который получен запрос и отправляется ответ.
 */
void Server::handle_request(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    // Создаем буфер для хранения данных HTTP-запроса
    auto buffer = std::make_shared<boost::beast::flat_buffer>();

    // Создаем объект для хранения HTTP-запроса
    auto request = std::make_shared<http::request<http::string_body>>();

    // Асинхронно читаем запрос с сокета
    http::async_read(*socket, *buffer, *request, [this, socket, buffer, request](boost::system::error_code ec, std::size_t) {
        if (!ec) {
            auto response = std::make_shared<http::response<http::string_body>>();
            try {
                if (request->method() == http::verb::post && request->target() == "/shorten") {
                    // Если метод POST и целевой путь "/shorten", обрабатываем запрос на сокращение URL
                    std::string body = request->body();
                    std::string long_url = extract_json_value(body, "url");
                    std::string short_path = extract_json_value(body, "short_path");

                    if (long_url.empty()) {
                        // Если не указан long_url, выбрасываем исключение
                        std::cerr << "URL is missing in the request body." << std::endl;
                        throw std::runtime_error("Missing URL in request body");
                    }

                    std::string short_url = short_path.empty() ? 
                        Storage::get_instance(config_).shorten_url(long_url) :
                        Storage::get_instance(config_).shorten_url(long_url, short_path);

                    response->result(http::status::ok);
                    response->set(http::field::content_type, "application/json");
                    response->body() = make_json_response("success", "short_url", short_url);
                }
                else if (request->method() == http::verb::get && request->target().find("/shorten/") == 0) {
                    // Если метод GET и целевой путь начинается с "/shorten/", обрабатываем запрос на редирект

                    // Убираем "/shorten/" из пути запроса
                    std::string short_url(request->target().substr(9));  

                    std::string long_url = Storage::get_instance(config_).get_original_url(short_url);

                    if (!long_url.empty()) {
                        // Если оригинальный URL найден, выполняем редирект
                        response->result(http::status::moved_permanently);  
                        response->set(http::field::location, long_url);  
                        response->body() = make_json_response("success", "short_url", short_url); 
                    } else {
                        // Если оригинальный URL не найден, возвращаем ошибку
                        response->result(http::status::not_found);
                        response->set(http::field::content_type, "application/json");
                        response->body() = make_json_error("Short URL not found");
                    }
                } 
                else {
                    // Если запрос не соответствует ожидаемым методам и путям, возвращаем ошибку
                    std::cerr << "Invalid method or target" << std::endl;
                    response->result(http::status::bad_request);
                    response->set(http::field::content_type, "application/json");
                    response->body() = make_json_error("Invalid request method or target");
                }
            } catch (const std::exception& e) {
                // Ловим исключения, если произошла ошибка в обработке запроса
                std::cerr << "Exception during request handling: " << e.what() << std::endl;
                response->result(http::status::internal_server_error);
                response->set(http::field::content_type, "application/json");
                response->body() = make_json_error("Server error");
            }

            // Подготавливаем ответ и отправляем его
            response->prepare_payload();
            http::async_write(*socket, *response, [socket, response](boost::system::error_code, std::size_t) {
                std::cout << "Response send" << std::endl;
            });
        } else {
            // Логируем ошибку, если произошла ошибка при чтении запроса
            std::cerr << "Error while reading request: " << ec.message() << std::endl;
        }
    });
}




