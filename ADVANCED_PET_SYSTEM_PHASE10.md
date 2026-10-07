# Advanced Pet System — Fase 10 — KO de 5 minutos

- O tempo de indisponibilidade após KO foi aumentado de 30 segundos para 5 minutos (300.000 ms).
- Ao chegar a 0 HP, o pet combatente continua entrando em KO, desaparece, cancela alvo/cast e retorna ao dono após o temporizador com HP restaurado.
- O KO continua temporário em runtime: não destrói ovo, não apaga pet e não zera intimidade.
- O requisito atual para o pet começar a auxiliar no combate permanece `pet_support_min_friendly: 900`, numa escala de intimidade de 0 a 1000.
- Em 900 de intimidade, o `rate_fix` de suporte começa em 50% e cresce até 150% em 1000, conforme a fórmula atual do core.
