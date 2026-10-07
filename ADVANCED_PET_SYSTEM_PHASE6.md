# Advanced Pet System — Fase 6: Tags de função

Classificação visual aplicada: `[off]`, `[def]`, `[uti]`.

- 48 pets ofensivos (`CustomCategory: 1`)
- 15 pets defensivos (`CustomCategory: 2`)
- 44 pets utilidade (`CustomCategory: 3`)
- Pet ativo: tag adicionada dinamicamente nos pacotes de nome/status, sem alterar o nome persistido.
- Ovos: nomes server-side prefixados nos bancos Renewal e Pre-Renewal.
- Famílias e evoluções mantêm a mesma categoria.

## Cliente 20250716
O inventário do Ragexe normalmente usa o nome de item do `itemInfo` client-side. Portanto, os ovos já estão classificados no servidor, mas a exibição da tag no inventário exige sincronizar `itemInfo.lub`/`itemInfo_true.lub` quando os arquivos do cliente forem disponibilizados.
