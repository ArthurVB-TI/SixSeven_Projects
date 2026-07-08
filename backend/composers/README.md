# composers/ — Gestão de dependências

Esta pasta guarda dependências que precisam ser "vendorizadas" (incluídas
no projeto), no espírito de um gerenciador de pacotes. Hoje há uma:

## libbcrypt (hash de senha)

O `PasswordHasher` (`app/Security/PasswordHasher.hpp`) inclui
`<bcrypt/bcrypt.h>`. Essa é a **libbcrypt** de Ricardo Garcia (wrapper sobre
o `crypt_blowfish` do Openwall), amplamente usada.

### Como obter

Como o ambiente pode estar sem acesso à internet, o download é manual.
Baixe uma vez e coloque o conteúdo em `composers/libbcrypt/`:

```bash
cd composers
git clone https://github.com/rg3/libbcrypt.git
# resultando em:
#   composers/libbcrypt/bcrypt.c
#   composers/libbcrypt/bcrypt.h
#   composers/libbcrypt/crypt_blowfish/*.c *.h *.S
```

O `CMakeLists.txt` da raiz já:
- compila esses `.c` como uma lib estática `bcrypt`;
- adiciona `composers/` ao include path, de modo que
  `#include <bcrypt/bcrypt.h>` resolva. **Para isso funcionar**, garanta que
  o header fique acessível como `bcrypt/bcrypt.h`. Duas opções:
  1. renomeie a pasta clonada para que exista `composers/bcrypt/bcrypt.h`
     **e** mantenha os fontes em `composers/bcrypt/` (ajuste o `BCRYPT_DIR`);
  2. ou crie um symlink/copie o header:
     ```bash
     mkdir -p composers/bcrypt
     cp composers/libbcrypt/bcrypt.h composers/bcrypt/bcrypt.h
     ```
  O `CMakeLists.txt` aponta `BCRYPT_DIR = composers/libbcrypt`; se você
  preferir manter tudo em `composers/bcrypt/`, ajuste `BCRYPT_DIR` lá.

### Alternativa (Argon2)

Se preferir Argon2 (também excelente), troque só o `PasswordHasher.hpp`
para usar `libargon2` — o resto do sistema depende apenas de
`PasswordHasher::hash()` / `verify()` (é um ponto de troca, padrão Strategy).

## OpenSSL, JsonCpp, Drogon, MySQL

Essas vêm do sistema/gerenciador (apt, vcpkg, brew) — ver o README da raiz.
Não precisam ser vendorizadas aqui.
