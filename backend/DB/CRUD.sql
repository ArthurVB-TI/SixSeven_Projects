-- ============================================================
--  CRUD.sql  -  Procedures de CRUD por tabela
--  Usam as funcoes de functions.sql para validacao.
--  A senha chega ja "hasheada" pelo backend (bcrypt/argon2).
-- ============================================================

USE SixSeven_Projects;

DELIMITER %

-- ============================ USUARIO =======================

DROP PROCEDURE IF EXISTS usuario_Create %
CREATE PROCEDURE usuario_Create(IN PARAM_NOME VARCHAR(100), IN PARAM_EMAIL VARCHAR(100), IN PARAM_SENHA VARCHAR(255))
BEGIN
    IF fn_email_valido(PARAM_EMAIL)
    AND fn_senha_valida(PARAM_SENHA)
    AND NOT EXISTS (SELECT 1 FROM usuario WHERE email = PARAM_EMAIL) THEN
        INSERT INTO usuario(nome, email, senha) VALUES (PARAM_NOME, PARAM_EMAIL, PARAM_SENHA);
        SELECT LAST_INSERT_ID() AS id;
    ELSE
        SELECT 0 AS id;
    END IF;
END%

DROP PROCEDURE IF EXISTS usuario_Update %
CREATE PROCEDURE usuario_Update(IN PARAM_ID INT, IN PARAM_NOME VARCHAR(100), IN PARAM_EMAIL VARCHAR(100), IN PARAM_SENHA VARCHAR(255))
BEGIN
    IF fn_email_valido(PARAM_EMAIL)
    AND fn_senha_valida(PARAM_SENHA)
    AND EXISTS (SELECT 1 FROM usuario WHERE id = PARAM_ID) THEN
        UPDATE usuario SET nome = PARAM_NOME, email = PARAM_EMAIL, senha = PARAM_SENHA WHERE id = PARAM_ID;
    END IF;
END%

DROP PROCEDURE IF EXISTS usuario_Delete %
CREATE PROCEDURE usuario_Delete(IN PARAM_ID INT)
BEGIN
    IF EXISTS (SELECT 1 FROM usuario WHERE id = PARAM_ID) THEN
        DELETE FROM usuario WHERE id = PARAM_ID;
    END IF;
END%

-- Login: retorna id/nome/email/senha(hash) por e-mail.
-- A verificacao do hash acontece no backend (Security).
DROP PROCEDURE IF EXISTS usuario_FindByEmail %
CREATE PROCEDURE usuario_FindByEmail(IN PARAM_EMAIL VARCHAR(100))
BEGIN
    SELECT id, nome, email, senha FROM usuario WHERE email = PARAM_EMAIL LIMIT 1;
END%

-- ============================ CONECTIONS ====================

DROP PROCEDURE IF EXISTS conections_Create %
CREATE PROCEDURE conections_Create(IN PARAM_NOME VARCHAR(100), IN PARAM_CAN_ACTIVE BOOLEAN, IN PARAM_IS_ACTIVE BOOLEAN, IN PARAM_ID_USUARIO INT)
BEGIN
    IF EXISTS (SELECT 1 FROM usuario WHERE id = PARAM_ID_USUARIO)
    AND NOT EXISTS (SELECT 1 FROM conections WHERE nome = PARAM_NOME AND id_usuario = PARAM_ID_USUARIO) THEN
        INSERT INTO conections(nome, can_Active, is_Active, id_usuario) VALUES (PARAM_NOME, PARAM_CAN_ACTIVE, PARAM_IS_ACTIVE, PARAM_ID_USUARIO);
        SELECT LAST_INSERT_ID() AS id;
    ELSE
        SELECT 0 AS id;
    END IF;
END%

DROP PROCEDURE IF EXISTS conections_Update %
CREATE PROCEDURE conections_Update(IN PARAM_ID INT, IN PARAM_NOME VARCHAR(100), IN PARAM_CAN_ACTIVE BOOLEAN, IN PARAM_IS_ACTIVE BOOLEAN, IN PARAM_ID_USUARIO INT)
BEGIN
    IF EXISTS (SELECT 1 FROM conections WHERE id = PARAM_ID)
    AND EXISTS (SELECT 1 FROM usuario WHERE id = PARAM_ID_USUARIO) THEN
        UPDATE conections SET nome = PARAM_NOME, can_Active = PARAM_CAN_ACTIVE, is_Active = PARAM_IS_ACTIVE, id_usuario = PARAM_ID_USUARIO WHERE id = PARAM_ID;
    END IF;
END%

DROP PROCEDURE IF EXISTS conections_Delete %
CREATE PROCEDURE conections_Delete(IN PARAM_ID INT)
BEGIN
    IF EXISTS (SELECT 1 FROM conections WHERE id = PARAM_ID) THEN
        DELETE FROM conections WHERE id = PARAM_ID;
    END IF;
