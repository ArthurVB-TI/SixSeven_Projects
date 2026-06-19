DELIMITER %

-- usuario

CREATE TRIGGER INSERINDO_usuario
AFTER INSERT ON usuario
FOR EACH ROW
BEGIN
    DECLARE VAR_TEXTO TEXT DEFAULT '';
    DECLARE VAR_NOME_TABELA VARCHAR(30) DEFAULT 'usuario';

    SET VAR_TEXTO = CONCAT(
        'Novo usuario inserido:\n',
        'id: ', NEW.id, '\n',
        'nome: ', NEW.nome, '\n',
        'email: ', NEW.email
    );

    INSERT INTO auditoria(Nome, Descricao) VALUES (VAR_NOME_TABELA, VAR_TEXTO);
END%

CREATE TRIGGER ALTERANDO_usuario
BEFORE UPDATE ON usuario
FOR EACH ROW
BEGIN
    DECLARE VAR_TEXTO TEXT DEFAULT '';
    DECLARE VAR_QTDE_ALTERACOES INT DEFAULT 0;
    DECLARE VAR_NOME_TABELA VARCHAR(30) DEFAULT 'usuario';

    IF NOT (NEW.nome <=> OLD.nome) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nNOME: Antigo - ', OLD.nome, ', Novo - ', NEW.nome);
        SET VAR_QTDE_ALTERACOES = VAR_QTDE_ALTERACOES + 1;
    END IF;
    IF NOT (NEW.email <=> OLD.email) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nEMAIL: Antigo - ', OLD.email, ', Novo - ', NEW.email);
        SET VAR_QTDE_ALTERACOES = VAR_QTDE_ALTERACOES + 1;
    END IF;
    IF NOT (NEW.senha <=> OLD.senha) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nSENHA: Alterada');
        SET VAR_QTDE_ALTERACOES = VAR_QTDE_ALTERACOES + 1;
    END IF;

    IF (VAR_TEXTO != '') THEN
        SET VAR_TEXTO = CONCAT('Houve ', VAR_QTDE_ALTERACOES, ' MODIFICAÇÃO(ÕES) no usuario ', OLD.nome, ' (id: ', OLD.id, ') com as seguintes ALTERAÇÕES:', VAR_TEXTO);
        INSERT INTO auditoria(Nome, Descricao) VALUES (VAR_NOME_TABELA, VAR_TEXTO);
    ELSE
        SIGNAL SQLSTATE '45000'
        SET MESSAGE_TEXT = 'Nenhuma informação alterada';
    END IF;
END%

CREATE TRIGGER EXCLUINDO_usuario
BEFORE DELETE ON usuario
FOR EACH ROW
BEGIN
    DECLARE VAR_TEXTO TEXT DEFAULT '';
    DECLARE VAR_NOME_TABELA VARCHAR(30) DEFAULT 'usuario';

    SET VAR_TEXTO = CONCAT(
        'Usuario deletado:\n',
        'id: ', OLD.id, '\n',
        'nome: ', OLD.nome, '\n',
        'email: ', OLD.email
    );

    INSERT INTO auditoria(Nome, Descricao) VALUES (VAR_NOME_TABELA, VAR_TEXTO);
END%

-- conections

CREATE TRIGGER INSERINDO_conections
AFTER INSERT ON conections
FOR EACH ROW
BEGIN
    DECLARE VAR_TEXTO TEXT DEFAULT '';
    DECLARE VAR_NOME_TABELA VARCHAR(30) DEFAULT 'conections';

    SET VAR_TEXTO = CONCAT(
        'Nova conection inserida:\n',
        'id: ', NEW.id, '\n',
        'nome: ', NEW.nome, '\n',
        'can_Active: ', NEW.can_Active, '\n',
        'is_Active: ', NEW.is_Active, '\n',
        'id_usuario: ', NEW.id_usuario
    );

    INSERT INTO auditoria(Nome, Descricao) VALUES (VAR_NOME_TABELA, VAR_TEXTO);
END%

