--
-- PostgreSQL database dump
--

\restrict ySv8yIYtMg9wOyDQhzWBSYtG4j1XAbc7w5be0t1YgKJLVhMzadAijW5z3KH0Xsh

-- Dumped from database version 17.9 (Debian 17.9-1.pgdg13+1)
-- Dumped by pg_dump version 17.9 (Debian 17.9-1.pgdg13+1)

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET transaction_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

--
-- Name: tv; Type: SCHEMA; Schema: -; Owner: postgres
--

CREATE SCHEMA tv;


ALTER SCHEMA tv OWNER TO postgres;

--
-- Name: log_subscription_change(); Type: FUNCTION; Schema: tv; Owner: postgres
--

CREATE FUNCTION tv.log_subscription_change() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
    IF OLD.subscription_status IS DISTINCT FROM NEW.subscription_status THEN
        INSERT INTO tv.subscription_history (user_id, old_status, new_status)
        VALUES (NEW.user_id, OLD.subscription_status, NEW.subscription_status);
    END IF;
    RETURN NEW;
END;
$$;


ALTER FUNCTION tv.log_subscription_change() OWNER TO postgres;

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: broadcasts; Type: TABLE; Schema: tv; Owner: postgres
--

CREATE TABLE tv.broadcasts (
    broadcast_id integer NOT NULL,
    title character varying(150) NOT NULL,
    duration_minutes integer,
    air_date_time timestamp without time zone,
    rating numeric(3,1),
    channel_id integer NOT NULL,
    genre_id integer
);


ALTER TABLE tv.broadcasts OWNER TO postgres;

--
-- Name: broadcasts_broadcast_id_seq; Type: SEQUENCE; Schema: tv; Owner: postgres
--

CREATE SEQUENCE tv.broadcasts_broadcast_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE tv.broadcasts_broadcast_id_seq OWNER TO postgres;

--
-- Name: broadcasts_broadcast_id_seq; Type: SEQUENCE OWNED BY; Schema: tv; Owner: postgres
--

ALTER SEQUENCE tv.broadcasts_broadcast_id_seq OWNED BY tv.broadcasts.broadcast_id;


--
-- Name: channels; Type: TABLE; Schema: tv; Owner: postgres
--

CREATE TABLE tv.channels (
    channel_id integer NOT NULL,
    full_name character varying(100) NOT NULL,
    short_name character varying(20),
    country character varying(50),
    city character varying(50),
    owner character varying(100),
    contact_phone character varying(20),
    contact_email character varying(100)
);


ALTER TABLE tv.channels OWNER TO postgres;

--
-- Name: users; Type: TABLE; Schema: tv; Owner: postgres
--

CREATE TABLE tv.users (
    user_id integer NOT NULL,
    username character varying(50) NOT NULL,
    password_hash text NOT NULL,
    role_id integer NOT NULL,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    last_name character varying(50),
    first_name character varying(50),
    birth_date date,
    passport_data text,
    subscription_status character varying(30) DEFAULT 'active'::character varying,
    phone character varying(20),
    email character varying(100)
);


ALTER TABLE tv.users OWNER TO postgres;

--
-- Name: view_history; Type: TABLE; Schema: tv; Owner: postgres
--

CREATE TABLE tv.view_history (
    view_id integer NOT NULL,
    user_id integer NOT NULL,
    broadcast_id integer NOT NULL,
    view_date timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    watch_duration_minutes integer,
    device character varying(50)
);


ALTER TABLE tv.view_history OWNER TO postgres;

--
-- Name: channel_broadcasts_viewers; Type: VIEW; Schema: tv; Owner: postgres
--

CREATE VIEW tv.channel_broadcasts_viewers AS
 SELECT c.channel_id,
    c.full_name AS channel_name,
    b.broadcast_id,
    b.title AS broadcast_title,
    b.air_date_time,
    u.user_id,
    u.last_name,
    u.first_name,
    vh.view_date,
    vh.watch_duration_minutes
   FROM (((tv.channels c
     JOIN tv.broadcasts b ON ((c.channel_id = b.channel_id)))
     LEFT JOIN tv.view_history vh ON ((b.broadcast_id = vh.broadcast_id)))
     LEFT JOIN tv.users u ON ((vh.user_id = u.user_id)));


