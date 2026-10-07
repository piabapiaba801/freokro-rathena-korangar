# FreokRO Advanced Pet System - Fase 9: Combat HP, Cooldowns e KO

## Escopo
Esta fase transforma pets [off] e [def] em unidades de combate com ciclo de vida temporario. Pets [uti] permanecem fora do sistema de alvo/KO de combate.

## HP por funcao
- [off]: usa o HP base do monstro/pet, preservando perfil de dano com menor resistencia relativa.
- [def]: recebe 175% do HP base ao ser inicializado.
- [uti]: nao entra como alvo de combate por esta fase.

## Cooldowns
- Skills ofensivas de [off]: cooldown explicito minimo de 5 segundos apos uso bem-sucedido.
- Skills ofensivas de [def]: cooldown explicito minimo de 8 segundos apos uso bem-sucedido.
- Skills de suporte continuam usando o campo Delay individual ja existente em petskillsupport; portanto mantem cooldown proprio configuravel por pet/skill.
- O cooldown e independente da chance de proc: tentativa que nao dispara a skill nao consome cooldown.

## KO / desaparecimento temporario
- Quando HP de [off]/[def] chega a zero, o pet NAO e destruido e o ovo NAO e perdido.
- O pet entra em KO, cancela alvo/cast e e removido visualmente do mapa.
- Respawn automatico apos 5 minutos junto ao dono.
- Retorna com 100% HP.
- Durante KO, skills de suporte/heal ficam suspensas e verificam novamente depois.
- O estado KO e runtime e deliberadamente nao persistente: logout/rehatch/restart nao grava uma 'morte' permanente no pet.

## Targeting
- [off] e [def] passam a ser alvos validos no pipeline de batalha.
- [uti] continua invalido como alvo de combate, evitando que pets de coleta/EXP/drop sejam mortos por exercer utilidade.

## Validacao
- pet.cpp, status.cpp e battle.cpp compilados individualmente.
- Build completo do map-server foi iniciado; nao foi concluido dentro da janela de execucao, portanto link final nao e declarado como confirmado.
