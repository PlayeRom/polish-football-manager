
#ifndef STRUCTS_H
#define STRUCTS_H

#include <cstring>

#define VERSION "1.07"

// Initial data file names:
#define FILE_FOOTBALLERS            "data/footballers.cfg" // old name Kluby_01.cfg
#define FILE_FOOTBALLERS_RIVALS     "data/footballers-rival.cfg" // old name Rywal2.cfg
#define FILE_FOOTBALLERS_TRANSFER   "data/footballers-transfer.cfg" // old name Transfer.cfg
#define FILE_ROUNDS                 "data/rounds.cfg" // old name Kolejka.cfg
#define FILE_MSG_MANAGER            "data/%s-msg-manager.cfg" // old name News.cfg
#define FILE_MSG_MATCH              "data/%s-msg-match.cfg" // old name Mecz.cfg
// Save file names:
#define FILE_SAVE_FOOTBALLERS_MANAGER   "save/footballers_manager.cfg" // old name Gra_mana.cfg
#define FILE_SAVE_FOOTBALLERS_RIVAL     "save/footballers_rival.cfg" // old name Rywal.cfg
#define FILE_SAVE_FOOTBALLERS_TRANSFER  "save/footballers_transfer.cfg" // old name Trans.cfg
#define FILE_SAVE_TABLE                 "save/table.cfg" // old name Tabela.cfg
#define FILE_SAVE_ROUNDS                "save/rounds.cfg" // old name Kolejka2.cfg
#define FILE_SAVE_MANAGER_NEWS          "save/manager-news.cfg" // old name News_m.cfg
#define FILE_SAVE_LAST_MATCH_REPORTS    "save/last-match.cfg" // old name Lastmatch.cfg
#define FILE_SAVE_CLUB                  "save/club.cfg" // old name Klub.cfg

#define ERROR_OPEN_FILE             "ERROR: open file \"%s\" failed!"

#define MAX_USER_NAME 15
#define MAX_USER_SURNAME 20
#define MAX_USER_NICK 20
#define MAX_CLUBS 16
#define MAX_TEAM_INSTRUCTIONS 6 // ilosc instrukcji dla zespolu w taktyce

#define TRAINING_CONDITIONS     1
#define TRAINING_PASSES         2 // podanie
#define TRAINING_FREE_KICKS     3 // rzuty wolne, stale frag. gry
#define TRAINING_TACTICKS       4 // taktyka
#define TRAINING_B              5
#define TRAINING_O              6
#define TRAINING_P              7
#define TRAINING_N              8
#define TRAINING_HOLIDAY        9 // wolne od treningu, odpoczynek
#define TRAINING_EMPTY          10 // wypelniacz pustego miejsca

// wartosci dla instrukcji w taktyce
#define INSTR_PASSES_MIXES      1 // podania mieszane
#define INSTR_PASSES_SHORT      2 // podania krotkie
#define INSTR_PASSES_MIDDLE     3 // podania srednie
#define INSTR_PASSES_LONG       4 // podania dlugie

#define INSTR_TREATMENT_NORMAL  1 // traktowanie rywala: normalnie
#define INSTR_TREATMENT_SOFT    2 // traktowanie rywala: delikatnie
#define INSTR_TREATMENT_HARD    3 // traktowanie rywala: twardo

#define INSTR_YES               1 // ogolnie tak
#define INSTR_NO                0 // ogolnie nie

#define INSTR_ATTIT_NORMAL      1 // nastawienie normalne
#define INSTR_ATTIT_DEFENSIVE   2 // nastawienie obronne
#define INSTR_ATTIT_ATTACK      3 // nastawienie atak

#define TRANNING_SLOTS_NUMBER   28 // 7 days x 4 slots per day

struct SClub {
    wchar_t managerName[MAX_USER_NAME];
    wchar_t managerSurname[MAX_USER_SURNAME];
    wchar_t nick[MAX_USER_NICK];
    int isNick; // 1 - user podał nick, 0 - nie podał nicka
    int roundNumber;
    int isMatch; // 0 - nie ma meczu, wtedy "C = Kontynuj", 1 - jest mecz "C = Mecz"
    int clubId; //numer od 1 do 16 oznaczający klub,
    int isBlockTransferFunds; //kasa - blokada dla fund. na transfery, 1 = blokada, bo prosic o fundusze mozna raz na ture
    int day, month, year, weekNumber; //dzien,miesiac,rok
    int training[TRANNING_SLOTS_NUMBER];
    /**
     * trening[TRANNING_SLOTS_NUMBER]
     * Tygodniowy plan treningu, mamy 7 dni po 4 sloty w ktorych coś jest trenowane
     * Wartości:
     * 1 - trenuja kondycje
     * 2 - podania
     * 3 - stałe fragmenty gry
     * 4 - taktyka
     * 5 - B
     * 6 - O
     * 7 - P
     * 8 - N
     * 9 - wolne od treningu TRAINING_HOLIDAY
     * 10 - plomba
     */

