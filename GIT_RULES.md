## Commit
Jak by měl vypadat:
- malý počet souvisejících změn
- nové věci brát jako celek (nový modul -> jeden commit) ALE ne mnoho souborů každý o hodně řádcích, myslet na toho kdo bude dělat review
- velká změna nebo více změn -> rozbít/rozdělit do více jednoduchých commitů
- odstranění funkce, upravení funkce -> commit pro každý takový samostatný zásah
- pokud je v názvu commitu "něco.. **a** něco jiného" -> rozdělit na dva commity

## Commit zpráva
Je jako email -> má předmět a tělo. Předmět by měl být jasný, stručný, výstižný. Vhodné psát anglicky v přítomném čase rozkazovacího způsobu. Např.:
- improve device lookup algorithm
- fix memory leak in gateway service
- remove obsolete code
- describe new feature in doc
- fix typo – *oprava překlepu např. v komentáři*
- change way the system responds to invalid requests

Pokud se commit týká určitého modulu, jeho název se uvede na začátek předmětu:
```
port_control: move configuring of RSS from generic_port_configurer
```

Pokud commit souvisí s určitým issue (úkolem) v Githubu, přidat na konec předmětu **každého** takového commitu `#n`, kde `n` je číslo daného úkolu:
```
vector_proc: implement optional Bohrian vector filtering #22
```
Číslo úkolu lze vidět za názvem úkolu, když si ho rozkliknu, pod názvem v přehledu otevřených úkolů nebo také v URL samotného úkolu.

Tělo commitu není povinné, ale pokud commit přidává/upravuje něco netriviálního, je lepší změny odůvodnit a **uvést do kontextu**. Raději popisovat záměr, důvod pro změny, než obsah změn (ten vidím v kódu). Např.:

```
This patch fixes vulnerability CVE-2017-0002. See, that we are fixing
only single path through the code. If we fix both paths we would introduce
a double free error.
```

```
This commit is preparation for future modification where local
data and worker statistics are not part of worker context but
they are moved to runtime of mitigation rule.
```

Pokud by tělo commitu mělo být hodně dlouhé, pak commit přidává/mění něco velmi složitého -> určitě by to šlo **rozbít na menší, lépe stravitelné změny**, které by šly popsat lépe.

Vzorový commit by pak mohl vypadat takto:
```
ansible: determine nosync_hours automatically

The default nosync_hours value is overriden by the inventary value.

However, variables defined inside of a playbook have higher priority
than variables from inventories. Thus, we have to drop the 10 hours
constant.
```
nebo takto
```
appfs_aggregator: collect all values per label

The label_value_info now keeps all parsed particular values and not just
their aggregations.
```

## Vývojové větve
Domluva je zhruba taková, že `main` bude větev, ze které se bude vycházet (vytvořím novou větev na základě `main`) a zase do ní navracet (merge oné větve zase do `main`). Tedy žádné `developement` apod. větve, protože náš `main` může být developement i "release" větev zároveň. Příklad:
```
main--------------------------------------------------------- ...
         \            \      /    \           /           /
          new_feature--------      small_fix--           /
                        \                               /
                         another_branch-----------------
```
Na cizí větve se nesahá, teda pokud se nedomluvíme, že někdo něco přebere nebo poupraví, ale jinak *moje větev, moje teritorium*.

## Pull request (zkratka PR)

Až budu mít něco, co chci prezentovat ostatním nebo i jen něco, co stojí za diskusi (či ji přímo vyžaduje), tak vytvořím pull request do `main`.
Git sám při prvním push nové větve nabídne, jak nastavit remote a poté i vypíše link na vytvoření pull requestu.

Pokud mám něco značně nedokončeného či mám jen zárodek a chci dát najevo, že se jedná o první nástřel/návrh, tak místo klasického PR vytvořím [draft pull request](https://github.blog/2019-02-14-introducing-draft-pull-requests/). Ten pak lze zplnohodnotnit na klasický PR.

Na `main` se tedy nesahá jinak než přes pull requesty (jakmile se něco schválí, tak se udělá merge do `main` a je to).

Do samotného pull requestu se napíše vhodný titulek a popis změn (toto jako snad jediné budeme psát česky, je to interně, pro nás, pro účely diskuse a review).

Při vytváření pull requestu se zaklikne `Enable auto-merge` a vybere se druh `merge-commit`, což vyvolá výzvu pro vyplnění titulku a těla merge commitu. Předvyplněný obsah je potřeba srozumitelně **přepsat**, aby stylem odpovídal normálnímu commitu.

Pro svázání pull requestu s úkolem se do popisu PR uvede odkaz na úkol:
```
Související úkol:
- issue #n
```
Místo `issue` lze uvést i `task`.

Pokud se má úkol po úspěšném začlenění pull requestu uzavřít, mohou být použity i [direktivy Githubu](https://docs.github.com/en/issues/tracking-your-work-with-issues/linking-a-pull-request-to-an-issue#linking-a-pull-request-to-an-issue-using-a-keyword) (`close`, `fix`, `resolve`...).

Konverzace u pull requestů uzavírá jejich autor! :P

### Interaktivní rebase (*Základní znalost Vimu nutností!*)

Pokud píšu něco nového ve své větvi, tak si tam můžu dělat co chci. Jakmile se to ale přiblíží stavu, kdy bych chtěl od ostatních review a po kontrole merge, musím to trochu uklidit -> k tomu slouží interaktivní rebase.

Dokáže přeskládat, upravit, odstranit, sloučit, rozdělit (to je náročnější, ale jde to udělat) commity v aktuální větvi. Dobré na úklid či zapracování změn od ostatních. Nebudu tady jen opisovat, takže viz kapitola 7 v `git_tutorial.pdf` v `#git` nebo cokoliv na internetu, popř. můžu dát osobní tutoriál.

- `git rebase -i origin/main`

Otevře se Vim, kde budou na jednotlivých řádcích commity a stručný, ale víceméně jasný popis, jaké příkazy lze před commity napsat a co provedou plus užitečné info okolo (**doporučuji přečíst**).

Příklad základních operací, které se hodí umět:
```
pick 10dff03 introduce GIT_RULES.md #2
pick 4818005 update GIT_RULES.md
pick 0258432 clarify interactive rebase usage
```
Řekněme, že chceme u druhého commitu změnit název a třetí commit sloučit s druhým. Provedeme to následovně:
1. upravíme slova (lze používat i jednopísmenné zkratky) před danými commity na
```
pick 10dff03 introduce GIT_RULES.md #2
reword 4818005 update GIT_RULES.md
fixup 0258432 clarify interactive rebase usage
```
2. uložíme, zavřeme Vim
3. vyskočí nám nový Vim, tentokrát s názvem commitu, který jsme označili pro úpravu (plus informace)
4. upravíme název commitu
5. uložíme, zavřeme Vim

Pokud vše proběhlo v pořádku, Git by měl vypsat `Successfully rebased and updated...`

Problém při rebase a nevíš, jestli jsi něco nepodělal?
`git rebase --abort` a zkus to znovu.

**POZOR**

Rebasováním (především slučováním commitů se změnami) se ztrácí historie. Přípustné je to jen tehdy, kdy žádná z verzí by nemohla být v budoucnu potřebná. Tedy u tohoto souboru nás historie nezajímá, ale u kódu chceme vidět historii všech podstatných změn (opravy překlepů či code style podstatné nejsou).
