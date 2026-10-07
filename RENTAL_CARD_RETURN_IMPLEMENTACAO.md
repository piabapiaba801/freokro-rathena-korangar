# Rental Card Return - implementação custom V8

Base: rAthena custom Skill Critical V7.

## Comportamento
Quando um equipamento rental expira no inventário, antes de o equipamento ser removido o servidor percorre seus slots de carta. Cada slot que contém um item real do tipo IT_CARD é devolvido ao mesmo personagem via RodEx/mail.

- O equipamento rental continua expirando e sendo destruído normalmente.
- Uma mensagem é enviada por carta, sem depender de espaço livre no inventário.
- Slots especiais de forged/created/pet (`CARD0_*`) são ignorados.
- IDs que não correspondam a `IT_CARD` são ignorados defensivamente.
- O retorno acontece antes de `pc_delitem`, preservando os dados necessários.
- Falha ao enfileirar o mail gera `ShowWarning` no servidor.
- Não requer alteração client-side.

## Validação
`src/map/pc.cpp` compilado isoladamente com sucesso (`obj/pc.o`).