CREATE TRIGGER ALTERANDO_conections
BEFORE UPDATE ON conections
FOR EACH ROW
BEGIN
    DECLARE VAR_TEXTO TEXT DEFAULT '';
    DECLARE VAR_QTDE_ALTERACOES INT DEFAULT 0;
    DECLARE VAR_NOME_TABELA VARCHAR(30) DEFAULT 'conections';

    IF NOT (NEW.nome <=> OLD.nome) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nNOME: Antigo - ', OLD.nome, ', Novo - ', NEW.nome);
        SET VAR_QTDE_ALTERACOES = VAR_QTDE_ALTERACOES + 1;
    END IF;
    IF NOT (NEW.can_Active <=> OLD.can_Active) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nCAN_ACTIVE: Antigo - ', OLD.can_Active, ', Novo - ', NEW.can_Active);
        SET VAR_QTDE_ALTERACOES = VAR_QTDE_ALTERACOES + 1;
    END IF;
    IF NOT (NEW.is_Active <=> OLD.is_Active) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nIS_ACTIVE: Antigo - ', OLD.is_Active, ', Novo - ', NEW.is_Active);
        SET VAR_QTDE_ALTERACOES = VAR_QTDE_ALTERACOES + 1;
    END IF;
    IF NOT (NEW.id_usuario <=> OLD.id_usuario) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nID_USUARIO: Antigo - ', OLD.id_usuario, ', Novo - ', NEW.id_usuario);
        SET VAR_QTDE_ALTERACOES = VAR_QTDE_ALTERACOES + 1;
    END IF;

    IF (VAR_TEXTO != '') THEN
        SET VAR_TEXTO = CONCAT('Houve ', VAR_QTDE_ALTERACOES, ' MODIFICAÇÃO(ÕES) na conection ', OLD.nome, ' (id: ', OLD.id, ') com as seguintes ALTERAÇÕES:', VAR_TEXTO);
        INSERT INTO auditoria(Nome, Descricao) VALUES (VAR_NOME_TABELA, VAR_TEXTO);
    ELSE
        SIGNAL SQLSTATE '45000'
        SET MESSAGE_TEXT = 'Nenhuma informação alterada';
    END IF;
END%

CREATE TRIGGER EXCLUINDO_conections
BEFORE DELETE ON conections
FOR EACH ROW
BEGIN
    DECLARE VAR_TEXTO TEXT DEFAULT '';
    DECLARE VAR_NOME_TABELA VARCHAR(30) DEFAULT 'conections';

    SET VAR_TEXTO = CONCAT(
        'Conection deletada:\n',
        'id: ', OLD.id, '\n',
        'nome: ', OLD.nome, '\n',
        'can_Active: ', OLD.can_Active, '\n',
        'is_Active: ', OLD.is_Active, '\n',
        'id_usuario: ', OLD.id_usuario
    );

    INSERT INTO auditoria(Nome, Descricao) VALUES (VAR_NOME_TABELA, VAR_TEXTO);
END%

-- dispositivos

CREATE TRIGGER INSERINDO_dispositivos
AFTER INSERT ON dispositivos
FOR EACH ROW
BEGIN
    DECLARE VAR_TEXTO TEXT DEFAULT '';
    DECLARE VAR_NOME_TABELA VARCHAR(30) DEFAULT 'dispositivos';

    SET VAR_TEXTO = CONCAT(
        'Novo dispositivo inserido:\n',
        'id: ', NEW.id, '\n',
        'nome: ', NEW.nome, '\n',
        'tipo: ', NEW.tipo, '\n',
        'id_usuario: ', NEW.id_usuario
    );

    INSERT INTO auditoria(Nome, Descricao) VALUES (VAR_NOME_TABELA, VAR_TEXTO);
END%

