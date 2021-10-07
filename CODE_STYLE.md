# Příručka pro psaní kvalitního kódu

Příručka je velmi stručná a slouží pouze jako "záchrana v případě nouze", kdy si člověk
není jistý, jestli je to, co se chystá napsat "prasárna" nebo ne.

Základní pravidla jsou převzata z https://www.pethas.cz/spravne-psat-zdrojovy-kod/

## Základní pravidla pro kód

Kód by se měl držet následujících pravidel (minimálně):

- Odsazení řádků musí odpovídat vnořeným blokům,
- jeden řádek – jeden příkaz (značka),
- logické celky odděleny mezerou,
- dodržujte jednotné názvy objektů,
- identifikátory by měly co nejpřesněji popisovat význam,
- dodržovat formát pro konstrukce,
- každá funkčnost naprogramovaná pouze jednou,
- kód nesmí obsahovat magická čísla.

## Základní pravidla pro komentáře

Komentáře by měly následovat následující body:

- Komentáře nesmí duplikovat kód,
- komentář musí jednoznačně osvětlit popisovanou část kódu,
- komentuje se hlavička každého souboru, se kterým se pracuje,
- komentuje se hlavička všech objetků, funkcí a konstrukcí procedur,
- komentují se všechny netriviální konstrukce.

## Příklady

### Obecná praktická pravidla

- Odsazuje se vždy mezerami, a to přesně 4,
- vše, co je ve zdrojových souborech je anglicky,
- maximální délka řádku je 120 znaků.

### Hlavička souboru

Každý soubor musí mít hlavičku ve formátu uvedném níže (podle typu souboru),
autoři jsou uváděni podle toho, kdo na daném souboru skutečně pracoval.
Aby se na hlavičky nezapomínalo (je to bodově penalizováno...), je jejich
přítomnost automaticky testována. Kvůli tomu je třeba dodržet strukturu
uvedenou níže, v opačném případě test neprojde.

**Zdrojové a hlavičkové soubory jazyka C (`*.c` a `*.h`)**
```c
/**
 * @file
 * File description (short information about what is inside)
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 * 
 * @author Michal Šmahel (xsmahe01)
 */
```

**Makefile**
```makefile
# File description (short information about what is inside)
#
# IFJ and IAL project (IFJ21 compiler) 
# Team: 128 (variant II)
#
# Author: Michal Šmahel (xsmahe01)
```

### Dokumentační komentáře

