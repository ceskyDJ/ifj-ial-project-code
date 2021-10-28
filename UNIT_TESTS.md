# Jak používat unit testy

Jelikož se jedná o náročnější problematiku, je v následujících odstavcích rozvedena
podrobněji. Stručný návod ke psaní testů a používání frameworku
[Unity](https://github.com/ThrowTheSwitch/Unity) je uvedeno níže.

## Co je to unit test?

Existuje mnoho způsobů, jak testovat kód, aby se nalezlo co nejvíce chyb dříve, než se
projekt dostane do produkční fáze (v našem případě než dojde k odevzdání). Jedním
z nejpoužívanějších druhů jsou právě unit testy (česky asi jednotkové testy). Jednotkami
konkrétně myslíme jednotlivé moduly, z nichž se následně poskládá celý program.

Jednotlivé moduly testujeme proto, že je to jednodušší, než testovat celek. Menší kusy
kódu obvykle obsahují jen minimum funkcí, které je nutné otestovat. Pokud si dáme
záležet, mělo by být pouze minimální riziko, že zůstane nějaká část kódu úplně nepokrytá
testy. Termínem "nepokrytá" myslíme stav, kdy se při žádném testu neskočí na daný řádek,
blok či do nějaké funkce.

Unit testy je možné psát ještě před implementací modulu coby návrh modulu. To v praxi
znamená, že místo toho, abychom hned začali psát kód funkcí, místo toho si nejprve
sedneme a sepíšeme si, jaké funkce budeme chtít využít a co od nich budeme vyžadovat.
Tyto informace mimochodem navíc tvoří rozhraní modulu. V případě, když k vývoji
přistupujeme tímto způsobem, náš postup se nazývá "test driven development", což je
česky prostě "vývoj řízený testy". Samotný kód tedy píšeme až ve chvíli, kdy máme
připravené rozhraní a definované testy, které budou ověřovat jeho správnost.

My jsme se však rozhodli přistoupit na opačný způsob, tedy upřednostnění implementace
před psaním testů. Nejprve si tedy rozmyslíme rozhaní modulu, poté modul implementujeme
a následně podle dříve definovaného rozhraní dopíšeme testy, které prověří, zda bylo
implementováno správně. Dále bude popisována výhradně tato metoda.


## Jak unit testy fungují?

Když píšeme unit testy, zaměřujeme se na to, abychom pečlivě otestovali každou funkci
modulu, pro něhož testy vytváříme. V ideálním případě by měly být pokryty okrajové
stavy a rovněměrně také běžné vstupy. Toho docílíme tak, že si otevřeme definici
rozhraní daného modulu (včetně neformálních verzí v úkolech na Githubu) a podle ní
vytvoříme testy. Následně se pokusíme nalézt možné rizikové vstupy (hraniční, neobvyklé
či jinak problematické). Případně můžeme také omezit přidělovanou paměť, abychom
otestovali správnou práci s dynamicky alokovanou pamětí.

Princip unit testů je poté vcelku prostý - voláme funkce z testovaného modulu
s využitím explicitních parametrů, pro jejichž kombinaci přesně známe správný
výstup (popř. správné chování) testované funkce. Pokud funkce vrátí požadovaný
výsledek či zařídí očekávaný vedlejší efekt, test je úspěšný. V případě odlišného
výsledku nebo vedlejšího efektu (případně nulového vedlejšího efektu, pokud je očekáván)
se test stává neúspěšným a značí, že je v implementaci funkce nějaká chyba.


## Unity Test

[Unity](https://github.com/ThrowTheSwitch/Unity) je jednoduchý minimalistický framework
pro psaní unit testů pro programy vyvíjené v jazyce C. Testování značně ulehčuje pomocí
automatického generování přípravného a ukončujícího kódu pro testy, generování výstupů
testů, vytváření podpůrných funkcí (zejména funkce `main()`), poskytování pomocných
maker apod.

Více informací o frameworku je možné zjistit v [Github 
repozitáři](https://github.com/ThrowTheSwitch/Unity) projektu nebo na [webových 
stránkách](http://www.throwtheswitch.org/unity) vývojáře.

Framework byl do tohoto projektu zabudován, aby bylo jeho použití jednodušší a plně
automatizované (mimo pohodlí programátora také kvůli automatickému spouštění pomocí
Github Actions). Pro automatické generování a spouštění testů se stará nástroj
[GNU Make](https://www.gnu.org/software/make/).

## Psaní testů

Psaní unit testů bylo maximálně zjednodušeno, takže stačí dodržet následující postup.
Pozor je třeba dávat na správné pojmenování souboru s testy a v něm obsažených
testovacích funkcí. Make je case-sensitive, takže v případě chybného názvu souboru
nedokáže testy správně přeložit. Pokud k takové situaci dojde, doporučuji nejprve
zkontrolovat pojmenování souboru s testy.

### Soubor s testy

Testy se píší do speciálního souboru psaného v jazyce C. Tento soubor musí přesně
dodržet následující formát názvu: `test_module_name.c`, kde `module_name.c` je přesný
název souboru, v němž je obsažena implementace testovaného modulu.

Tento soubor musí být obsažen v adresáři `$PROJECT/test/unit/`.

```c
// test_module_name.c

// Always required include of Unity test framework
#include "../../unity/src/unity.h"
// You need to include all header file of tested module, too
#include "../../src/module_name.h"

// TODO: tests goes here
```

### Testovací funkce

Do souboru s testy (viz předchozí podsekci) se následně umisťují testovací funkce.
Jedná se o běžné funkce z jazyka C, které neberou žádné parametry a zároveň nic nevrací.
Jejich název může být jakýkoliv, ale musí mít následující prefix (case sensitive):
`test_`.

```c
/**
 * Individual tests of add function
 */
void test_add_function(void) {
    // my_tested_function() must return 0 for 42 as the first positioned parameter
    TEST_ASSERT_EQUAL(0, my_tested_function(42));
    // my_tested_function() must return ABCh for 11 as the first positioned parameter
    TEST_ASSERT_EQUAL_HEX(0xABC, my_tested_function(11));
}

/**
 * Tests for removing functionality
 */
void test_remove_function(void) {
    // Or you can prepare test for future implementing (will be displayed as ignored)
    // TODO: Implement this tests
    TEST_IGNORE();
}
```

Kromě speciálních maker (popsaných v následující podsekci) je možné mít ve funkci
naprosto cokoliv. Stále se jedná o plnohodnotný jazyk C, takže funguje naprosto vše,
co lze udělat i kdekoliv jinde. Jen místo navracení z funkce nebo psaní běžných
`assert`ů se využívaní ona speicální makra poskytovaná frameworkem.

Také je možné psát si doplňující funkce (pro různé inicializace apod.), protože
i testy by měly držet DRY (Don't Repeat Yourself) a KISS (Keep It Simple, Stupid),
aby se v tom následně někdo, kdo v modulu bude později dělat změny, neztratil. Tyto
doplňkové funkce naopak **nesmějí** začínat prefixem `test_`.

### Speciální makra

Framework nám poskytuje sadu užitečných maker, abychom si nemuseli sami zajišťovat
porovnávání polí, hexadecimálních hodnot apod. Kompletní seznam maker je k dispozici
v [dokumentaci frameworku](https://github.com/ThrowTheSwitch/Unity/blob/master/docs/UnityAssertionsReference.md#the-assertions-in-all-their-blessed-glory).

Základní makra pro vyhodnocení testů jsou tato:
- `TEST_ASSERT(condition)`
  - Otestuje, zda je splněna podmínka `condition` a podle toho vyhodnotí úspěšnost testu.
- `TEST_ASSERT_FALSE(condition)`
  - Otestuje, zda **nebyla** splněna podmínka `condition` (opak `TEST_ASSERT()`).
- `TEST_ASSERT_NULL(pointer)`
  - Otestuje, zda pointer ukazuje na `NULL`.
- `TEST_ASSERT_NOT_NULL(pointer)`
  - Otestuje, zda pointer **neukazuje** na `NULL` (zda je validní).
- `TEST_ASSERT_EQUAL_INT(expected, actual)`
  - Otestuje, že platí `expected == actual`, kde `expected` je očekávaná (správná)
    hodnota a `actual` je hodnota např. navrácená z testované funkce. Obě hodnoty musí
    být typu `int`.
- `TEST_ASSERT_EQUAL_UINT(expected, actual)`
  - Obdoba `TEST_ASSERT_EQUAL_INT()` pro hodnoty typu `unsigned int`.
- `TEST_ASSERT_EQUAL_HEX (expected, actual)`
  - Hexadecimální porovnání dvou čísel, jinak viz `TEST_ASSERT_EQUAL_INT()`.
- `TEST_ASSERT_EQUAL_CHAR (expected, actual)`
  - Otestuje shodnost očekávaného znaku `expected` a testovaného znaku `actual`.
- `TEST_ASSERT_EQUAL_PTR (expected, actual)`
  - Otestuje shodnost referenčního ukazatele `expected` s testovaným `actual`. Porovnává
    adresy, na které ukazatelé míří.
- `TEST_ASSERT_EQUAL_STRING (expected, actual)`
  - Otestuje shodnost textových řetězců `expected` (referenční) a `actual` (testovaný).
- `TEST_ASSERT_EQUAL_INT_ARRAY (expected, actual, num_elements)`
  - Otestuje shodnost prvků polí `expected` (referenční pole) a `actual` (testované),
    `num_elements` určuje počet prvků od začátku pole (`expected[0]`), které budou
    porovnány.
- `TEST_ASSERT_EACH_EQUAL_INT (expected, actual, num_elements)`
  - Otestuje, zda je prvních `num_elements` prvků z pole `actual` rovná hodnotě
    `expected` typu `int`.

Makra jsou poměrně univerzálně navržená, většinou testy stačí nahradit `INT` za `CHAR`
a místo čísel porovnáváme znaky. Totéž s různými velikostmi celých čísel `INT8`, `INT16`
a jejich bezznaménkovými variantami (`UINT`, `UINT8` apod.). Navíc je možné použít i
`STRING` pro práci s řetězcem a `PTR` pro práci s ukazateli.

V případě, že neexistuje žádné makro, které by pokrylo požadované ověření správnosti,
je možné použít následující makra a ověření si doprogramovat ručně:
- `TEST_FAIL()`
  - Nastaví test jako neúspěšný
- `TEST_FAIL_MESSAGE(message)`
  - Viz `TEST_FAIL()`, navíc vypíše zadanou zprávu
- `TEST_PASS()`
  - Nastaví test jako úspěšný
- `TEST_PASS_MESSAGE(message)`
  - Viz `TEST_PASS()`, navíc vypíše zadanou zprávu

### Inicializační a destrukční funkce

Pokud je třeba před (případně za) každým testem daného modulu provést nějakou akci,
není nutné dělat to ručně. Takový postup by pravděpodobně vedl k zanesení chyb
do testovacích funkcí, což určitě nechceme. Lepší řešení je využít speciální funkce
`setUp()` a `tearDown()`, které jsou na řešení toho problému navržené. Funkce `setUp()`
je zavolána před voláním každé `test_` funkce a `tearDown()` bezprostředně po opuštění
každé testovací funkce.

```c
/**
 * Runs before each test function
 */
void setUp(void)
{
    // TODO: Add something to run before each test function
}

/**
 * Runs after each test function
 */
void tearDown(void)
{
    // TODO: Add something to run after completion of each test function
}
```

Tyto funkce jsou zcela nepovinné a běžně je není třeba uvádět. Vzhledem k zachování
vyšší přehlednost je vhodné uvádět je pouze tehdy, jsou-li potřeba. Pro doplnění raději
uvedu, že se dávají do stejného souboru jako testovací funkce (viz podsekci Testovací
funkce).
