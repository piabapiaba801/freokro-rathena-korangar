# FreaokRO V14.4 - @Funk Emergency Feature Control

Administrative emergency control layer. `@Funk` opens a remote NPC dialog (no physical NPC) and is restricted to group level 99+.

Every custom system tracked since the project baseline now has a named ON/OFF switch in the central panel/config. New FreaokRO features must add a FUNK switch as part of their implementation checklist.

Important: this is a server-side kill switch. Client assets/icons/HUD may remain visible when a feature is OFF, but authoritative server behavior must refuse/stop the disabled feature. Existing persistent data is never deleted by OFF.

`funk_master_enable` is the global emergency master switch. Individual flags are in `conf/import/battle_conf.txt`. Runtime changes made through @Funk last until map-server restart; startup defaults come from battle_conf.

Catalog EXP remains separately switchable; turning it OFF does not disable identification/catalog.

## Atualizacao V14.6 - controles individuais das skills

O painel agora separa os controles de Coleta em Area, Olhar do Cacador,
Oficio Universal, Retorno ao Ponto Salvo, Domar Criatura, Ecommerce e Asa de
Leaf. O controle Basic Skills continua sendo o interruptor do grupo inteiro.

O leilao usa simultaneamente MASTER, Basic Skills e Auction. Ecommerce usa
MASTER, Basic Skills e seu controle proprio. Olhar do Cacador usa MASTER,
Basic Skills, Monster Catalog e seu controle proprio. Catalog EXP continua
controlando somente o pagamento de EXP.

Esta atualizacao esta aplicada no source e na configuracao inicial. Exige uma
futura compilacao/substituicao do map-server para entrar em producao.