CREATE TRIGGER ALTERANDO_dispositivos
BEFORE UPDATE ON dispositivos
FOR EACH ROW
BEGIN
    DECLARE VAR_TEXTO TEXT DEFAULT '';
    DECLARE VAR_QTDE_ALTERACOES INT DEFAULT 0;
    DECLARE VAR_NOME_TABELA VARCHAR(30) DEFAULT 'dispositivos';

    IF NOT (NEW.nome <=> OLD.nome) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nNOME: Antigo - ', OLD.nome, ', Novo - ', NEW.nome);
        SET VAR_QTDE_ALTERACOES = VAR_QTDE_ALTERACOES + 1;
    END IF;
    IF NOT (NEW.tipo <=> OLD.tipo) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nTIPO: Antigo - ', OLD.tipo, ', Novo - ', NEW.tipo);
        SET VAR_QTDE_ALTERACOES = VAR_QTDE_ALTERACOES + 1;
    END IF;
    IF NOT (NEW.id_usuario <=> OLD.id_usuario) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nID_USUARIO: Antigo - ', OLD.id_usuario, ', Novo - ', NEW.id_usuario);
        SET VAR_QTDE_ALTERACOES = VAR_QTDE_ALTERACOES + 1;
    END IF;

    IF (VAR_TEXTO != '') THEN
        SET VAR_TEXTO = CONCAT('Houve ', VAR_QTDE_ALTERACOES, ' MODIFICAÇÃO(ÕES) no dispositivo ', OLD.nome, ' (id: ', OLD.id, ') com as seguintes ALTERAÇÕES:', VAR_TEXTO);
        INSERT INTO auditoria(Nome, Descricao) VALUES (VAR_NOME_TABELA, VAR_TEXTO);
    ELSE
        SIGNAL SQLSTATE '45000'
        SET MESSAGE_TEXT = 'Nenhuma informação alterada';
    END IF;
END%

CREATE TRIGGER EXCLUINDO_dispositivos
BEFORE DELETE ON dispositivos
FOR EACH ROW
BEGIN
    DECLARE VAR_TEXTO TEXT DEFAULT '';
    DECLARE VAR_NOME_TABELA VARCHAR(30) DEFAULT 'dispositivos';

    SET VAR_TEXTO = CONCAT(
        'Dispositivo deletado:\n',
        'id: ', OLD.id, '\n',
        'nome: ', OLD.nome, '\n',
        'tipo: ', OLD.tipo, '\n',
        'id_usuario: ', OLD.id_usuario
    );

    INSERT INTO auditoria(Nome, Descricao) VALUES (VAR_NOME_TABELA, VAR_TEXTO);
END%

-- dispositivos_conections

CREATE TRIGGER INSERINDO_dispositivos_conections
AFTER INSERT ON dispositivos_conections
FOR EACH ROW
BEGIN
    DECLARE VAR_TEXTO TEXT DEFAULT '';
    DECLARE VAR_NOME_TABELA VARCHAR(30) DEFAULT 'dispositivos_conections';

    SET VAR_TEXTO = CONCAT(
        'Nova relação dispositivo-conection inserida:\n',
        'id_conections: ', NEW.id_conections, '\n',
        'id_dispositivos: ', NEW.id_dispositivos
    );

    INSERT INTO auditoria(Nome, Descricao) VALUES (VAR_NOME_TABELA, VAR_TEXTO);
END%

CREATE TRIGGER EXCLUINDO_dispositivos_conections
BEFORE DELETE ON dispositivos_conections
FOR EACH ROW
BEGIN
    DECLARE VAR_TEXTO TEXT DEFAULT '';
    DECLARE VAR_NOME_TABELA VARCHAR(30) DEFAULT 'dispositivos_conections';

    SET VAR_TEXTO = CONCAT(
        'Relação dispositivo-conection deletada:\n',
        'id_conections: ', OLD.id_conections, '\n',
        'id_dispositivos: ', OLD.id_dispositivos
    );

    INSERT INTO auditoria(Nome, Descricao) VALUES (VAR_NOME_TABELA, VAR_TEXTO);
END%

-- info

CREATE TRIGGER INSERINDO_info
AFTER INSERT ON info
FOR EACH ROW
BEGIN
    DECLARE VAR_TEXTO TEXT DEFAULT '';
    DECLARE VAR_NOME_TABELA VARCHAR(30) DEFAULT 'info';

    SET VAR_TEXTO = CONCAT(
        'Nova info inserida:\n',
        'id: ', NEW.id, '\n',
        'E_b: ', NEW.E_b, '\n',
        'E_r: ', NEW.E_r, '\n',
        'timer: ', NEW.timer, '\n',
        'M_er: ', NEW.M_er, '\n',
        'V_mer: ', NEW.V_mer, '\n',
        'M_vmer: ', NEW.M_vmer
    );

    INSERT INTO auditoria(Nome, Descricao) VALUES (VAR_NOME_TABELA, VAR_TEXTO);