END%

-- Conexoes de um usuario especifico (pagina inicial do site).
DROP PROCEDURE IF EXISTS conections_IndexByUsuario %
CREATE PROCEDURE conections_IndexByUsuario(IN PARAM_ID_USUARIO INT)
BEGIN
    SELECT id, nome, can_Active, is_Active, id_usuario
    FROM conections WHERE id_usuario = PARAM_ID_USUARIO;
END%

-- ============================ DISPOSITIVOS ==================

DROP PROCEDURE IF EXISTS dispositivos_Create %
CREATE PROCEDURE dispositivos_Create(IN PARAM_NOME VARCHAR(100), IN PARAM_TIPO VARCHAR(50), IN PARAM_ID_USUARIO INT)
BEGIN
    IF EXISTS (SELECT 1 FROM usuario WHERE id = PARAM_ID_USUARIO)
    AND NOT EXISTS (SELECT 1 FROM dispositivos WHERE nome = PARAM_NOME AND id_usuario = PARAM_ID_USUARIO) THEN
        INSERT INTO dispositivos(nome, tipo, id_usuario) VALUES (PARAM_NOME, PARAM_TIPO, PARAM_ID_USUARIO);
        SELECT LAST_INSERT_ID() AS id;
    ELSE
        SELECT 0 AS id;
    END IF;
END%

DROP PROCEDURE IF EXISTS dispositivos_Update %
CREATE PROCEDURE dispositivos_Update(IN PARAM_ID INT, IN PARAM_NOME VARCHAR(100), IN PARAM_TIPO VARCHAR(50), IN PARAM_ID_USUARIO INT)
BEGIN
    IF EXISTS (SELECT 1 FROM dispositivos WHERE id = PARAM_ID)
    AND EXISTS (SELECT 1 FROM usuario WHERE id = PARAM_ID_USUARIO) THEN
        UPDATE dispositivos SET nome = PARAM_NOME, tipo = PARAM_TIPO, id_usuario = PARAM_ID_USUARIO WHERE id = PARAM_ID;
    END IF;
END%

DROP PROCEDURE IF EXISTS dispositivos_Delete %
CREATE PROCEDURE dispositivos_Delete(IN PARAM_ID INT)
BEGIN
    IF EXISTS (SELECT 1 FROM dispositivos WHERE id = PARAM_ID) THEN
        DELETE FROM dispositivos WHERE id = PARAM_ID;
    END IF;
END%

-- ==================== DISPOSITIVOS_CONECTIONS ===============

DROP PROCEDURE IF EXISTS dispositivos_conections_Create %
CREATE PROCEDURE dispositivos_conections_Create(IN PARAM_ID_CONECTIONS INT, IN PARAM_ID_DISPOSITIVOS INT)
BEGIN
    IF EXISTS (SELECT 1 FROM conections WHERE id = PARAM_ID_CONECTIONS)
    AND EXISTS (SELECT 1 FROM dispositivos WHERE id = PARAM_ID_DISPOSITIVOS)
    AND NOT EXISTS (SELECT 1 FROM dispositivos_conections WHERE id_conections = PARAM_ID_CONECTIONS AND id_dispositivos = PARAM_ID_DISPOSITIVOS) THEN
        INSERT INTO dispositivos_conections(id_conections, id_dispositivos) VALUES (PARAM_ID_CONECTIONS, PARAM_ID_DISPOSITIVOS);
    END IF;
END%

DROP PROCEDURE IF EXISTS dispositivos_conections_Delete %
CREATE PROCEDURE dispositivos_conections_Delete(IN PARAM_ID_CONECTIONS INT, IN PARAM_ID_DISPOSITIVOS INT)
BEGIN
    IF EXISTS (SELECT 1 FROM dispositivos_conections WHERE id_conections = PARAM_ID_CONECTIONS AND id_dispositivos = PARAM_ID_DISPOSITIVOS) THEN
        DELETE FROM dispositivos_conections WHERE id_conections = PARAM_ID_CONECTIONS AND id_dispositivos = PARAM_ID_DISPOSITIVOS;
    END IF;
END%

-- ============================ INFO ==========================
-- "Upsert": o hardware envia leitura; se ja existe atualiza,
-- senao cria. Alem disso grava no historico (via logic layer).

DROP PROCEDURE IF EXISTS info_Create %
CREATE PROCEDURE info_Create(IN PARAM_ID INT, IN PARAM_E_B INT, IN PARAM_E_R INT, IN PARAM_TIMER INT, IN PARAM_M_ER INT, IN PARAM_V_MER INT, IN PARAM_M_VMER INT)
BEGIN
    IF (PARAM_E_B >= 0) AND (PARAM_E_R >= 0) AND (PARAM_TIMER > 0)
    AND EXISTS (SELECT 1 FROM conections WHERE id = PARAM_ID)
    AND NOT EXISTS (SELECT 1 FROM info WHERE id = PARAM_ID) THEN
        INSERT INTO info(id, E_b, E_r, timer, M_er, V_mer, M_vmer)
        VALUES (PARAM_ID, PARAM_E_B, PARAM_E_R, PARAM_TIMER, PARAM_M_ER, PARAM_V_MER, PARAM_M_VMER);
    END IF;
