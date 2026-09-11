-- =============================================================
-- Информационная система для ведения заказов компанией
-- Физическая модель БД (PostgreSQL)
-- =============================================================

-- 1. Роли (Roles)
CREATE TABLE IF NOT EXISTS roles (
    role_id SERIAL PRIMARY KEY,
    role_name VARCHAR(100) NOT NULL,
    CONSTRAINT uq_roles_role_name UNIQUE (role_name)
);

-- 2. Пользователи (Users)
CREATE TABLE IF NOT EXISTS users (
    user_id SERIAL PRIMARY KEY,
    role_id INT NOT NULL REFERENCES roles(role_id) ON DELETE RESTRICT ON UPDATE CASCADE,
    email VARCHAR(255) NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    CONSTRAINT uq_users_email UNIQUE (email)
);

-- 3. Заказчики (Customers)
CREATE TABLE IF NOT EXISTS customers (
    customer_id SERIAL PRIMARY KEY,
    user_id INT NOT NULL REFERENCES users(user_id) ON DELETE CASCADE ON UPDATE CASCADE,
    organization_name VARCHAR(255) NOT NULL,
    contact_person VARCHAR(255) NOT NULL,
    address TEXT NOT NULL,
    phone VARCHAR(20) NOT NULL,
    CONSTRAINT uq_customers_user UNIQUE (user_id),
    CONSTRAINT uq_customers_phone UNIQUE (phone)
);

-- 4. Заказы (Orders)
CREATE TABLE IF NOT EXISTS orders (
    order_id SERIAL PRIMARY KEY,
    customer_id INT NOT NULL REFERENCES customers(customer_id) ON DELETE CASCADE ON UPDATE CASCADE,
    purchase_date DATE NOT NULL DEFAULT CURRENT_DATE
);

-- 5. Товары (Products)
CREATE TABLE IF NOT EXISTS products (
    product_id SERIAL PRIMARY KEY,
    product_name VARCHAR(255) NOT NULL,
    price NUMERIC(12, 2) NOT NULL CHECK (price >= 0),
    reference_info TEXT,
    is_delivery_available BOOLEAN NOT NULL DEFAULT TRUE
);

-- 6. Способы доставки (Delivery Methods)
CREATE TABLE IF NOT EXISTS delivery_methods (
    delivery_method_id SERIAL PRIMARY KEY,
    method_name VARCHAR(150) NOT NULL,
    speed VARCHAR(100) NOT NULL,
    CONSTRAINT uq_delivery_methods_name UNIQUE (method_name)
);

-- 7. Тарифы доставки (Delivery Tariffs)
CREATE TABLE IF NOT EXISTS delivery_tariffs (
    delivery_tariff_id SERIAL PRIMARY KEY,
    product_id INT NOT NULL REFERENCES products(product_id) ON DELETE CASCADE ON UPDATE CASCADE,
    delivery_method_id INT NOT NULL REFERENCES delivery_methods(delivery_method_id) ON DELETE CASCADE ON UPDATE CASCADE,
    base_delivery_price NUMERIC(12, 2) NOT NULL CHECK (base_delivery_price >= 0),
    CONSTRAINT uq_delivery_tariffs_product_method UNIQUE (product_id, delivery_method_id)
);

-- 8. Позиции заказа (Order Items)
CREATE TABLE IF NOT EXISTS order_items (
    order_item_id SERIAL PRIMARY KEY,
    order_id INT NOT NULL REFERENCES orders(order_id) ON DELETE CASCADE ON UPDATE CASCADE,
    product_id INT NOT NULL REFERENCES products(product_id) ON DELETE RESTRICT ON UPDATE CASCADE,
    delivery_method_id INT NOT NULL REFERENCES delivery_methods(delivery_method_id) ON DELETE RESTRICT ON UPDATE CASCADE,
    actual_delivery_cost NUMERIC(12, 2) NOT NULL CHECK (actual_delivery_cost >= 0),
    quantity INT NOT NULL CHECK (quantity > 0)
);

-- =============================================================
-- Начальные данные
-- =============================================================

-- Роли
INSERT INTO roles (role_name) VALUES ('admin') ON CONFLICT (role_name) DO NOTHING;
INSERT INTO roles (role_name) VALUES ('customer') ON CONFLICT (role_name) DO NOTHING;
INSERT INTO roles (role_name) VALUES ('user') ON CONFLICT (role_name) DO NOTHING;

-- Администратор по умолчанию (пароль: Admin123!)
-- SHA-256 хэш пароля "Admin123!"
INSERT INTO users (role_id, email, password_hash)
SELECT r.role_id, 'admin@company.com',
       '3eb3fe66b31e3b4d10fa70b5cad49c7112294af6ae4e476a1c405155d45aa121'
FROM roles r WHERE r.role_name = 'admin'
ON CONFLICT (email) DO NOTHING;

-- Начальные товары
INSERT INTO products (product_name, price, reference_info, is_delivery_available) VALUES
('Ноутбук Pro 15', 85000.00, 'Процессор i7, 16GB RAM, 512GB SSD', TRUE),
('Монитор 27 4K', 32000.00, 'IPS, 144Hz, HDR400', TRUE),
('Клавиатура Механическая', 7500.00, 'Cherry MX Brown, RGB подсветка', TRUE),
('Мышь Беспроводная', 4500.00, 'Оптический сенсор 16000 DPI', TRUE),
('Принтер лазерный', 24000.00, 'МФУ, двусторонняя печать, Wi-Fi', TRUE)
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