    // +/-BOPN
    int treBOPN;

    int inst[MAX_TEAM_INSTRUCTIONS];
    /**
     * instrukcje dla zespolu podczas meczu
     * 0 - podania: 1 - mieszane, 2 - krótkie, 3 - srednie, 4 - dlugie
     * 1 - obochodzenie sie z rywalem: 1 - normalne, 2 - delikatne, 3 - twarde
     * 2 - pressing: 0 - nie, 1 - tak
     * 3 - pułapki ofsajdowe: 0 - nie, 1 - tak
     * 4 - gra z kontry: 0 - nie, 1 - tak
     * 5 - nastawienie: 1 - normalne, 2 - obronne, 3 - atak
     */

    int teamSetting; // ustawienie zespolu
    int isAssistantMsg; // asystent=1 widomość od asystenta
    int lastMatchRivalId;
    int playerGoals, rivalGoals; // gole z ostatniego meczu playerGoals - gole gracza, rivalGoals - gole przeciwnika
    int isRiot;
    float trained[5];
    /** punkty wytrenowania zespolowego
     * 0 - kondycja
     * 1 - podania
     * 2 - stałe fragmenty
     * 3 - taktyka
     */

    float finances[14];
    /**
     * finanse
     * // przygody
     * 0 - bilety
     * 1 - dochod TV
     * 2 - handel przed stadionem
     * 3 - sprzedaz zawodnikow
     * 4 - reklama i sponsorzy
     * 5 - razem zysk
     * // wydatki
     * 6 - płące
     * 7 - premie
     * 8 - kupno zawodnikow
     * 9 - kary ligowe
     * 10 - razem wydatki
     * //
     * 11 - zysk (roznica przychod - wydatki)
     * 12 - calkowita kasa w klubie
     * 13 - fundusze na transfery (kasa/2)
     */

    float financesLastMonth[14]; // finanse2 - to samo co finanse tylko dla poprzednigo miesiaca
    float ileTrans; // TODO do usuniecia, nie uzywane
    float totalExpensesTransfers; // łączna suma wydaków na kupno zawodników
    float totalRevenuesTransfers; // łączna suma zysków za sprzedarz zawodników

    int rivalData[4];
    /**
     * rivalData[4]
     * 0 - numer klubu
     * 1 - 0 - gram w domu, 1 - gram na wyjeździe
     * 2 - taktyka / ustawienie rywala
     * 3 - numer jako kolej. kol, czyli index klubu rywala w kolejce Kolejka.nr[rywal[3]]
     */

    int rivalInst[MAX_TEAM_INSTRUCTIONS]; // instrukcje rywala, to samo co inst, tylko ze dla rywala

    int isRivalSet, controlMatchesAmount; //dla rywala

    // gdy = 1 to nie mozna rozegrac meczu bo w skaldzie sa niepowolane osoby, z kartkami (1 czerwona, lub 2 zolte),
    // albo gracz w skladzie do wyleczenia kontuzji ma wiecej niz 7 dni
    // gdy = 0, to mozna przejsc do meczu
    int isNotAllowedTeamPlayer;

    int goalsLeague[480];
    int goalsControls[10]; //gole dla meczy kontrolnych

    int managerStats[12];
    /**
     * Tablica Club.managerStats[12] (statystyki managera)
     * 0 - wygrane ligi
     * 1 - wygrane puchary
     * 2 - przyznane nagrody dla managera
     * 3 - punkty managera
     * 4 - rozegrane mecze managera
     * 5 - mecze wygrana managera
     * 6 - mecze zremisowane managera
     * 7 - mecze przegrane managera
     * 8 - zdobyte gole managera
     * 9 - stracone gole managera
     * 10 - ilość wszytskich kupionych zawodnikow managera
     * 11 - ilość wszytskich sprzedanych zawodnikow managera
     */

    int season; // wiosna, jesien

    // dla opcji wyswietlania napisow w meczu, 1 - automatycznie, 0 - za kazdym razem press any key
    int isMatchAutoMsg;
    int isWalkover; // = 1 - last match lose by walkover
};

struct SRound {
    int number, day, month, year; //nr kolejki dniń miesiąc i rok
    int clubNumbers[MAX_CLUBS]; //numerowe oznaczenie drużyn dla łatwej identyfikacji, ustawione parami:
    // dom - wyjazd
    // dom - wyjazd, itd.
    // czyli jak mamy sekwencję id1, id2, id3, id4, id4...
    // to id1 gra z id2, id3 gra z id4, ird

    // TODO: kol1 wydaje się niepotrzebne, mamy nr jako numery klubow i potem mozemy siegnac do tablicy clubsNames
    wchar_t kol1[MAX_CLUBS][19]; //16 drużyn po 19 znaków
};