END%

DROP PROCEDURE IF EXISTS info_Update %
CREATE PROCEDURE info_Update(IN PARAM_ID INT, IN PARAM_E_B INT, IN PARAM_E_R INT, IN PARAM_TIMER INT, IN PARAM_M_ER INT, IN PARAM_V_MER INT, IN PARAM_M_VMER INT)
BEGIN
    IF (PARAM_E_B >= 0) AND (PARAM_E_R >= 0) AND (PARAM_TIMER > 0)
    AND EXISTS (SELECT 1 FROM info WHERE id = PARAM_ID) THEN
        UPDATE info SET E_b = PARAM_E_B, E_r = PARAM_E_R, timer = PARAM_TIMER, M_er = PARAM_M_ER, V_mer = PARAM_V_MER, M_vmer = PARAM_M_VMER WHERE id = PARAM_ID;
    END IF;
END%

DROP PROCEDURE IF EXISTS info_Delete %
CREATE PROCEDURE info_Delete(IN PARAM_ID INT)
BEGIN
    IF EXISTS (SELECT 1 FROM info WHERE id = PARAM_ID) THEN
        DELETE FROM info WHERE id = PARAM_ID;
    END IF;
END%

-- Config que o firmware puxa (GET /config?id=): E_b e timer.
DROP PROCEDURE IF EXISTS info_GetConfig %
CREATE PROCEDURE info_GetConfig(IN PARAM_ID INT)
BEGIN
    SELECT id, E_b, timer FROM info WHERE id = PARAM_ID LIMIT 1;
END%

-- Grava um ponto no historico (chamada pela logic layer a cada push).
DROP PROCEDURE IF EXISTS info_historico_Add %
CREATE PROCEDURE info_historico_Add(IN PARAM_ID INT, IN PARAM_E_B INT, IN PARAM_E_R INT, IN PARAM_TIMER INT, IN PARAM_M_ER INT, IN PARAM_V_MER INT, IN PARAM_M_VMER INT)
BEGIN
    IF EXISTS (SELECT 1 FROM conections WHERE id = PARAM_ID) THEN
        INSERT INTO info_historico(id_conection, E_b, E_r, timer, M_er, V_mer, M_vmer)
        VALUES (PARAM_ID, PARAM_E_B, PARAM_E_R, PARAM_TIMER, PARAM_M_ER, PARAM_V_MER, PARAM_M_VMER);
    END IF;
END%

-- Ultimos N pontos do historico (graficos).
DROP PROCEDURE IF EXISTS info_historico_Index %
CREATE PROCEDURE info_historico_Index(IN PARAM_ID INT, IN PARAM_LIMITE INT)
BEGIN
    SELECT id, id_conection, E_b, E_r, timer, M_er, V_mer, M_vmer, criado_em
    FROM info_historico
    WHERE id_conection = PARAM_ID
    ORDER BY criado_em DESC
    LIMIT PARAM_LIMITE;
END%

-- ==================== REFRESH TOKEN (sessao) ================

DROP PROCEDURE IF EXISTS refresh_token_Create %
CREATE PROCEDURE refresh_token_Create(IN PARAM_ID_USUARIO INT, IN PARAM_HASH CHAR(64), IN PARAM_EXPIRA DATETIME)
BEGIN
    INSERT INTO refresh_token(id_usuario, token_hash, expira_em) VALUES (PARAM_ID_USUARIO, PARAM_HASH, PARAM_EXPIRA);
END%

DROP PROCEDURE IF EXISTS refresh_token_Find %
CREATE PROCEDURE refresh_token_Find(IN PARAM_HASH CHAR(64))
BEGIN
    SELECT id, id_usuario, token_hash, expira_em, revogado
    FROM refresh_token
    WHERE token_hash = PARAM_HASH AND revogado = FALSE AND expira_em > NOW()
    LIMIT 1;
END%

DROP PROCEDURE IF EXISTS refresh_token_Revoke %
CREATE PROCEDURE refresh_token_Revoke(IN PARAM_HASH CHAR(64))
BEGIN
    UPDATE refresh_token SET revogado = TRUE WHERE token_hash = PARAM_HASH;
END%

DROP PROCEDURE IF EXISTS refresh_token_RevokeAllByUsuario %
CREATE PROCEDURE refresh_token_RevokeAllByUsuario(IN PARAM_ID_USUARIO INT)
BEGIN
    UPDATE refresh_token SET revogado = TRUE WHERE id_usuario = PARAM_ID_USUARIO;
END%

DELIMITER ;
