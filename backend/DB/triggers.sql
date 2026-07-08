-- ============================================================
--  triggers.sql  -  Auditoria automatica (tabela auditoria)
--  Um trigger de INSERT/UPDATE/DELETE por tabela de dominio.
--  Correcoes: nomes de coluna alinhados as tabelas reais.
--  Nota: o UPDATE de senha registra apenas "Alterada".
-- ============================================================

USE SixSeven_Projects;

DELIMITER %

-- ============================ USUARIO =======================

DROP TRIGGER IF EXISTS INSERINDO_usuario %
CREATE TRIGGER INSERINDO_usuario
AFTER INSERT ON usuario
FOR EACH ROW
BEGIN
    INSERT INTO auditoria(Nome, Descricao) VALUES ('usuario',
        CONCAT('Novo usuario inserido:\n',
               'id: ', NEW.id, '\n',
               'nome: ', NEW.nome, '\n',
               'email: ', NEW.email));
END%

DROP TRIGGER IF EXISTS ALTERANDO_usuario %
CREATE TRIGGER ALTERANDO_usuario
BEFORE UPDATE ON usuario
FOR EACH ROW
BEGIN
    DECLARE VAR_TEXTO TEXT DEFAULT '';
    DECLARE VAR_QTDE INT DEFAULT 0;

    IF NOT (NEW.nome <=> OLD.nome) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nNOME: Antigo - ', OLD.nome, ', Novo - ', NEW.nome);
        SET VAR_QTDE = VAR_QTDE + 1;
    END IF;
    IF NOT (NEW.email <=> OLD.email) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nEMAIL: Antigo - ', OLD.email, ', Novo - ', NEW.email);
        SET VAR_QTDE = VAR_QTDE + 1;
    END IF;
    IF NOT (NEW.senha <=> OLD.senha) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nSENHA: Alterada');
        SET VAR_QTDE = VAR_QTDE + 1;
    END IF;

    IF (VAR_TEXTO != '') THEN
        INSERT INTO auditoria(Nome, Descricao) VALUES ('usuario',
            CONCAT('Houve ', VAR_QTDE, ' MODIFICACAO(OES) no usuario ', OLD.nome, ' (id: ', OLD.id, '):', VAR_TEXTO));
    END IF;
END%

DROP TRIGGER IF EXISTS EXCLUINDO_usuario %
CREATE TRIGGER EXCLUINDO_usuario
BEFORE DELETE ON usuario
FOR EACH ROW
BEGIN
    INSERT INTO auditoria(Nome, Descricao) VALUES ('usuario',
        CONCAT('Usuario deletado:\n',
               'id: ', OLD.id, '\n',
               'nome: ', OLD.nome, '\n',
               'email: ', OLD.email));
END%

-- ============================ CONECTIONS ====================

DROP TRIGGER IF EXISTS INSERINDO_conections %
CREATE TRIGGER INSERINDO_conections
AFTER INSERT ON conections
FOR EACH ROW
BEGIN
    INSERT INTO auditoria(Nome, Descricao) VALUES ('conections',
        CONCAT('Nova conection inserida:\n',
               'id: ', NEW.id, '\n',
               'nome: ', NEW.nome, '\n',
               'can_Active: ', NEW.can_Active, '\n',
               'is_Active: ', NEW.is_Active, '\n',
               'id_usuario: ', NEW.id_usuario));
END%

DROP TRIGGER IF EXISTS ALTERANDO_conections %
CREATE TRIGGER ALTERANDO_conections
BEFORE UPDATE ON conections
FOR EACH ROW
BEGIN
    DECLARE VAR_TEXTO TEXT DEFAULT '';
    DECLARE VAR_QTDE INT DEFAULT 0;

    IF NOT (NEW.nome <=> OLD.nome) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nNOME: Antigo - ', OLD.nome, ', Novo - ', NEW.nome);
        SET VAR_QTDE = VAR_QTDE + 1;
    END IF;
    IF NOT (NEW.can_Active <=> OLD.can_Active) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nCAN_ACTIVE: Antigo - ', OLD.can_Active, ', Novo - ', NEW.can_Active);
        SET VAR_QTDE = VAR_QTDE + 1;
    END IF;
    IF NOT (NEW.is_Active <=> OLD.is_Active) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nIS_ACTIVE: Antigo - ', OLD.is_Active, ', Novo - ', NEW.is_Active);
        SET VAR_QTDE = VAR_QTDE + 1;
    END IF;
    IF NOT (NEW.id_usuario <=> OLD.id_usuario) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nID_USUARIO: Antigo - ', OLD.id_usuario, ', Novo - ', NEW.id_usuario);
        SET VAR_QTDE = VAR_QTDE + 1;
    END IF;

    IF (VAR_TEXTO != '') THEN
        INSERT INTO auditoria(Nome, Descricao) VALUES ('conections',
            CONCAT('Houve ', VAR_QTDE, ' MODIFICACAO(OES) na conection ', OLD.nome, ' (id: ', OLD.id, '):', VAR_TEXTO));
    END IF;
END%

