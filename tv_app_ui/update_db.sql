-- Таблица связи сотрудников каналов с пользователями
CREATE TABLE IF NOT EXISTS tv.channel_employees (
    user_id INT PRIMARY KEY REFERENCES tv.users(user_id) ON DELETE CASCADE,
    channel_id INT NOT NULL REFERENCES tv.channels(channel_id) ON DELETE CASCADE
);

-- Таблица истории подписок (лицевой счёт)
CREATE TABLE IF NOT EXISTS tv.subscription_history (
    history_id SERIAL PRIMARY KEY,
    viewer_id INT REFERENCES tv.viewers(viewer_id) ON DELETE CASCADE,
    change_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    old_status VARCHAR(30),
    new_status VARCHAR(30),
    comment TEXT
);

-- Добавим триггер для автоматической записи в subscription_history при изменении статуса
CREATE OR REPLACE FUNCTION tv.log_subscription_change()
RETURNS TRIGGER AS $$
BEGIN
    IF OLD.subscription_status IS DISTINCT FROM NEW.subscription_status THEN
        INSERT INTO tv.subscription_history (viewer_id, old_status, new_status)
        VALUES (NEW.viewer_id, OLD.subscription_status, NEW.subscription_status);
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

DROP TRIGGER IF EXISTS trg_subscription_change ON tv.viewers;
CREATE TRIGGER trg_subscription_change
AFTER UPDATE OF subscription_status ON tv.viewers
FOR EACH ROW EXECUTE FUNCTION tv.log_subscription_change();

-- Добавим представление для отчёта о передачах канала со зрителями
CREATE OR REPLACE VIEW tv.channel_broadcasts_viewers AS
SELECT 
    c.channel_id,
    c.full_name AS channel_name,
    b.broadcast_id,
    b.title AS broadcast_title,
    b.air_date_time,
    v.viewer_id,
    v.last_name,
    v.first_name,
    vh.view_date,
    vh.watch_duration_minutes
FROM tv.channels c
JOIN tv.broadcasts b ON c.channel_id = b.channel_id
LEFT JOIN tv.view_history vh ON b.broadcast_id = vh.broadcast_id
LEFT JOIN tv.viewers v ON vh.viewer_id = v.viewer_id
ORDER BY c.channel_id, b.air_date_time, vh.view_date;
