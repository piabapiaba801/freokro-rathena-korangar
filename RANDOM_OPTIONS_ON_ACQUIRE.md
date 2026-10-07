# Random Options on Acquire

- Random options naturais no momento em que o monstro cria o drop no chao: DESABILITADAS.
- Novo ponto autoritativo: entrada de item novo no inventario do jogador (`pc_additem`).
- Itens que ja possuem random options nao sao sobrescritos.
- Trade, storage, guild storage, mail, vending e outros fluxos de transferencia nao rerrolam o item.
- Categorias configuraveis em `conf/import/random_option_on_acquire.conf`.
- O link fornecido (Enhanced MVP Tomb) nao especifica quais grupos/chances usar por categoria; por seguranca, os grupos ficam `None` ate a tabela de balanceamento ser definida.
