-- ============================================================
--  functions.sql  -  Funcoes auxiliares do banco (helpers)
--  Regras de dominio reutilizaveis pelas procedures e views.
-- ============================================================

USE SixSeven_Projects;

DELIMITER %

-- ------------------------------------------------------------
--  fn_email_valido: valida formato de e-mail.
-- ------------------------------------------------------------
DROP FUNCTION IF EXISTS fn_email_valido %
CREATE FUNCTION fn_email_valido(p_email VARCHAR(100))
RETURNS BOOLEAN
DETERMINISTIC
BEGIN
    RETURN p_email REGEXP '^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$';
END%

-- ------------------------------------------------------------
--  fn_senha_valida: regra minima de senha (o HASH ja vem pronto
--  do backend; aqui garantimos que nao seja vazio/curto demais).
-- ------------------------------------------------------------
DROP FUNCTION IF EXISTS fn_senha_valida %
CREATE FUNCTION fn_senha_valida(p_senha VARCHAR(255))
RETURNS BOOLEAN
DETERMINISTIC
BEGIN
    RETURN CHAR_LENGTH(p_senha) >= 6;
END%

-- ------------------------------------------------------------
--  fn_percentual_energia: E_r em relacao a E_b (0-100+).
--  Base = quanto o hardware "deveria" receber.
-- ------------------------------------------------------------
DROP FUNCTION IF EXISTS fn_percentual_energia %
CREATE FUNCTION fn_percentual_energia(p_E_r INT, p_E_b INT)
RETURNS INT
DETERMINISTIC
BEGIN
    IF p_E_b IS NULL OR p_E_b <= 0 THEN
        RETURN 0;
    END IF;
    RETURN ROUND((p_E_r / p_E_b) * 100);
END%

-- ------------------------------------------------------------
--  fn_is_estavel: replica a logica do firmware (isStable).
--  V_mer <= limiar => estavel.
-- ------------------------------------------------------------
DROP FUNCTION IF EXISTS fn_is_estavel %
CREATE FUNCTION fn_is_estavel(p_V_mer INT, p_limiar INT)
RETURNS BOOLEAN
DETERMINISTIC
BEGIN
    RETURN ABS(p_V_mer) <= p_limiar;
END%

-- ------------------------------------------------------------
--  fn_conta_conections_usuario: total de conexoes de um usuario.
-- ------------------------------------------------------------
DROP FUNCTION IF EXISTS fn_conta_conections_usuario %
CREATE FUNCTION fn_conta_conections_usuario(p_id_usuario INT)
RETURNS INT
READS SQL DATA
BEGIN
    DECLARE v_total INT DEFAULT 0;
    SELECT COUNT(*) INTO v_total
    FROM conections
    WHERE id_usuario = p_id_usuario;
    RETURN v_total;
END%

DELIMITER ;
