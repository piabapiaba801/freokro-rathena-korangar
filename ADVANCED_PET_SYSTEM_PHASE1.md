# FreokRO — Advanced Pet System — Fase 1

## Objetivo
Ativar de forma conservadora os perfis de combate e suporte de pets que ja acompanham a arvore rAthena/FreokRO, sem inventar status para entradas sem perfil conhecido.

## Alteracoes
- `db/import/pet_db.yml`: Body ativado com os perfis existentes de AttackRate, RetaliateRate, ChangeTargetRate e SupportScript.
- `conf/battle/pet.conf`: pet_attack_support, pet_damage_support e pet_status_support habilitados.
- Intimidade minima de suporte mantida em 900/1000.
- `db/re/pet_db.yml` nao foi substituido: captura, ovos, comida, bonus e evolucoes Renewal permanecem como estavam.

## Comportamento
Pets com perfil de suporte passam a poder atacar junto do dono, retaliar e executar suas rotinas individuais quando a intimidade permitir. Os perfis incluem, conforme cada pet, ataque, cura, recuperacao de status, bonus temporarios e petloot.

## Limite desta fase
Entradas que nao possuem perfil no import nao receberam numeros inventados. A expansao de novos pets depende da auditoria do client 2025 (`petInfo.lub`, `PetEvolution.lub` e assets).