DROP TRIGGER IF EXISTS EXCLUINDO_conections %
CREATE TRIGGER EXCLUINDO_conections
BEFORE DELETE ON conections
FOR EACH ROW
BEGIN
    INSERT INTO auditoria(Nome, Descricao) VALUES ('conections',
        CONCAT('Conection deletada:\n',
               'id: ', OLD.id, '\n',
               'nome: ', OLD.nome, '\n',
               'id_usuario: ', OLD.id_usuario));
END%

-- ============================ DISPOSITIVOS ==================

DROP TRIGGER IF EXISTS INSERINDO_dispositivos %
CREATE TRIGGER INSERINDO_dispositivos
AFTER INSERT ON dispositivos
FOR EACH ROW
BEGIN
    INSERT INTO auditoria(Nome, Descricao) VALUES ('dispositivos',
        CONCAT('Novo dispositivo inserido:\n',
               'id: ', NEW.id, '\n',
               'nome: ', NEW.nome, '\n',
               'tipo: ', NEW.tipo, '\n',
               'id_usuario: ', NEW.id_usuario));
END%

DROP TRIGGER IF EXISTS ALTERANDO_dispositivos %
CREATE TRIGGER ALTERANDO_dispositivos
BEFORE UPDATE ON dispositivos
FOR EACH ROW
BEGIN
    DECLARE VAR_TEXTO TEXT DEFAULT '';
    DECLARE VAR_QTDE INT DEFAULT 0;

    IF NOT (NEW.nome <=> OLD.nome) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nNOME: Antigo - ', OLD.nome, ', Novo - ', NEW.nome);
        SET VAR_QTDE = VAR_QTDE + 1;
    END IF;
    IF NOT (NEW.tipo <=> OLD.tipo) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nTIPO: Antigo - ', OLD.tipo, ', Novo - ', NEW.tipo);
        SET VAR_QTDE = VAR_QTDE + 1;
    END IF;
    IF NOT (NEW.id_usuario <=> OLD.id_usuario) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nID_USUARIO: Antigo - ', OLD.id_usuario, ', Novo - ', NEW.id_usuario);
        SET VAR_QTDE = VAR_QTDE + 1;
    END IF;

    IF (VAR_TEXTO != '') THEN
        INSERT INTO auditoria(Nome, Descricao) VALUES ('dispositivos',
            CONCAT('Houve ', VAR_QTDE, ' MODIFICACAO(OES) no dispositivo ', OLD.nome, ' (id: ', OLD.id, '):', VAR_TEXTO));
    END IF;
END%

DROP TRIGGER IF EXISTS EXCLUINDO_dispositivos %
CREATE TRIGGER EXCLUINDO_dispositivos
BEFORE DELETE ON dispositivos
FOR EACH ROW
BEGIN
    INSERT INTO auditoria(Nome, Descricao) VALUES ('dispositivos',
        CONCAT('Dispositivo deletado:\n',
               'id: ', OLD.id, '\n',
               'nome: ', OLD.nome, '\n',
               'tipo: ', OLD.tipo, '\n',
               'id_usuario: ', OLD.id_usuario));
END%

-- ==================== DISPOSITIVOS_CONECTIONS ===============

DROP TRIGGER IF EXISTS INSERINDO_dispositivos_conections %
CREATE TRIGGER INSERINDO_dispositivos_conections
AFTER INSERT ON dispositivos_conections
FOR EACH ROW
BEGIN
    INSERT INTO auditoria(Nome, Descricao) VALUES ('dispositivos_conections',
        CONCAT('Nova relacao dispositivo-conection:\n',
               'id_conections: ', NEW.id_conections, '\n',
               'id_dispositivos: ', NEW.id_dispositivos));
END%

DROP TRIGGER IF EXISTS EXCLUINDO_dispositivos_conections %
CREATE TRIGGER EXCLUINDO_dispositivos_conections
BEFORE DELETE ON dispositivos_conections
FOR EACH ROW
BEGIN
    INSERT INTO auditoria(Nome, Descricao) VALUES ('dispositivos_conections',
        CONCAT('Relacao dispositivo-conection deletada:\n',
               'id_conections: ', OLD.id_conections, '\n',
               'id_dispositivos: ', OLD.id_dispositivos));
END%

-- ============================ INFO ==========================
-- Nota: info recebe muitos updates do hardware. Auditamos apenas
-- INSERT e DELETE para nao inundar a auditoria a cada leitura.

DROP TRIGGER IF EXISTS INSERINDO_info %
CREATE TRIGGER INSERINDO_info
AFTER INSERT ON info
FOR EACH ROW
BEGIN
    INSERT INTO auditoria(Nome, Descricao) VALUES ('info',
        CONCAT('Nova info inserida:\n',
               'id: ', NEW.id, '\n',
               'E_b: ', NEW.E_b, '  E_r: ', NEW.E_r, '  timer: ', NEW.timer));
END%

DROP TRIGGER IF EXISTS EXCLUINDO_info %
CREATE TRIGGER EXCLUINDO_info
BEFORE DELETE ON info
FOR EACH ROW
BEGIN
    INSERT INTO auditoria(Nome, Descricao) VALUES ('info',
        CONCAT('Info deletada:\n',
               'id: ', OLD.id, '\n',
               'E_b: ', OLD.E_b, '  E_r: ', OLD.E_r));
END%

DELIMITER ;