struct STable {
    int clubId; // numer klubu, licząc od 1, ktorego dane dotycza
    int numberMatchesPlayed; // rozegrane mecze
    int wins;                // wygrane
    int draws;               // remisy
    int losses;              // przegrane
    int goalsScored;         // gole zdobyte
    int goalsLost;           // gole strancone
    int goalsDiff;           // roznica goli
    int points;              // punkty
    int tablePosition;       // pozycja w tabeli
} /*__attribute__((packed))*/;

#define MAX_NEWS_LENGTH 500

struct SNews {
    int num; // number of message
    std::wstring message; // content of message
    SNews() : num(0), message(L"") {}
};

// pozycja na ktore gra gracz, dla Footballer.data[2]
#define PLAYERS_POS_B 1 // goalkeeper
#define PLAYERS_POS_O 2 // defender
#define PLAYERS_POS_P 3 // midfielder
#define PLAYERS_POS_N 4 // striker
// co zawodnik trenuje
#define PLAYERS_TRAINING_NONE 0 // nie trenuje
#define PLAYERS_TRAINING_B    1 // bycia bramkarzem
#define PLAYERS_TRAINING_O    2 // obronę
#define PLAYERS_TRAINING_P    3 // pomoc
#define PLAYERS_TRAINING_N    4 // atak
// morale zawodnika
#define PLAYERS_MORALE_FATAL   -3
#define PLAYERS_MORALE_BAD     -2
#define PLAYERS_MORALE_LOW     -1
#define PLAYERS_MORALE_MIDDLE   0
#define PLAYERS_MORALE_GOOD     1
#define PLAYERS_MORALE_VGOOD    2
#define PLAYERS_MORALE_SUPER    3

#define MORALE_THRESHOLD        5

struct SFootballer {
    wchar_t name[4], surname[20];
    int data[23];
    /**
     * Tabela SFootballer.data
     * 0 - liczba porzadkowa gracza w druzynie, np. 1 - to bramkarz, 2 - obrońca, ... 12 - pierwszy rezerwowoy, itd.
     * 1 - co trenuje, 0 - nie trenuje, 1 - bramkarz, 2 - obrona, 3 - pomoc, 4 - atak
     * 2 - pozycja na jakiej gra, 1 - B, 2 - O, 3 - P, 4 - N
     * 3 - umiejetnosci B, od 1 do 20
     * 4 - umeijetnosci O, od 1 do 20
     * 5 - umiejetnosci P, od 1 do 20
     * 6 - umiejetnosci N, od 1 do 20
     * 7 - morale zawodnika: -3 - "Fatalne", -2 "Złe", -1 "Niskie", 0 "Średnie", 1 "Dobre", 2 "B.dobre", 3 "Super";
     * 8 - licznik zmiany morale, gdy osiagnie -5 lub 5, to sie zeruje i modyfikuje odpowiednio morale [7]
     * 9 - forma zawodnika od 1 do 10
     * 10 - licznik zmiany formy, gdy osiagnie wartosci -3 lub 3, to sie zeruje i odpowiednio zmienia forme zawodnika [9]
     * 11 - kondycja od 0 do 100 %
     * 12 - zolte kartki dla meczu (co mecz sie zeruje), = 1 gdy zawodnik otrzyma zolta w meczu, = 2 to czerwona
     * 13 - zółte kartki, = 2 to gracz pauzuje w przyszlym meczu i sie zaruje
     * 14 - czerwone kartki, gdy = 1 to gracz paizuje i się zeruje
     * 15 - kontuzja, = 0 gdy zawodnik jest zdrowy, gdy > 0 przechowuje ilosc dni do wyleczenia kontuzji, jezeli tych
     *      dni jest 7 lub mniej, to zawodnik moze grac, ale jego kondycja jest ograniczona max do 90%
     * 16 - ilość zdobytch goli
     * 17 - normalnie 0, gdy = 1 to zawodnik wystawiony jest na sprzedaz
     * 18 - za ile dni wygasa kontrakt zawodnika
     * 19 - flaga czy gracz kontuzjowany, 0 - nie, 1 - tak, potrzebne do okreslenia czy byl komunikat o jego kontucji
     * 20 - forma podczas meczu
     * 21 - gole zdobyte w meczu, co mesz sie zeruje, po meczu dodawane do [16]
     * 22 - numer kluby do ktorego nalezy zawodnik, gdy 0 to nie nalezy do zadnego
     */
    float finances[4]; // 16 bajtów
    /**
     * Tabela SFootballer.waga
     * 0 - wartosc/cena $ zawodnika
     * 1 - miesięczna pensja zawodnika
     * 2 - premia za gola dla zawodnika
     * 3 - cena sprzedazy zawodnika
     */
};

/**
 * Raport ostatniego meczu, struktura jednego wpisu razem z kolorem
 */
struct SLastMatch {
    int textcolor;
    wchar_t text[MAX_NEWS_LENGTH];
};

#endif /* STRUCTS_H */
