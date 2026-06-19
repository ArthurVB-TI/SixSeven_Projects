CREATE TABLE usuario(
    id INT PRIMARY KEY AUTO_INCREMENT,
    nome VARCHAR(100) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    senha VARCHAR(32) NOT NULL
)ENGINE=INNODB;

CREATE TABLE conections(
    id INT PRIMARY KEY AUTO_INCREMENT,
    nome VARCHAR(100) NOT NULL,
    can_Active BOOLEAN DEFAULT FALSE,
    is_Active BOOLEAN DEFAULT FALSE,
    id_usuario INT NOT NULL,
    FOREIGN KEY (id_usuario) REFERENCES usuario(id)
        ON UPDATE CASCADE
)ENGINE=INNODB;

CREATE TABLE dispositivos(
    id INT PRIMARY KEY AUTO_INCREMENT,
    nome VARCHAR(100) NOT NULL,
    tipo VARCHAR(50) NOT NULL,
    id_usuario INT NOT NULL,
    FOREIGN KEY (id_conection) REFERENCES usuario(id)
        ON UPDATE CASCADE
        ON DELETE CASCADE
)ENGINE=INNODB;

CREATE TABLE dispositivos_conections(
    id_conections INT NOT NULL,
    id_dispositivos INT NOT NULL,
    FOREIGN KEY (id_conections) REFERENCES conections(id)
        ON UPDATE CASCADE,
    FOREIGN KEY (id_dispositivos) REFERENCES dispositivos(id)
        ON UPDATE CASCADE
)ENGINE=INNODB;

CREATE TABLE info(
    id INT PRIMARY KEY,
    E_b INT NOT NULL,                           /* Energia Base */
    E_r INT NOT NULL,                           /* Energia Recebida */
    timer INT NOT NULL,                         /* Unidade de tempo em Millisegundos */
    M_er INT NOT NULL,                          /* Media de energia recebida */
    V_mer INT NOT NULL,                         /* Variação da M_er */
    M_vmer INT NOT NULL,                        /* Média da V_mer */
    FOREIGN KEY (id) REFERENCES conections(id)
        ON UPDATE CASCADE
)ENGINE=INNODB;

CREATE TABLE auditoria(
    id INT PRIMARY KEY AUTO_INCREMENT,
    Descricao TEXT,
    Nome VARCHAR(80) NOT NULL
)ENGINE=INNODB;