ALTER VIEW tv.channel_broadcasts_viewers OWNER TO postgres;

--
-- Name: channel_employees; Type: TABLE; Schema: tv; Owner: postgres
--

CREATE TABLE tv.channel_employees (
    user_id integer NOT NULL,
    channel_id integer NOT NULL
);


ALTER TABLE tv.channel_employees OWNER TO postgres;

--
-- Name: channels_channel_id_seq; Type: SEQUENCE; Schema: tv; Owner: postgres
--

CREATE SEQUENCE tv.channels_channel_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE tv.channels_channel_id_seq OWNER TO postgres;

--
-- Name: channels_channel_id_seq; Type: SEQUENCE OWNED BY; Schema: tv; Owner: postgres
--

ALTER SEQUENCE tv.channels_channel_id_seq OWNED BY tv.channels.channel_id;


--
-- Name: genres; Type: TABLE; Schema: tv; Owner: postgres
--

CREATE TABLE tv.genres (
    genre_id integer NOT NULL,
    genre_name character varying(50) NOT NULL
);


ALTER TABLE tv.genres OWNER TO postgres;

--
-- Name: genres_genre_id_seq; Type: SEQUENCE; Schema: tv; Owner: postgres
--

CREATE SEQUENCE tv.genres_genre_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE tv.genres_genre_id_seq OWNER TO postgres;

--
-- Name: genres_genre_id_seq; Type: SEQUENCE OWNED BY; Schema: tv; Owner: postgres
--

ALTER SEQUENCE tv.genres_genre_id_seq OWNED BY tv.genres.genre_id;


--
-- Name: roles; Type: TABLE; Schema: tv; Owner: postgres
--

CREATE TABLE tv.roles (
    role_id integer NOT NULL,
    role_name character varying(30) NOT NULL
);


ALTER TABLE tv.roles OWNER TO postgres;

--
-- Name: roles_role_id_seq; Type: SEQUENCE; Schema: tv; Owner: postgres
--

CREATE SEQUENCE tv.roles_role_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE tv.roles_role_id_seq OWNER TO postgres;

--
-- Name: roles_role_id_seq; Type: SEQUENCE OWNED BY; Schema: tv; Owner: postgres
--

ALTER SEQUENCE tv.roles_role_id_seq OWNED BY tv.roles.role_id;


--
-- Name: schedule_view; Type: VIEW; Schema: tv; Owner: postgres
--

CREATE VIEW tv.schedule_view AS
 SELECT b.broadcast_id,
    b.title,
    g.genre_name AS genre,
    b.duration_minutes,
    b.air_date_time,
    b.rating,
    c.full_name AS channel_name,
    c.short_name,
    c.city,
    b.channel_id
   FROM ((tv.broadcasts b
     JOIN tv.channels c ON ((b.channel_id = c.channel_id)))
     LEFT JOIN tv.genres g ON ((b.genre_id = g.genre_id)))
  WHERE (b.air_date_time >= CURRENT_DATE)
  ORDER BY b.air_date_time;


ALTER VIEW tv.schedule_view OWNER TO postgres;

--
-- Name: subscription_history; Type: TABLE; Schema: tv; Owner: postgres
--

CREATE TABLE tv.subscription_history (
    history_id integer NOT NULL,
    user_id integer NOT NULL,
    change_date timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    old_status character varying(30),
    new_status character varying(30),
    comment text
);


ALTER TABLE tv.subscription_history OWNER TO postgres;

--
-- Name: subscription_history_history_id_seq; Type: SEQUENCE; Schema: tv; Owner: postgres
--

CREATE SEQUENCE tv.subscription_history_history_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE tv.subscription_history_history_id_seq OWNER TO postgres;

