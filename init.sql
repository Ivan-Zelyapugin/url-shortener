-- Проверяем, существует ли база данных
DO
$$
BEGIN
    IF NOT EXISTS (SELECT FROM pg_database WHERE datname = 'url_shortener') THEN
        CREATE DATABASE url_shortener;
    END IF;
END
$$;

-- Подключаемся к базе данных
\c url_shortener

-- Проверяем, существует ли таблица urls
DO
$$
BEGIN
    IF NOT EXISTS (SELECT 1 FROM information_schema.tables WHERE table_name = 'urls') THEN
        CREATE TABLE urls (
            short_url VARCHAR(255) PRIMARY KEY,
            long_url TEXT NOT NULL
        );
    END IF;
END
$$;

