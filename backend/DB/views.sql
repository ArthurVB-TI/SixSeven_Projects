-- ============================================================
--  views.sql  -  Visoes de leitura (index_*)
--  Correcoes: virgulas finais removidas, "concetions" -> conections,
--  colunas inexistentes (marca/modelo) removidas de dispositivos,
--  nomes de coluna alinhados as tabelas reais.
--  index_usuario NAO expoe a senha (seguranca).
-- ============================================================

USE SixSeven_Projects;

CREATE OR REPLACE VIEW index_usuario AS
SELECT
    id,
    nome,
    email
FROM
    usuario;

CREATE OR REPLACE VIEW index_conections AS
SELECT
    id,
    nome,
    can_Active,
    is_Active,
    id_usuario
FROM
    conections;

CREATE OR REPLACE VIEW index_dispositivos AS
SELECT
    id,
    nome,
    tipo,
    id_usuario
FROM
    dispositivos;

CREATE OR REPLACE VIEW index_dispositivos_conections AS
SELECT
    id_conections,
    id_dispositivos
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
    M_vmer,
    atualizado_em
FROM
    info;

-- Visao rica: junta conexao + ultima leitura + dono.
-- Usada pela pagina de "informacoes" do site.
CREATE OR REPLACE VIEW index_conection_info AS
SELECT
    c.id              AS id_conection,
    c.nome            AS conection_nome,
    c.can_Active,
    c.is_Active,
    c.id_usuario,
    u.nome            AS usuario_nome,
    i.E_b, i.E_r, i.timer, i.M_er, i.V_mer, i.M_vmer,
    i.atualizado_em
FROM
    conections c
    INNER JOIN usuario u ON u.id = c.id_usuario
    LEFT  JOIN info    i ON i.id = c.id;
