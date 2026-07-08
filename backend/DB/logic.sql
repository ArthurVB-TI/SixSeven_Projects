-- ============================================================
--  logic.sql  -  Camada de Servico do banco (dispatchers)
--  Equivale ao antigo POO.sql: um "Controller" por tabela que
--  recebe uma ACAO e roteia para o CRUD certo. Inclui a
--  orquestracao do push do hardware (info + historico numa
--  transacao) e o dispatcher de refresh_token.
-- ============================================================

USE SixSeven_Projects;

DELIMITER %

-- ============================ USUARIO =======================
DROP PROCEDURE IF EXISTS usuario_Controller %
CREATE PROCEDURE usuario_Controller(IN ACAO VARCHAR(20), IN PARAM_ID INT, IN PARAM_NOME VARCHAR(100), IN PARAM_EMAIL VARCHAR(100), IN PARAM_SENHA VARCHAR(255))
BEGIN
    IF (ACAO = 'Create') THEN
        CALL usuario_Create(PARAM_NOME, PARAM_EMAIL, PARAM_SENHA);
    ELSEIF (ACAO = 'Update') THEN
        CALL usuario_Update(PARAM_ID, PARAM_NOME, PARAM_EMAIL, PARAM_SENHA);
    ELSEIF (ACAO = 'Delete') THEN
        CALL usuario_Delete(PARAM_ID);
    ELSEIF (ACAO = 'FindByEmail') THEN
        CALL usuario_FindByEmail(PARAM_EMAIL);
    ELSEIF (ACAO = 'Index') THEN
        SELECT * FROM index_usuario;
    END IF;
END%

-- ============================ CONECTIONS ====================
DROP PROCEDURE IF EXISTS conections_Controller %
CREATE PROCEDURE conections_Controller(IN ACAO VARCHAR(20), IN PARAM_ID INT, IN PARAM_NOME VARCHAR(100), IN PARAM_CAN_ACTIVE BOOLEAN, IN PARAM_IS_ACTIVE BOOLEAN, IN PARAM_ID_USUARIO INT)
BEGIN
    IF (ACAO = 'Create') THEN
        CALL conections_Create(PARAM_NOME, PARAM_CAN_ACTIVE, PARAM_IS_ACTIVE, PARAM_ID_USUARIO);
    ELSEIF (ACAO = 'Update') THEN
        CALL conections_Update(PARAM_ID, PARAM_NOME, PARAM_CAN_ACTIVE, PARAM_IS_ACTIVE, PARAM_ID_USUARIO);
    ELSEIF (ACAO = 'Delete') THEN
        CALL conections_Delete(PARAM_ID);
    ELSEIF (ACAO = 'IndexByUsuario') THEN
        CALL conections_IndexByUsuario(PARAM_ID_USUARIO);
    ELSEIF (ACAO = 'Index') THEN
        SELECT * FROM index_conections;
    END IF;
END%

-- ============================ DISPOSITIVOS ==================
DROP PROCEDURE IF EXISTS dispositivos_Controller %
CREATE PROCEDURE dispositivos_Controller(IN ACAO VARCHAR(20), IN PARAM_ID INT, IN PARAM_NOME VARCHAR(100), IN PARAM_TIPO VARCHAR(50), IN PARAM_ID_USUARIO INT)
BEGIN
    IF (ACAO = 'Create') THEN
        CALL dispositivos_Create(PARAM_NOME, PARAM_TIPO, PARAM_ID_USUARIO);
    ELSEIF (ACAO = 'Update') THEN
        CALL dispositivos_Update(PARAM_ID, PARAM_NOME, PARAM_TIPO, PARAM_ID_USUARIO);
    ELSEIF (ACAO = 'Delete') THEN
        CALL dispositivos_Delete(PARAM_ID);
    ELSEIF (ACAO = 'Index') THEN
        SELECT * FROM index_dispositivos;
    END IF;
END%

-- ==================== DISPOSITIVOS_CONECTIONS ===============
DROP PROCEDURE IF EXISTS dispositivos_conections_Controller %
CREATE PROCEDURE dispositivos_conections_Controller(IN ACAO VARCHAR(20), IN PARAM_ID_CONECTIONS INT, IN PARAM_ID_DISPOSITIVOS INT)
BEGIN
    IF (ACAO = 'Create') THEN
        CALL dispositivos_conections_Create(PARAM_ID_CONECTIONS, PARAM_ID_DISPOSITIVOS);
    ELSEIF (ACAO = 'Delete') THEN
        CALL dispositivos_conections_Delete(PARAM_ID_CONECTIONS, PARAM_ID_DISPOSITIVOS);
    ELSEIF (ACAO = 'Index') THEN
        SELECT * FROM index_dispositivos_conections;
    END IF;
END%

