CREATE OR REPLACE VIEW index_usuario AS
SELECT
    id,
    nome,
    email,
    senha
FROM 
    usuario;

CREATE OR REPLACE VIEW index_conections AS
SELECT
    id,
    nome,
    can_Active,
    is_Active,
    id_usuario,
FROM 
    concetions;

CREATE OR REPLACE VIEW index_dispositivos AS
SELECT
    id,
    nome,
    marca,
    modelo,
    can_Active,
    is_Active,
    id_usuario,
FROM
    dispositivos;

CREATE OR REPLACE VIEW index_dispositivos_conections AS
SELECT
    id_conection,
    id_dispositivo,
FROM
    dispositivos_conections;

CREATE OR REPLACE VIEW index_info AS
SELECT
    id,
    E_b,
    E_r,
    timer,
    M_er,
    V_mer,
    M_vmer
FROM
    info;