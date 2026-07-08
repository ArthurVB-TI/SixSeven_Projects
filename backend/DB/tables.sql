-- ============================================================
--  tables.sql  -  Definicao das tabelas
--  Mantidas as tabelas originais. Correcoes aplicadas:
--   * dispositivos: FK corrigida (id_usuario -> usuario, era
--     "id_conection" inexistente apontando para usuario).
--   * usuario.senha ampliada p/ 255 (hash bcrypt/argon2 seguro).
--   * Adicionadas: info_historico (serie temporal p/ graficos)
--     e refresh_token (sessao persistente / manter logado).
-- ============================================================

USE SixSeven_Projects;

-- Ordem de DROP respeita as dependencias (filhas primeiro)
DROP TABLE IF EXISTS refresh_token;
DROP TABLE IF EXISTS info_historico;
DROP TABLE IF EXISTS auditoria;
DROP TABLE IF EXISTS info;
DROP TABLE IF EXISTS dispositivos_conections;
DROP TABLE IF EXISTS dispositivos;
DROP TABLE IF EXISTS conections;
DROP TABLE IF EXISTS usuario;

-- ------------------------------------------------------------
--  usuario
-- ------------------------------------------------------------
CREATE TABLE usuario(
    id INT PRIMARY KEY AUTO_INCREMENT,
    nome VARCHAR(100) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    senha VARCHAR(255) NOT NULL                 /* hash bcrypt/argon2 */
)ENGINE=INNODB;

-- ------------------------------------------------------------
--  conections  (1 conexao = 1 hardware)
-- ------------------------------------------------------------
CREATE TABLE conections(
    id INT PRIMARY KEY AUTO_INCREMENT,
    nome VARCHAR(100) NOT NULL,
    can_Active BOOLEAN DEFAULT FALSE,           /* hardware pode ser ligado */
    is_Active BOOLEAN DEFAULT FALSE,            /* hardware esta ligado */
    id_usuario INT NOT NULL,
    FOREIGN KEY (id_usuario) REFERENCES usuario(id)
        ON UPDATE CASCADE
        ON DELETE CASCADE
)ENGINE=INNODB;

-- ------------------------------------------------------------
--  dispositivos  (FK corrigida: id_usuario -> usuario)
-- ------------------------------------------------------------
CREATE TABLE dispositivos(
    id INT PRIMARY KEY AUTO_INCREMENT,
    nome VARCHAR(100) NOT NULL,
    tipo VARCHAR(50) NOT NULL,
    id_usuario INT NOT NULL,
    FOREIGN KEY (id_usuario) REFERENCES usuario(id)
        ON UPDATE CASCADE
        ON DELETE CASCADE
)ENGINE=INNODB;

-- ------------------------------------------------------------
--  dispositivos_conections  (N:N)
-- ------------------------------------------------------------
CREATE TABLE dispositivos_conections(
    id_conections INT NOT NULL,
    id_dispositivos INT NOT NULL,
    PRIMARY KEY (id_conections, id_dispositivos),
    FOREIGN KEY (id_conections) REFERENCES conections(id)
        ON UPDATE CASCADE
        ON DELETE CASCADE,
    FOREIGN KEY (id_dispositivos) REFERENCES dispositivos(id)
        ON UPDATE CASCADE
        ON DELETE CASCADE
)ENGINE=INNODB;

-- ------------------------------------------------------------
--  info  (ultima leitura por conexao/hardware)
--  id = id da conexao (1:1). Espelha o payload do firmware.
-- ------------------------------------------------------------
CREATE TABLE info(
    id INT PRIMARY KEY,
    E_b INT NOT NULL,                           /* Energia Base */
    E_r INT NOT NULL,                           /* Energia Recebida */
    timer INT NOT NULL,                         /* Tempo (ms) */
    M_er INT NOT NULL,                          /* Media de energia recebida */
    V_mer INT NOT NULL,                         /* Variacao da M_er */
    M_vmer INT NOT NULL,                        /* Media da V_mer */
    atualizado_em TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (id) REFERENCES conections(id)
        ON UPDATE CASCADE
        ON DELETE CASCADE
)ENGINE=INNODB;

-- ------------------------------------------------------------
--  info_historico  (serie temporal p/ graficos)
--  Cada push do hardware gera 1 linha aqui.
-- ------------------------------------------------------------
CREATE TABLE info_historico(
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    id_conection INT NOT NULL,
    E_b INT NOT NULL,
    E_r INT NOT NULL,
    timer INT NOT NULL,
    M_er INT NOT NULL,
    V_mer INT NOT NULL,
    M_vmer INT NOT NULL,
    criado_em TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_hist_conection_data (id_conection, criado_em),
    FOREIGN KEY (id_conection) REFERENCES conections(id)
        ON UPDATE CASCADE
        ON DELETE CASCADE
)ENGINE=INNODB;

-- ------------------------------------------------------------
--  auditoria  (preenchida pelos triggers)
-- ------------------------------------------------------------
CREATE TABLE auditoria(
    id INT PRIMARY KEY AUTO_INCREMENT,
    Descricao TEXT,
    Nome VARCHAR(80) NOT NULL,
    criado_em TIMESTAMP DEFAULT CURRENT_TIMESTAMP
)ENGINE=INNODB;

-- ------------------------------------------------------------
--  refresh_token  (sessao persistente - "manter logado")
--  Guarda o hash do refresh token, nunca o token cru.
-- ------------------------------------------------------------
CREATE TABLE refresh_token(
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    id_usuario INT NOT NULL,
    token_hash CHAR(64) NOT NULL UNIQUE,        /* SHA-256 hex do token */
    expira_em DATETIME NOT NULL,
    revogado BOOLEAN NOT NULL DEFAULT FALSE,
    criado_em TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_rt_usuario (id_usuario),
    FOREIGN KEY (id_usuario) REFERENCES usuario(id)
        ON UPDATE CASCADE
        ON DELETE CASCADE
)ENGINE=INNODB;