END%

CREATE TRIGGER ALTERANDO_info
BEFORE UPDATE ON info
FOR EACH ROW
BEGIN
    DECLARE VAR_TEXTO TEXT DEFAULT '';
    DECLARE VAR_QTDE_ALTERACOES INT DEFAULT 0;
    DECLARE VAR_NOME_TABELA VARCHAR(30) DEFAULT 'info';

    IF NOT (NEW.E_b <=> OLD.E_b) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nE_B: Antigo - ', OLD.E_b, ', Novo - ', NEW.E_b);
        SET VAR_QTDE_ALTERACOES = VAR_QTDE_ALTERACOES + 1;
    END IF;
    IF NOT (NEW.E_r <=> OLD.E_r) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nE_R: Antigo - ', OLD.E_r, ', Novo - ', NEW.E_r);
        SET VAR_QTDE_ALTERACOES = VAR_QTDE_ALTERACOES + 1;
    END IF;
    IF NOT (NEW.timer <=> OLD.timer) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nTIMER: Antigo - ', OLD.timer, ', Novo - ', NEW.timer);
        SET VAR_QTDE_ALTERACOES = VAR_QTDE_ALTERACOES + 1;
    END IF;
    IF NOT (NEW.M_er <=> OLD.M_er) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nM_ER: Antigo - ', OLD.M_er, ', Novo - ', NEW.M_er);
        SET VAR_QTDE_ALTERACOES = VAR_QTDE_ALTERACOES + 1;
    END IF;
    IF NOT (NEW.V_mer <=> OLD.V_mer) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nV_MER: Antigo - ', OLD.V_mer, ', Novo - ', NEW.V_mer);
        SET VAR_QTDE_ALTERACOES = VAR_QTDE_ALTERACOES + 1;
    END IF;
    IF NOT (NEW.M_vmer <=> OLD.M_vmer) THEN
        SET VAR_TEXTO = CONCAT(VAR_TEXTO, '\nM_VMER: Antigo - ', OLD.M_vmer, ', Novo - ', NEW.M_vmer);
        SET VAR_QTDE_ALTERACOES = VAR_QTDE_ALTERACOES + 1;
    END IF;

    IF (VAR_TEXTO != '') THEN
        SET VAR_TEXTO = CONCAT('Houve ', VAR_QTDE_ALTERACOES, ' MODIFICAÇÃO(ÕES) na info (id: ', OLD.id, ') com as seguintes ALTERAÇÕES:', VAR_TEXTO);
        INSERT INTO auditoria(Nome, Descricao) VALUES (VAR_NOME_TABELA, VAR_TEXTO);
    ELSE
        SIGNAL SQLSTATE '45000'
        SET MESSAGE_TEXT = 'Nenhuma informação alterada';
    END IF;
END%

CREATE TRIGGER EXCLUINDO_info
BEFORE DELETE ON info
FOR EACH ROW
BEGIN
    DECLARE VAR_TEXTO TEXT DEFAULT '';
    DECLARE VAR_NOME_TABELA VARCHAR(30) DEFAULT 'info';

    SET VAR_TEXTO = CONCAT(
        'Info deletada:\n',
        'id: ', OLD.id, '\n',
        'E_b: ', OLD.E_b, '\n',
        'E_r: ', OLD.E_r, '\n',
        'timer: ', OLD.timer, '\n',
        'M_er: ', OLD.M_er, '\n',
        'V_mer: ', OLD.V_mer, '\n',
        'M_vmer: ', OLD.M_vmer
    );

    INSERT INTO auditoria(Nome, Descricao) VALUES (VAR_NOME_TABELA, VAR_TEXTO);
END%

DELIMITER ;