--
-- Name: subscription_history_history_id_seq; Type: SEQUENCE OWNED BY; Schema: tv; Owner: postgres
--

ALTER SEQUENCE tv.subscription_history_history_id_seq OWNED BY tv.subscription_history.history_id;


--
-- Name: users_user_id_seq; Type: SEQUENCE; Schema: tv; Owner: postgres
--

CREATE SEQUENCE tv.users_user_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE tv.users_user_id_seq OWNER TO postgres;

--
-- Name: users_user_id_seq; Type: SEQUENCE OWNED BY; Schema: tv; Owner: postgres
--

ALTER SEQUENCE tv.users_user_id_seq OWNED BY tv.users.user_id;


--
-- Name: view_history_view_id_seq; Type: SEQUENCE; Schema: tv; Owner: postgres
--

CREATE SEQUENCE tv.view_history_view_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE tv.view_history_view_id_seq OWNER TO postgres;

--
-- Name: view_history_view_id_seq; Type: SEQUENCE OWNED BY; Schema: tv; Owner: postgres
--

ALTER SEQUENCE tv.view_history_view_id_seq OWNED BY tv.view_history.view_id;


--
-- Name: broadcasts broadcast_id; Type: DEFAULT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.broadcasts ALTER COLUMN broadcast_id SET DEFAULT nextval('tv.broadcasts_broadcast_id_seq'::regclass);


--
-- Name: channels channel_id; Type: DEFAULT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.channels ALTER COLUMN channel_id SET DEFAULT nextval('tv.channels_channel_id_seq'::regclass);


--
-- Name: genres genre_id; Type: DEFAULT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.genres ALTER COLUMN genre_id SET DEFAULT nextval('tv.genres_genre_id_seq'::regclass);


--
-- Name: roles role_id; Type: DEFAULT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.roles ALTER COLUMN role_id SET DEFAULT nextval('tv.roles_role_id_seq'::regclass);


--
-- Name: subscription_history history_id; Type: DEFAULT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.subscription_history ALTER COLUMN history_id SET DEFAULT nextval('tv.subscription_history_history_id_seq'::regclass);


--
-- Name: users user_id; Type: DEFAULT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.users ALTER COLUMN user_id SET DEFAULT nextval('tv.users_user_id_seq'::regclass);


--
-- Name: view_history view_id; Type: DEFAULT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.view_history ALTER COLUMN view_id SET DEFAULT nextval('tv.view_history_view_id_seq'::regclass);


--
-- Data for Name: broadcasts; Type: TABLE DATA; Schema: tv; Owner: postgres
--

COPY tv.broadcasts (broadcast_id, title, duration_minutes, air_date_time, rating, channel_id, genre_id) FROM stdin;
1	Время	60	2025-05-22 21:00:00	8.5	1	1
2	Пусть говорят	45	2025-05-21 19:00:00	6.2	1	2
3	Вести	30	2025-05-22 20:00:00	7.8	2	1
4	Новости дня	30	2026-05-22 22:17:22.468317	7.5	1	1
5	Спорт-обзор	45	2026-05-21 22:17:22.468317	6.8	1	4
7	Утреннее шоу	90	2026-05-25 08:29:43.255186	7.2	1	6
8	Тестовая передача	30	2026-05-23 00:00:27.689697	7.5	1	1
6	Вечернее шоу	60	2026-05-24 22:17:22.468	8.2	2	3
11	Даша путешественница	67	2026-05-30 14:29:59.974	0.0	5	6
\.


--
-- Data for Name: channel_employees; Type: TABLE DATA; Schema: tv; Owner: postgres
--

COPY tv.channel_employees (user_id, channel_id) FROM stdin;
2	1
2	2
2	5
6	5
\.


--
-- Data for Name: channels; Type: TABLE DATA; Schema: tv; Owner: postgres
--

