-- ============================================================
--  data.sql  -  Dados inseridos manualmente (seed)
--  Usuario de teste + 2 conexoes (hardwares) + dispositivos +
--  leitura atual + historico sintetico para popular os graficos.
--
--  Senha do usuario de teste: "123456"
--  O hash abaixo e um bcrypt valido de "123456".
--  (Voce pode recriar pelo endpoint POST /auth/register.)
-- ============================================================

USE SixSeven_Projects;

-- Usuario de teste ------------------------------------------------
INSERT INTO usuario(nome, email, senha) VALUES
('Arthur Victor', 'arthur@sixseven.dev',
 '$2a$10$N9qo8uLOickgx2ZMRZoMyeIjZAgcfl7p92ldGxad68LJZdL17lhWy');

SET @uid = LAST_INSERT_ID();

-- Conexoes (cada uma = 1 hardware) -------------------------------
INSERT INTO conections(nome, can_Active, is_Active, id_usuario) VALUES
('Hardware Sala',    TRUE,  TRUE,  @uid),
('Hardware Oficina', TRUE,  FALSE, @uid);

-- Recupera os ids reais das conexoes recem-criadas (a 1a e a 2a):
SELECT id INTO @c1 FROM conections WHERE id_usuario = @uid ORDER BY id LIMIT 1;
SELECT id INTO @c2 FROM conections WHERE id_usuario = @uid ORDER BY id LIMIT 1 OFFSET 1;

-- Dispositivos ---------------------------------------------------
INSERT INTO dispositivos(nome, tipo, id_usuario) VALUES
('Potenciometro Recebida', 'sensor', @uid),
('Potenciometro Base',     'sensor', @uid),
('LED Duplo',              'atuador', @uid),
('Buzzer',                 'atuador', @uid);

-- Relacao N:N (associa os 4 dispositivos a conexao 1) ------------
INSERT INTO dispositivos_conections(id_conections, id_dispositivos)
SELECT @c1, id FROM dispositivos WHERE id_usuario = @uid;

-- Leitura atual (tabela info) ------------------------------------
INSERT INTO info(id, E_b, E_r, timer, M_er, V_mer, M_vmer) VALUES
(@c1, 3000, 2850, 10000, 2820, 90, 75);

INSERT INTO info(id, E_b, E_r, timer, M_er, V_mer, M_vmer) VALUES
(@c2, 2500, 0, 10000, 0, 0, 0);

-- Historico sintetico (para os graficos) -------------------------
-- 12 pontos oscilando ao redor da media, hardware 1.
INSERT INTO info_historico(id_conection, E_b, E_r, timer, M_er, V_mer, M_vmer, criado_em) VALUES
(@c1, 3000, 2500, 10000, 2500,  20,  20, NOW() - INTERVAL 120 SECOND),
(@c1, 3000, 2650, 10000, 2575,  75,  47, NOW() - INTERVAL 110 SECOND),
(@c1, 3000, 2700, 10000, 2617,  83,  59, NOW() - INTERVAL 100 SECOND),
(@c1, 3000, 2900, 10000, 2687, 213, 100, NOW() - INTERVAL  90 SECOND),
(@c1, 3000, 2750, 10000, 2700,  50,  90, NOW() - INTERVAL  80 SECOND),
(@c1, 3000, 2820, 10000, 2720, 100,  92, NOW() - INTERVAL  70 SECOND),
(@c1, 3000, 2880, 10000, 2743, 137,  99, NOW() - INTERVAL  60 SECOND),
(@c1, 3000, 2810, 10000, 2751,  59,  90, NOW() - INTERVAL  50 SECOND),
(@c1, 3000, 2790, 10000, 2755,  35,  80, NOW() - INTERVAL  40 SECOND),
(@c1, 3000, 2850, 10000, 2768,  82,  78, NOW() - INTERVAL  30 SECOND),
(@c1, 3000, 2830, 10000, 2775,  55,  70, NOW() - INTERVAL  20 SECOND),
(@c1, 3000, 2850, 10000, 2820,  90,  75, NOW() - INTERVAL  10 SECOND);