-- ============================ INFO ==========================
DROP PROCEDURE IF EXISTS info_Controller %
CREATE PROCEDURE info_Controller(IN ACAO VARCHAR(20), IN PARAM_ID INT, IN PARAM_E_B INT, IN PARAM_E_R INT, IN PARAM_TIMER INT, IN PARAM_M_ER INT, IN PARAM_V_MER INT, IN PARAM_M_VMER INT)
BEGIN
    IF (ACAO = 'Create') THEN
        CALL info_Create(PARAM_ID, PARAM_E_B, PARAM_E_R, PARAM_TIMER, PARAM_M_ER, PARAM_V_MER, PARAM_M_VMER);
    ELSEIF (ACAO = 'Update') THEN
        CALL info_Update(PARAM_ID, PARAM_E_B, PARAM_E_R, PARAM_TIMER, PARAM_M_ER, PARAM_V_MER, PARAM_M_VMER);
    ELSEIF (ACAO = 'Delete') THEN
        CALL info_Delete(PARAM_ID);
    ELSEIF (ACAO = 'GetConfig') THEN
        CALL info_GetConfig(PARAM_ID);
    ELSEIF (ACAO = 'Index') THEN
        SELECT * FROM index_info;
    END IF;
END%

-- ------------------------------------------------------------
--  info_Push: chamada quando o hardware envia leitura (POST /data).
--  Faz upsert na tabela info E grava um ponto no historico,
--  tudo numa transacao. Preserva o E_b/timer configurados pelo
--  site: o hardware envia E_r/M_er/... e o back mantem a config.
-- ------------------------------------------------------------
DROP PROCEDURE IF EXISTS info_Push %
CREATE PROCEDURE info_Push(IN PARAM_ID INT, IN PARAM_E_B INT, IN PARAM_E_R INT, IN PARAM_TIMER INT, IN PARAM_M_ER INT, IN PARAM_V_MER INT, IN PARAM_M_VMER INT)
BEGIN
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        RESIGNAL;
    END;

    START TRANSACTION;

    IF EXISTS (SELECT 1 FROM info WHERE id = PARAM_ID) THEN
        UPDATE info
        SET E_r = PARAM_E_R, M_er = PARAM_M_ER, V_mer = PARAM_V_MER, M_vmer = PARAM_M_VMER
        WHERE id = PARAM_ID;
    ELSE
        INSERT INTO info(id, E_b, E_r, timer, M_er, V_mer, M_vmer)
        VALUES (PARAM_ID, PARAM_E_B, PARAM_E_R, PARAM_TIMER, PARAM_M_ER, PARAM_V_MER, PARAM_M_VMER);
    END IF;

    CALL info_historico_Add(PARAM_ID, PARAM_E_B, PARAM_E_R, PARAM_TIMER, PARAM_M_ER, PARAM_V_MER, PARAM_M_VMER);

    COMMIT;
END%

-- ------------------------------------------------------------
--  info_SetConfig: chamada pelo site (manipular hardware).
--  Grava apenas E_b e timer; o hardware puxa via GET /config.
-- ------------------------------------------------------------
DROP PROCEDURE IF EXISTS info_SetConfig %
CREATE PROCEDURE info_SetConfig(IN PARAM_ID INT, IN PARAM_E_B INT, IN PARAM_TIMER INT)
BEGIN
    IF EXISTS (SELECT 1 FROM info WHERE id = PARAM_ID) THEN
        UPDATE info SET E_b = PARAM_E_B, timer = PARAM_TIMER WHERE id = PARAM_ID;
    ELSEIF EXISTS (SELECT 1 FROM conections WHERE id = PARAM_ID) THEN
        INSERT INTO info(id, E_b, E_r, timer, M_er, V_mer, M_vmer)
        VALUES (PARAM_ID, PARAM_E_B, 0, PARAM_TIMER, 0, 0, 0);
    END IF;
END%

-- ==================== REFRESH TOKEN dispatcher ==============
DROP PROCEDURE IF EXISTS refresh_token_Controller %
CREATE PROCEDURE refresh_token_Controller(IN ACAO VARCHAR(20), IN PARAM_ID_USUARIO INT, IN PARAM_HASH CHAR(64), IN PARAM_EXPIRA DATETIME)
BEGIN
    IF (ACAO = 'Create') THEN
        CALL refresh_token_Create(PARAM_ID_USUARIO, PARAM_HASH, PARAM_EXPIRA);
    ELSEIF (ACAO = 'Find') THEN
        CALL refresh_token_Find(PARAM_HASH);
    ELSEIF (ACAO = 'Revoke') THEN
        CALL refresh_token_Revoke(PARAM_HASH);
    ELSEIF (ACAO = 'RevokeAll') THEN
        CALL refresh_token_RevokeAllByUsuario(PARAM_ID_USUARIO);
    END IF;
END%

DELIMITER ;