COPY tv.channels (channel_id, full_name, short_name, country, city, owner, contact_phone, contact_email) FROM stdin;
2	Россия 1	R1	Россия	Москва	ВГТРК	+7-495-234-56-78	russia1@vgtrk.ru
5	НТВ	NTV	Россия	Москва	Газпром-Медиа	+7-495-345-67-89	ntv@ntv.ru
6	ТНT	TNT	Россия	Москва	Газпром-Медиа	+7-495-456-78-90	tnt@tnt.ru
1	Первый канал	1TV	Россия	Москва	АО "Первый канал"	+7-495-123-45-67	info@1tv.ru
\.


--
-- Data for Name: genres; Type: TABLE DATA; Schema: tv; Owner: postgres
--

COPY tv.genres (genre_id, genre_name) FROM stdin;
1	Новости
2	Ток-шоу
3	Сериал
4	Спорт
5	Документальный
6	Развлекательное
7	Кино
8	Драма
\.


--
-- Data for Name: roles; Type: TABLE DATA; Schema: tv; Owner: postgres
--

COPY tv.roles (role_id, role_name) FROM stdin;
1	admin
2	channel_employee
3	viewer
\.


--
-- Data for Name: subscription_history; Type: TABLE DATA; Schema: tv; Owner: postgres
--

COPY tv.subscription_history (history_id, user_id, change_date, old_status, new_status, comment) FROM stdin;
1	3	2026-04-28 23:29:31.889657	активный	VIP-подписка	Повышение статуса за активность
2	1	2026-05-13 23:29:31.889657	VIP-подписка	активный	Отказ от VIP
3	2	2026-05-23 23:29:31.889657	активный	заблокирован	Нарушение правил
4	3	2026-05-28 23:29:31.889657	заблокирован	активный	Разблокировка после апелляции
5	6	2026-05-29 10:22:59.914141	active	VIP	\N
6	6	2026-05-29 10:22:59.969329	active	VIP	Покупка подписки
7	6	2026-05-29 10:24:22.983912	VIP	inactive	\N
8	6	2026-05-29 10:24:22.988009	VIP	inactive	Отмена подписки
9	6	2026-05-29 10:24:41.970769	inactive	inactive	Отмена подписки
10	6	2026-05-29 10:24:59.551072	inactive	VIP	\N
11	6	2026-05-29 10:24:59.555306	inactive	VIP	Покупка подписки
12	6	2026-05-29 11:13:35.849136	VIP	inactive	\N
13	6	2026-05-29 11:13:35.855909	VIP	inactive	Отмена подписки
14	6	2026-05-29 11:13:41.153314	inactive	active	\N
15	6	2026-05-29 11:13:41.157358	inactive	active	Покупка подписки
16	6	2026-05-29 11:13:46.257642	active	VIP	\N
17	6	2026-05-29 11:13:46.319226	active	VIP	Покупка подписки
18	6	2026-05-29 11:13:56.117751	VIP	inactive	\N
20	3	2026-05-29 11:42:24.6042	active	blocked	\N
21	3	2026-05-29 11:42:32.029756	blocked	active	\N
22	3	2026-05-29 11:42:36.172539	active	blocked	\N
23	6	2026-05-29 11:46:08.661589	inactive	VIP	\N
24	6	2026-05-29 11:46:08.665781	inactive	VIP	Покупка подписки
25	3	2026-05-29 11:53:44.528102	blocked	active	\N
26	4	2026-05-29 11:53:56.85638	active	blocked	\N
27	7	2026-05-29 12:16:04.532034	active	VIP	Покупка подписки
28	7	2026-05-29 12:16:08.150147	VIP	inactive	Отмена подписки
30	3	2026-05-29 12:29:22.092318	active	VIP	Покупка подписки
31	7	2026-05-29 14:18:27.903	inactive	VIP	промокод
32	7	2026-05-29 14:42:29.135992	VIP	VIP	Покупка подписки
33	7	2026-05-29 19:36:59.001933	VIP	active	Покупка подписки
\.


--
-- Data for Name: users; Type: TABLE DATA; Schema: tv; Owner: postgres
--

