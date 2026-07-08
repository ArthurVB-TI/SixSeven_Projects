-- ============================================================
--  config.sql  -  Criacao do banco e parametros de sessao
--  Projeto: SixSeven_Projects
--  Ordem de execucao: config -> tables -> views -> functions
--                     -> CRUD -> logic -> triggers -> data
-- ============================================================

CREATE DATABASE IF NOT EXISTS SixSeven_Projects
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

USE SixSeven_Projects;

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 1;
SET SQL_MODE = 'STRICT_TRANS_TABLES,NO_ZERO_DATE,NO_ZERO_IN_DATE,ERROR_FOR_DIVISION_BY_ZERO';
