# Advanced Pet System - Fase 11 - Intimidade 750 + AutoFeed VIP

- Escala de intimidade: 0; Awkward 1-74; Shy 75-187; Neutral 188-449; Cordial 450-674; Loyal 675-750.
- Intimidade maxima: 750. Pets existentes acima do teto sao limitados a 750 quando carregados.
- Intimidade inicial: 150 para todos os pets (default e entradas explicitas).
- Ajuda em combate: inicia exatamente no primeiro ponto Cordial, 450.
- Curva de suporte foi reescalada para o novo teto: 50% no ponto 450 ate 150% em 750.
- AutoFeed: exclusivo para contas Group Level 1+. Group Level 0 nao pode ativar via @autofeed nem pela configuracao do cliente, e o timer nao alimenta contas 0.
- AutoFeed perfeito: espera fome <= 75 (Neutral), ponto em que pet_food concede o IntimacyFed integral; evita a faixa 76-90, que concede apenas metade, e evita overfeed >90.
- @autofeed now permanece VIP e alimenta imediatamente por solicitacao explicita do VIP; o modo automatico e que espera o ponto otimo.