COPY tv.users (user_id, username, password_hash, role_id, created_at, last_name, first_name, birth_date, passport_data, subscription_status, phone, email) FROM stdin;
1	admin	240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9	1	2026-05-22 21:09:15.755808	Администраторов	Системный	1980-01-01	0000 000000	active	+7-999-000-00-00	admin@tv.ru
7	1234	03ac674216f3e15c761ee1a5e255f067953623c8b388b4459e13f978d7c846f4	3	2026-05-29 12:15:45.685718	Стариков	Михаил	2000-01-01	1219230	active	89997123512	stawrv@gmail.com
6	ger	03ac674216f3e15c761ee1a5e255f067953623c8b388b4459e13f978d7c846f4	2	2026-05-29 09:11:10.179806	Андреев	Герман	1990-01-01	234234	active	89997125135	ger@gmail.com
2	channel1	a665a45920422f9d417e4867efdc4fb8a04a1f3fff1fa07e998e86f7f7a27ae3	2	2026-05-22 21:09:15.755808	Каналов	Сотрудник	1990-05-15	1111 111111	active	+7-999-111-11-11	channel@tv.ru
4	supervisor1	a665a45920422f9d417e4867efdc4fb8a04a1f3fff1fa07e998e86f7f7a27ae3	3	2026-05-22 21:09:15.755808	Надзорова	Инспектор	1975-08-20	2222 222222	blocked	+7-999-222-22-22	supervisor@tv.ru
3	viewer1	a665a45920422f9d417e4867efdc4fb8a04a1f3fff1fa07e998e86f7f7a27ae3	3	2026-05-22 21:09:15.755808	Иванов	Иван	1985-04-10	4510 123456	active	+7-912-345-67-89	ivan@example.com
\.


--
-- Data for Name: view_history; Type: TABLE DATA; Schema: tv; Owner: postgres
--

COPY tv.view_history (view_id, user_id, broadcast_id, view_date, watch_duration_minutes, device) FROM stdin;
1	1	1	2026-05-28 22:25:18.727483	45	Smart TV
2	2	1	2026-05-27 22:25:18.727483	30	Mobile
3	3	2	2026-05-26 22:25:18.727483	60	Laptop
4	4	3	2026-05-25 22:25:18.727483	15	Tablet
5	1	2	2026-05-24 22:25:18.727483	90	PC
6	3	1	2026-05-28 21:25:18.727483	25	Smart TV
7	7	7	2026-05-29 14:18:10.015	90	tecno
8	7	11	2026-05-30 07:06:44.143	90	8
\.


--
-- Name: broadcasts_broadcast_id_seq; Type: SEQUENCE SET; Schema: tv; Owner: postgres
--

SELECT pg_catalog.setval('tv.broadcasts_broadcast_id_seq', 11, true);


--
-- Name: channels_channel_id_seq; Type: SEQUENCE SET; Schema: tv; Owner: postgres
--

SELECT pg_catalog.setval('tv.channels_channel_id_seq', 13, true);


--
-- Name: genres_genre_id_seq; Type: SEQUENCE SET; Schema: tv; Owner: postgres
--

SELECT pg_catalog.setval('tv.genres_genre_id_seq', 8, true);


--
-- Name: roles_role_id_seq; Type: SEQUENCE SET; Schema: tv; Owner: postgres
--

SELECT pg_catalog.setval('tv.roles_role_id_seq', 4, true);


--
-- Name: subscription_history_history_id_seq; Type: SEQUENCE SET; Schema: tv; Owner: postgres
--

SELECT pg_catalog.setval('tv.subscription_history_history_id_seq', 33, true);


--
-- Name: users_user_id_seq; Type: SEQUENCE SET; Schema: tv; Owner: postgres
--

SELECT pg_catalog.setval('tv.users_user_id_seq', 7, true);


--
-- Name: view_history_view_id_seq; Type: SEQUENCE SET; Schema: tv; Owner: postgres
--

SELECT pg_catalog.setval('tv.view_history_view_id_seq', 8, true);