Všechny funkce musí mít dokumentační komentář, který lze naparsovat pomocí
[Doxygenu](https://www.doxygen.nl/index.html). Doxygen podporuje několik formátů,
nicméně my budeme používat následující:
```c
/**
 * What function does (short)
 * 
 * Detailed description of function behaviour, specialities etc.
 * 
 * @param first_param Description of the first_param parameter
 * @param second_param Description of the second_param parameter
 * @return What does the function returns?
 */
bool function_name(int first_param, char second_param)
{
    // Implementation goes here...
}
```

### Základní konstrukce

Formát základních konstrukcí je pevně daný a měl by být používán přesně jako
na následujících příkladech (pokud to nebude z nějakého důvodu možné, vítězí
selský rozum).

#### Identifikátory

Pro názvy proměnných, funkcí a typů se používá tzv. snake_case. Slova jsou oddělena
podtržítkem (`_`) a jsou psána výhradně malými písmeny. Konstanty a makra jsou psány
verzálkami a jednotlivá slova jsou oddělena rovněž podtržítky.

Názvy struktur (pokud není vhodné nechat strukturu anonymní) neobsahují žádný prefix
nebo suffix. Ten je přítomen až u typů (názvy struktur jsou jasně oddělené). Nově
definované typy mají příponu (suffix) `_t`.

Hvězdička u ukazatelů se uvádí bezprostředně před názvem identifikátoru. Lépe se pak
odlišuje, zda jde o ukazatel či nikoliv, v případech hromadné deklarace.

Pokud se něco vztahuje k nějakého modulu nebo abstraktní datové struktuře, je třeba
jako předponu (prefix) použít název modulu, případně abstraktní datové struktury.

V souborech se zdrojovými kódy je třeba dodržovat jistou hierarchii (viz níže).

```c
// Priority 0: Compiler's directives
#define CONST_VALUE 1
#define MY_MACRO(param) do { /* ... */ } while(0);

// Priority 1: Type definitions
typedef int my_int_t;
typedef struct super_struct {
    // ...
} sstruct_t;

// Priority 2: Function declarations
void sstruct_init(sstruct_t *sstruct);

// Priority 3: Own functions
int long_function_name(first_parameter, second_parameter)
{
    // ...
}

// Variables
int variable_with_longer_name = 11;
char basic_var, *pointer_var = NULL;
```

#### Podmínky

Pokud to není vyloženě lepší (velké množství porovnání), upřednostní se větvené
`if`y před `switch`em. Switch se vyplatí u 4 a více hodnot, ale na i tak je vždy
lepší promyslet, jestli nebude přehlednější využití `if`ů.

V případě potřeby využití jednoho obslužného kódu pro více případů je třeba doplnit
komentář informující o tom, že nebyl zapomenut `break`, ale jedná se o záměr.

```c
if (condition1) {
    // ...
} else if (condition2 && condition3) {
    // ...
} else {
    // ...
}

switch (variable) {
    case 0:
        // ...
        break;
    case 1: // No break here
    case 2:
        // ...
        break;
    default:
        // ...
        break;
}

int ternary_opp_example = (condition ? 0 : 1);
int long_ternary_opp = (condition
        ? true_expression
        : false_expression);
```

#### Cykly

U cyklů se upřednostňuje postfixový formát inkrementace/dekrementace. Je třeba
zachovávat stejný formát mezer a stejnou pozici složených závorek (viz níže).

Pokud možno se vyhýbat `break`, většinou to značí špatně formulovanou podmínku.
Tady je prostě třeba využít selský rozum a zvolit přehlednější variantu.

```c
for (unsigned int i = 0; i < COUNT; i++) {
    // ...
}

while (condition) {
    // ...
}

do {
    // ...
} while (condition);
```

#### Funkce

Funkce musí mít dokumentační komentář (již bylo zmíněno). Dále je nutné rozumně
pojmenovávat formální parametry, aby byly čitelné a na první pohled bylo jasné,
co funkce pro svůj běh potřebuje. Někdy je lepší použít delší, ale jasnější
identifikátory. Každopádně je třeba jejich význam jasně vysvětlit v dokumentačním
komentáři.

Deklarace lokálních proměnných je třeba dělat na začátku těla funkce, pokud
se nejedná o iterační proměnnou cyklu nebo jiný podobný případ, který působí
na velmi malém úseku kódu. Příkaz `return` je vhodné od ostatního kódu oddělovat
prázdným řádkem, lépe se poté hledá. Případné `free` a volání "úklidových" funkcí
je naopak vhodné nalepit těsně k němu.

```c
/**
 * Starts and controls this program
 * 
 * @param argc Number of input arguments from CLI
 * @param argv Input arguments from CLI
 * @return Program's exit code (== 0 for success, != 0 for error)
 */
int main(int argc, char *argv[])
{
	int first_var;
	char *second_var, another_var;
	// ...
	
    // Some code...
    
    return 0;
}
```

#### Návěští

V případě, že je třeba dostat se z 3. (nebo nižší) úrovně zanoření cyklů, je vhodné
použít `goto` na rozumné návěští. Takové věci je však třeba psát s rozmyslem!

```c
for (...) {
    for (...) {
        for (...) {
            if (something) {
                goto out_of_cycles;
            }
            // ...
        }
    }
}

out_of_cycles:
// ...
```

Dále se návěští dá vcelku dobře použít, když je třeba před vyskočením z funkce
provést několik "úklidových" příkazů, a dojde k nějaké chybě. Poté se prostě jen
skočí na příslušné návěští, provede se úklid a z fukce se mizí pryč.
