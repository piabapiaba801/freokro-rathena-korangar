# PENDÊNCIA CLIENT-SIDE — FAZER NO WINDOWS

Nenhum arquivo do cliente foi alterado nesta etapa.

1. Registrar IDs 9000–9007 no cliente 2025-07-16 usado pelo projeto (nomes, descrições e ícones/skill info conforme o formato do cliente).
2. Expor as skills novas na árvore/HUD do cliente e validar seleção/alvo/efeitos visuais.
3. Skill 2: criar apresentação visual dedicada caso a mensagem de chat server-side não seja a UI final desejada.
4. Skill 3: validar a janela de crafting do packetver escolhido; HUD custom dedicado, se desejado, é client-side.
5. Skill 7: cadastrar/validar sprites, eggs, acessórios e recursos visuais dos pets adicionais.
6. Skill 8: o mercado remoto server-side funciona por diálogo/menu. A UI própria, a segunda função ativa e o sistema de leilão permanecem pendentes porque a especificação final dessas duas partes não foi definida.
7. Validar GRF/Lua/Lub/System/skillinfo e packetver no Windows antes de distribuir o cliente.

## Voice Chat — standby para Windows / cliente 2025-07-16
8. Compilar `voice-chat.dll` em Visual Studio 2022, `Release | Win32`.
9. O upstream atualmente documenta offsets específicos para **Client 2025-07-16**: `ACCOUNT_ID=0x011FB9A4`, `CHAR_ID=0x011FB9A8`, `LOGIN_ID1=0x011FB244`. Confirmar esses offsets no nosso executável exato antes da distribuição final.
10. Configurar IP/porta do Voice Server no cliente (`voice_port` padrão 7000) e definir o mesmo `voice_client_secret` usado no servidor.
11. Integrar/carregar `voice-chat.dll` no Ragexe 2025-07-16 (VoiceLinker ou método equivalente) e validar hook Direct3D 9/overlay ImGui.
12. Testar microfone/saída, PTT, Open Mic, proximidade, Party, Guild, Room, Whisper/Direct Call, mute/volume individual e comportamento em WoE/GvG.
13. Validar autenticação estrita `LOGIN_ID1` contra o `auth_advisory` enviado pelo map-server.

## Anti-Cheat / Guard — standby Windows/client-side
- Criar Guard DLL x86 especificamente para `2025-07-16_Ragexe_175220998_clientinfo.exe`.
- Validar integridade do Ragexe, módulos/DLLs críticas e arquivos/GRF definidos pelo projeto.
- Implementar proteção user-mode contra alterações/injeção/debugging somente no escopo necessário ao processo do jogo; sem driver kernel e sem varredura de documentos/navegador/microfone.
- Criar handshake de sessão `Client Guard <-> Guard Service <-> rAthena`, com nonce/token de curta duração e heartbeat; o servidor nunca deve confiar apenas na presença da DLL.
- Vincular a sessão à autenticação AID/CID/login e invalidá-la em logout/reconexão.
- Compilar e assinar nossos próprios binários a partir do source auditado; não depender de executável fechado de terceiros.
- Integrar com o patcher futuro para manifestos/hashes assinados e atualização segura do Guard/cliente.
- Fazer rollout primeiro em modo telemetria; calibrar falsos positivos antes de qualquer punição automática.

## MVP Tomb Extended — standby client-side / Windows
- Validar visualmente a janela 420x620 no executável exato `2025-07-16_Ragexe_175220998_clientinfo.exe`.
- Validar tags `<ITEM>/<INFO>` e popup nativo de item para itens oficiais e customizados.
- Para itens customizados, sincronizar nome/descrição/ícone/sprite e dados Lua/Lub/GRF do cliente.
- Definir/instalar sprite visual customizado do túmulo (se desejado); o server-side funciona com o sprite padrão sem isso.
- Ajustar tipografia/cores/layout somente após teste real no Ragexe para evitar depender de markup não suportado.

## Random Options on Acquire
- Validar exibicao das random options no Ragexe 2025-07-16 apos definirmos os grupos/categorias finais.
- Nenhuma logica de sorteio depende do cliente; a autoridade permanece server-side.

## Random Options V2
- Nenhuma mudança client-side obrigatória para a distribuição 0..5 em itens oficiais.
- Validar visualização/tooltip das 1–5 opções no Ragexe 2025-07-16.
- Itens customizados continuam exigindo dados client-side correspondentes para nome/descrição/ícone/sprite quando aplicável.

## Preserve Toggle
- Nenhuma dependencia client-side obrigatoria identificada. Validar apenas feedback visual/status icon no Ragexe 2025-07-16 durante teste Windows.

## GearProtect
- Nenhuma alteracao client-side obrigatoria. Validar apenas UX/mensagem apresentada pelo Ragexe 2025-07-16 quando uma venda protegida for recusada.

## Skill/Magic Critical - validacao visual
- Validar no Ragexe 2025-07-16 se ZC_NOTIFY_SKILL com action DMG_CRITICAL (10) e DMG_MULTI_HIT_CRITICAL (13) produz os baloes/numeros vermelhos tradicionais em TODAS as skills, inclusive magia, AoE, ground skills e multi-hit.
- Alguns caminhos legados de skill sobrescrevem o tipo visual no servidor; mapear os casos no teste Windows e ajustar somente a apresentacao, sem alterar a regra server-side de critico.

## Auditoria Windows 2026-09-24
- O ZIP V19 recebido contem somente o projeto server-side.
- Nao havia Ragexe, GRF, data folder, petInfo, PetEvolution, itemInfo,
  skillinfo, Voice DLL, Guard DLL ou source Guard no ZIP recebido.
- Tambem nao havia cliente Ragexe ou GRF nas pastas Downloads, Desktop e
  Documents do Windows auditado.
- Foi baixado o snapshot ROenglishRE de 2025-07-11, commit
  71528a28d8946e4ab8473d694df50d2b1dbbae19.
- Foram preparados em CLIENT_SIDE_PREPARED: itemInfo completo e dependencias,
  petInfo.lub, PetEvolution.lub, skillinfoz e demais dados Renewal do snapshot.
- Foi baixado o source do voice-chat-client com solucao Release Win32 e
  offsets documentados para 2025-07-16.
- Nenhum arquivo foi aplicado em cliente/GRF porque o cliente legitimo esta
  ausente. Nenhum teste in-game ou visual foi declarado concluido.
- Skills CUSTOM 9000-9008 nao foram cadastradas no cliente porque permanecem
  desabilitadas na arvore Novice e no factory da FreokRO V19.
- Guard DLL nao foi inventada: source e especificacao final continuam ausentes.