--
-- Name: broadcasts broadcasts_pkey; Type: CONSTRAINT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.broadcasts
    ADD CONSTRAINT broadcasts_pkey PRIMARY KEY (broadcast_id);


--
-- Name: channel_employees channel_employees_pkey; Type: CONSTRAINT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.channel_employees
    ADD CONSTRAINT channel_employees_pkey PRIMARY KEY (user_id, channel_id);


--
-- Name: channels channels_pkey; Type: CONSTRAINT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.channels
    ADD CONSTRAINT channels_pkey PRIMARY KEY (channel_id);


--
-- Name: genres genres_genre_name_key; Type: CONSTRAINT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.genres
    ADD CONSTRAINT genres_genre_name_key UNIQUE (genre_name);


--
-- Name: genres genres_pkey; Type: CONSTRAINT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.genres
    ADD CONSTRAINT genres_pkey PRIMARY KEY (genre_id);


--
-- Name: roles roles_pkey; Type: CONSTRAINT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.roles
    ADD CONSTRAINT roles_pkey PRIMARY KEY (role_id);


--
-- Name: roles roles_role_name_key; Type: CONSTRAINT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.roles
    ADD CONSTRAINT roles_role_name_key UNIQUE (role_name);


--
-- Name: subscription_history subscription_history_pkey; Type: CONSTRAINT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.subscription_history
    ADD CONSTRAINT subscription_history_pkey PRIMARY KEY (history_id);


--
-- Name: users users_pkey; Type: CONSTRAINT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.users
    ADD CONSTRAINT users_pkey PRIMARY KEY (user_id);


--
-- Name: users users_username_key; Type: CONSTRAINT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.users
    ADD CONSTRAINT users_username_key UNIQUE (username);


--
-- Name: view_history view_history_pkey; Type: CONSTRAINT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.view_history
    ADD CONSTRAINT view_history_pkey PRIMARY KEY (view_id);


--
-- Name: idx_broadcasts_channel; Type: INDEX; Schema: tv; Owner: postgres
--

CREATE INDEX idx_broadcasts_channel ON tv.broadcasts USING btree (channel_id);


--
-- Name: idx_users_username; Type: INDEX; Schema: tv; Owner: postgres
--

CREATE INDEX idx_users_username ON tv.users USING btree (username);


--
-- Name: broadcasts broadcasts_channel_id_fkey; Type: FK CONSTRAINT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.broadcasts
    ADD CONSTRAINT broadcasts_channel_id_fkey FOREIGN KEY (channel_id) REFERENCES tv.channels(channel_id) ON DELETE CASCADE;


--
-- Name: broadcasts broadcasts_genre_id_fkey; Type: FK CONSTRAINT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.broadcasts
    ADD CONSTRAINT broadcasts_genre_id_fkey FOREIGN KEY (genre_id) REFERENCES tv.genres(genre_id);


--
-- Name: channel_employees channel_employees_channel_id_fkey; Type: FK CONSTRAINT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.channel_employees
    ADD CONSTRAINT channel_employees_channel_id_fkey FOREIGN KEY (channel_id) REFERENCES tv.channels(channel_id) ON DELETE CASCADE;


--
-- Name: channel_employees channel_employees_user_id_fkey; Type: FK CONSTRAINT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.channel_employees
    ADD CONSTRAINT channel_employees_user_id_fkey FOREIGN KEY (user_id) REFERENCES tv.users(user_id) ON DELETE CASCADE;


--
-- Name: subscription_history subscription_history_user_id_fkey; Type: FK CONSTRAINT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.subscription_history
    ADD CONSTRAINT subscription_history_user_id_fkey FOREIGN KEY (user_id) REFERENCES tv.users(user_id) ON DELETE CASCADE;


--
-- Name: users users_role_id_fkey; Type: FK CONSTRAINT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.users
    ADD CONSTRAINT users_role_id_fkey FOREIGN KEY (role_id) REFERENCES tv.roles(role_id) ON DELETE RESTRICT;


