-- =============================================================
-- Информационная система для ведения заказов компанией
-- Физическая модель БД (SQLite)
-- =============================================================

PRAGMA foreign_keys = ON;

-- 1. Роли (Roles)
CREATE TABLE IF NOT EXISTS roles (
    role_id INTEGER PRIMARY KEY AUTOINCREMENT,
    role_name TEXT NOT NULL UNIQUE
);

-- 2. Пользователи (Users)
CREATE TABLE IF NOT EXISTS users (
    user_id INTEGER PRIMARY KEY AUTOINCREMENT,
    role_id INTEGER NOT NULL REFERENCES roles(role_id) ON DELETE RESTRICT ON UPDATE CASCADE,
    email TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL
);

-- 3. Заказчики (Customers)
CREATE TABLE IF NOT EXISTS customers (
    customer_id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL UNIQUE REFERENCES users(user_id) ON DELETE CASCADE ON UPDATE CASCADE,
    organization_name TEXT NOT NULL,
    contact_person TEXT NOT NULL,
    address TEXT NOT NULL,
    phone TEXT NOT NULL UNIQUE
);

-- 4. Заказы (Orders)
CREATE TABLE IF NOT EXISTS orders (
    order_id INTEGER PRIMARY KEY AUTOINCREMENT,
    customer_id INTEGER NOT NULL REFERENCES customers(customer_id) ON DELETE CASCADE ON UPDATE CASCADE,
    purchase_date TEXT NOT NULL DEFAULT (CURRENT_DATE)
);

-- 5. Товары (Products)
CREATE TABLE IF NOT EXISTS products (
    product_id INTEGER PRIMARY KEY AUTOINCREMENT,
    product_name TEXT NOT NULL,
    price REAL NOT NULL CHECK (price >= 0),
    reference_info TEXT,
    is_delivery_available INTEGER NOT NULL DEFAULT 1
);

-- 6. Способы доставки (Delivery Methods)
CREATE TABLE IF NOT EXISTS delivery_methods (
    delivery_method_id INTEGER PRIMARY KEY AUTOINCREMENT,
    method_name TEXT NOT NULL UNIQUE,
    speed TEXT NOT NULL
);

-- 7. Тарифы доставки (Delivery Tariffs)
CREATE TABLE IF NOT EXISTS delivery_tariffs (
    delivery_tariff_id INTEGER PRIMARY KEY AUTOINCREMENT,
    product_id INTEGER NOT NULL REFERENCES products(product_id) ON DELETE CASCADE ON UPDATE CASCADE,
    delivery_method_id INTEGER NOT NULL REFERENCES delivery_methods(delivery_method_id) ON DELETE CASCADE ON UPDATE CASCADE,
    base_delivery_price REAL NOT NULL CHECK (base_delivery_price >= 0),
    UNIQUE (product_id, delivery_method_id)
);

-- 8. Позиции заказа (Order Items)
CREATE TABLE IF NOT EXISTS order_items (
    order_item_id INTEGER PRIMARY KEY AUTOINCREMENT,
    order_id INTEGER NOT NULL REFERENCES orders(order_id) ON DELETE CASCADE ON UPDATE CASCADE,
    product_id INTEGER NOT NULL REFERENCES products(product_id) ON DELETE RESTRICT ON UPDATE CASCADE,
    delivery_method_id INTEGER NOT NULL REFERENCES delivery_methods(delivery_method_id) ON DELETE RESTRICT ON UPDATE CASCADE,
    actual_delivery_cost REAL NOT NULL CHECK (actual_delivery_cost >= 0),
    quantity INTEGER NOT NULL CHECK (quantity > 0)
);

-- =============================================================
-- Начальные данные
-- =============================================================

INSERT INTO roles (role_name) VALUES ('admin') ON CONFLICT (role_name) DO NOTHING;
INSERT INTO roles (role_name) VALUES ('customer') ON CONFLICT (role_name) DO NOTHING;
INSERT INTO roles (role_name) VALUES ('user') ON CONFLICT (role_name) DO NOTHING;

-- Администратор по умолчанию (пароль: Admin123!)
INSERT INTO users (role_id, email, password_hash)
SELECT r.role_id, 'admin@company.com',
       '3eb3fe66b31e3b4d10fa70b5cad49c7112294af6ae4e476a1c405155d45aa121'
FROM roles r WHERE r.role_name = 'admin'
ON CONFLICT (email) DO NOTHING;

-- Тестовые товары
INSERT INTO products (product_name, price, reference_info, is_delivery_available) VALUES
('Ноутбук Pro 15', 85000.00, 'Процессор i7, 16GB RAM, 512GB SSD', 1),
('Монитор 27 4K', 32000.00, 'IPS, 144Hz, HDR400', 1),
('Клавиатура Механическая', 7500.00, 'Cherry MX Brown, RGB подсветка', 1),
('Мышь Беспроводная', 4500.00, 'Оптический сенсор 16000 DPI', 1),
('Принтер лазерный', 24000.00, 'МФУ, двусторонняя печать, Wi-Fi', 1)
ON CONFLICT DO NOTHING;

-- Способы доставки
INSERT INTO delivery_methods (method_name, speed) VALUES
('Курьерская доставка (экспресс)', '1-2 дня'),
('Транспортная компания (стандарт)', '3-5 дней'),
('Самовывоз со склада', 'В день заказа')
ON CONFLICT (method_name) DO NOTHING;

-- Тарифы доставки
INSERT INTO delivery_tariffs (product_id, delivery_method_id, base_delivery_price)
SELECT p.product_id, m.delivery_method_id, 1200.00
FROM products p, delivery_methods m
WHERE m.method_name LIKE 'Курьерская%'
ON CONFLICT (product_id, delivery_method_id) DO NOTHING;

INSERT INTO delivery_tariffs (product_id, delivery_method_id, base_delivery_price)
SELECT p.product_id, m.delivery_method_id, 600.00
FROM products p, delivery_methods m
WHERE m.method_name LIKE 'Транспортная%'
ON CONFLICT (product_id, delivery_method_id) DO NOTHING;

INSERT INTO delivery_tariffs (product_id, delivery_method_id, base_delivery_price)
SELECT p.product_id, m.delivery_method_id, 0.00
FROM products p, delivery_methods m
WHERE m.method_name LIKE 'Самовывоз%'
ON CONFLICT (product_id, delivery_method_id) DO NOTHING;