--
-- Name: view_history view_history_broadcast_id_fkey; Type: FK CONSTRAINT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.view_history
    ADD CONSTRAINT view_history_broadcast_id_fkey FOREIGN KEY (broadcast_id) REFERENCES tv.broadcasts(broadcast_id) ON DELETE CASCADE;


--
-- Name: view_history view_history_user_id_fkey; Type: FK CONSTRAINT; Schema: tv; Owner: postgres
--

ALTER TABLE ONLY tv.view_history
    ADD CONSTRAINT view_history_user_id_fkey FOREIGN KEY (user_id) REFERENCES tv.users(user_id) ON DELETE CASCADE;


--
-- Name: SCHEMA tv; Type: ACL; Schema: -; Owner: postgres
--

GRANT USAGE ON SCHEMA tv TO tv_app;


--
-- Name: TABLE broadcasts; Type: ACL; Schema: tv; Owner: postgres
--

GRANT ALL ON TABLE tv.broadcasts TO tv_app;


--
-- Name: SEQUENCE broadcasts_broadcast_id_seq; Type: ACL; Schema: tv; Owner: postgres
--

GRANT SELECT,USAGE ON SEQUENCE tv.broadcasts_broadcast_id_seq TO tv_app;


--
-- Name: TABLE channels; Type: ACL; Schema: tv; Owner: postgres
--

GRANT ALL ON TABLE tv.channels TO tv_app;


--
-- Name: TABLE users; Type: ACL; Schema: tv; Owner: postgres
--

GRANT ALL ON TABLE tv.users TO tv_app;


--
-- Name: TABLE view_history; Type: ACL; Schema: tv; Owner: postgres
--

GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE tv.view_history TO tv_app;


--
-- Name: TABLE channel_broadcasts_viewers; Type: ACL; Schema: tv; Owner: postgres
--

GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE tv.channel_broadcasts_viewers TO tv_app;


--
-- Name: SEQUENCE channels_channel_id_seq; Type: ACL; Schema: tv; Owner: postgres
--

GRANT SELECT,USAGE ON SEQUENCE tv.channels_channel_id_seq TO tv_app;


--
-- Name: TABLE genres; Type: ACL; Schema: tv; Owner: postgres
--

GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE tv.genres TO tv_app;


--
-- Name: SEQUENCE genres_genre_id_seq; Type: ACL; Schema: tv; Owner: postgres
--

GRANT SELECT,USAGE ON SEQUENCE tv.genres_genre_id_seq TO tv_app;


--
-- Name: TABLE roles; Type: ACL; Schema: tv; Owner: postgres
--

GRANT ALL ON TABLE tv.roles TO tv_app;


--
-- Name: SEQUENCE roles_role_id_seq; Type: ACL; Schema: tv; Owner: postgres
--

GRANT SELECT,USAGE ON SEQUENCE tv.roles_role_id_seq TO tv_app;


--
-- Name: TABLE schedule_view; Type: ACL; Schema: tv; Owner: postgres
--

GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE tv.schedule_view TO tv_app;


--
-- Name: TABLE subscription_history; Type: ACL; Schema: tv; Owner: postgres
--

GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE tv.subscription_history TO tv_app;


--
-- Name: SEQUENCE subscription_history_history_id_seq; Type: ACL; Schema: tv; Owner: postgres
--

GRANT USAGE ON SEQUENCE tv.subscription_history_history_id_seq TO tv_app;


--
-- Name: SEQUENCE users_user_id_seq; Type: ACL; Schema: tv; Owner: postgres
--

GRANT SELECT,USAGE ON SEQUENCE tv.users_user_id_seq TO tv_app;


--
-- Name: SEQUENCE view_history_view_id_seq; Type: ACL; Schema: tv; Owner: postgres
--

GRANT USAGE ON SEQUENCE tv.view_history_view_id_seq TO tv_app;


--
-- PostgreSQL database dump complete
--

\unrestrict ySv8yIYtMg9wOyDQhzWBSYtG4j1XAbc7w5be0t1YgKJLVhMzadAijW5z3KH0Xsh

