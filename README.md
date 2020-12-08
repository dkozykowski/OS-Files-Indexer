<div id="content">

# Systemy Operacyjne 1: Projekt Indeksowanie plików w katalogach

<div id="table-of-contents">

## Table of Contents

<div id="text-table-of-contents">

*   [1\. Zarys projektu](#org7fe9d26)
*   [2\. Wywołanie i parametry](#orgbbaa29a)
*   [3\. Działanie programu](#org22e4824)
    *   [3.1\. Procedura indeksowania](#org5fd9498)
    *   [3.2\. Dostępne komendy](#org45a70ee)
        *   [3.2.1\. Stronicowanie danych](#org7c2b0db)
    *   [3.3\. Struktura danych](#org873e335)
    *   [3.4\. Zapis i odczyt pliku](#org478e86f)
    *   [3.5\. Indeksowanie w tle](#org19ab485)
*   [4\. Ograniczenia](#org6848216)

</div>

</div>

<div id="outline-container-org7fe9d26" class="outline-2">

## <span class="section-number-2">1</span> Zarys projektu

<div class="outline-text-2" id="text-1">

W ramach projektu ma powstać program, który: przechodzi wszystkie pliki w zadanym katalogu i podkatalogach, tworzy w pamięci strukturę danych zawierającą potrzebne dane o tych plikach, a następnie czekający na komendy będące zapytaniami o dane zawarte w tej strukturze. Aby nie przeglądać za każdym razem całego katalogu, cała struktura danych jest zapisywana do pliku i wczytywana w kolejnych wywołaniach programu. Strukturę danych opisującą zawartość katalogów będziemy nazywać _indeksem_.

Pewne operacje muszą odbywać w oddzielnych wątkach i jest to zaznaczone w treści zadania. Nie można z tych wątków zrezygnować. Można jednak tworzyć wątki, których treść zadania nie narzuca, jeśli jest taka potrzeba.

</div>

</div>

<div id="outline-container-orgbbaa29a" class="outline-2">

## <span class="section-number-2">2</span> Wywołanie i parametry

<div class="outline-text-2" id="text-2">

Powinien powstać jeden program o nazwie `mole`, który przyjmuje następujące parametry wywołania:

<dl class="org-dl">

<dt>-d sciezka</dt>

<dd>ścieżka do katalogu, który będzie przeglądany. W przypadku, gdy opcja nie jest podana, zamiast tego wykorzystywana jest ścieżka wskazywana przez zmienną środowiskową `$MOLE_DIR`. Jeśli zmienna nie jest ustawiona, to wyświetlany jest błąd wywołania i program się kończy. Ostateczna wartość tego parametru będzie dalej oznaczona **path-d**.</dd>

<dt>-f sciezka</dt>

<dd>ścieżka do pliku, z którego będzie czytany/zapisywany indeks. Jeśli opcja nie jest podana, to zamiast tego wykorzystywana jest ścieżka wskazywania przez zmienną środowiskową `$MOLE_INDEX_PATH`. Jeśli zmienna nie jest ustawiona, to domyślną wartością jest plik `.mole-index` w katalogu domowym użytkownika. Ostateczna wartość tego parametru będzie dalej oznaczona **path-f**.</dd>

<dt>-t n</dt>

<dd>gdzie n to liczba całkowita z przedziału \([30,7200]\). Jest to czas przerwy w sekundach pomiędzy kolejnymi przebiegami indeksowania katalogu opisanymi dalej. Ta opcja jest opcjonalna, jeśli nie jest podana, to periodyczne uruchamianie indeksowania jest wyłączone. Wartość parametru będzie dalej oznaczona jako **t**.</dd>

</dl>

</div>

</div>

<div id="outline-container-org22e4824" class="outline-2">

## <span class="section-number-2">3</span> Działanie programu

<div class="outline-text-2" id="text-3">

Po uruchomieniu program próbuje otworzyć plik **path-f** i jeśli ten istnieje, czyta z niego indeks katalogu. W przeciwnym przypadku uruchamia procedurę indeksowania opisaną dalej. Następnie przechodzi do oczekiwania na komendy wpisywane na `stdin` przez użytkownika.

</div>

<div id="outline-container-org5fd9498" class="outline-3">

### <span class="section-number-3">3.1</span> Procedura indeksowania

<div class="outline-text-3" id="text-3-1">

Indeks ma zbierać informacje o następujących typach plików:

*   katalogi,
*   obrazy JPEG,
*   pliki PNG,
*   pliki skompresowane gzipem,
*   pliki skompresowane zipem, w tym wszystkie pliki typu docx, odt i inne, które korzystają z formatu zip.

Rozpoznawanie typu pliku ma się odbywać na podstawie odczytu sygnatury pliku (tzw. magic number), a nie na podstawie rozszerzenia. Wszystkie pozostałe typy plików są w indeksie pomijane.

Indeks ma przechowywać następujące informacje:

*   nazwę pliku,
*   pełną ścieżkę,
*   rozmiar,
*   uid właściciela,
*   typ (jeden z wyżej wymienionych).

Procedura indeksowania polega na uruchomieniu pojedynczego wątku, który w pierwszej kolejności czyści strukturę indeksu, a następnie sprawdza wszystkie pliki w katalogu **path-d** i podkatalogach. Dla każdego z plików wykonuje sprawdzenie. czy typ pliku należy do typów, które indeksujemy i jeśli tak jest, zapisuje wymienione dane pliku do indeksu. Na koniec zapisuje indeks do pliku **path-f**.

Po zakończeniu indeksowania, które dzieje się zawsze w tle (w oddzielnym wątku), na stdout wypisywany jest komunikat o zakończeniu działania.

</div>

</div>

<div id="outline-container-org45a70ee" class="outline-3">

### <span class="section-number-3">3.2</span> Dostępne komendy

<div class="outline-text-3" id="text-3-2">

Przetwarzanie komend ma działać możliwie równolegle z indeksowaniem. Nie jest dopuszczalne, żeby w trakcie potencjalnie długiego indeksowania, dostęp do zapytań był zablokowany. Natomiast dopuszcza się sytuację, gdy odpowiedzi na zapytania są na bazie starych danych, jeśli nowe indeksowanie jeszcze się nie zakończyło.

Program ma oczekiwać na kolejne linie z stdin. Każda linia powinna zawierać jedną z podanych komend. Jeśli wejście od użytkownika nie jest poprawną komendą wyświetlany jest błąd i program oczekuje na kolejne wejście.

Komendy:

<dl class="org-dl">

<dt>exit</dt>

<dd>rozpoczyna procedurę kończenia programu – program przestaje przyjmować kolejne komendy, jeśli aktualnie jakaś operacja indeksowania jest w toku, to program czeka na jej zakończenie (w tym na zapisanie wyniku indeksowania do pliku), po czym cały program kończy się.</dd>

<dt>exit!</dt>

<dd>szybkie kończenie programu – program przestaje przyjmować komendy. Jeśli trwa proces indeksowania, jest on przerywany. Jeśli trwa zapis wyniku indeksowania do pliku, to zapis zostaje dokończony (niedopuszczalne jest pozostawienie pliku indeksu w niespójnym stanie). Potem program się kończy.</dd>

<dt>index</dt>

<dd>jeśli w danej chwili nie trwa indeksowanie, to uruchamia w tle indeksowanie katalogu **path-d** i natychmiast zaczyna oczekiwać na kolejną komendę z stdin. Jeśli w danej chwili indeksowanie jest w toku, to wyświetla komunikat z tą informacją i nie powoduje żadnych dodatkowych akcji.</dd>

<dt>count</dt>

<dd>przechodzi po całym indeksie i wylicza ile jest plików poszczególnych typów i wypisuje tę informację na stdout w formie prostej tabeli</dd>

<dt>largerthan x</dt>

<dd>x jest żądanym rozmiarem pliku. Przechodzi po indeksie i wypisuje na stdout informacje: pełna ścieżka, rozmiar, typ na temat wszystkich plików, których rozmiar jest większy od podanego x</dd>

<dt>namepart y</dt>

<dd>y jest fragmentem nazwy pliku, potencjalnie zawierającym spacje. Przechodzi po indeksie i wypisuje wszystkie wpisy, gdzie y jest fragmentem nazwy pliku. Wypisuje te same informacje o plikach, co largerthan.</dd>

<dt>owner uid</dt>

<dd>uid jest identyfikatorem użytkownika. Przechodzi indeks i wypisuje na stdout, w tym samym formacie, co poprzednio, pliki, których właścicielem jest podany użytkownik.</dd>

</dl>

</div>

<div id="outline-container-org7c2b0db" class="outline-4">

#### <span class="section-number-4">3.2.1</span> Stronicowanie danych

<div class="outline-text-4" id="text-3-2-1">

Jeśli którakolwiek z komend largerthan, namepart, owner, będzie wypisywać więcej niż 3 rekordy, należy dostarczyć użytkownikowi możliwość przewijania wyników. W tym celu należy skorzystać ze zmiennej środowiskowej `$PAGER`. Jeśli zmienna środowiskowa `$PAGER<div class="outline-text-4" id="text-3-2-1" nie jest ustawiona lub wynik zawiera nie więcej niż trzy rekordy, to po prostu wypisujemy znalezione rekordy na stdout. W przeciwnym przypadku, należy uruchomić program o nazwie wskazywanej przez zmienną pager z użyciem funkcji `popen` i do uzyskanego strumienia plikowego, który będzie standardowym wejściem uruchomionego programu, wypisywać kolejne rekordy. Na koniec zamknąć strumień i zaczekać na proces potomny z użyciem `pclose`.

Funkcje `popen` i `pclose` nie były omówione na zajęciach, należy samodzielnie uzyskać informacje na temat ich działania.

Przykładową wartością zmiennej `$PAGER` może być less. Wtedy to przewijania wyników będzie użyty właśnie program less.

</div>

</div>

</div>

<div id="outline-container-org873e335" class="outline-3">

### <span class="section-number-3">3.3</span> Struktura danych

<div class="outline-text-3" id="text-3-3">

Struktura danych realizując indeks nie jest narzucona, można używać dowolnych struktur, w tym tablic, drzew, list. Projektując strukturę należy wziąć pod uwagę to, że będzie ona odczytywana i zapisywana z/do pliku, więc stosowanie np. wskaźników niesie ze sobą dodatkowe komplikacje.

Można założyć, że długości nazw plików i ścieżek są ograniczone. W takim przypadku w trakcie indeksowania wykrywać, gdy nazwy są zbyt długie i wypisywać komunikat ostrzegawczy (bez przerywania programu). Limity długości muszą być łatwe do skonfigurowania na etapie kompilacji.

</div>

</div>

<div id="outline-container-org478e86f" class="outline-3">

### <span class="section-number-3">3.4</span> Zapis i odczyt pliku

<div class="outline-text-3" id="text-3-4">

Zapis i odczyt pliku indeksu tylko z użyciem niskopoziomowych operacji na plikach. Dodatkowo nie wolno bez potrzeby dokonywać konwersji liczb, czy innych bytów na napisy, a zapisywać je binarnie. Nie jest wymagane, aby zapisane pliki dało się przenosić między architekturami (czyli nie są problemem różnice w takich rzeczach jak: długości typów, kolejność bajtów w liczbie, wyrównanie pól struktur).

</div>

</div>

<div id="outline-container-org19ab485" class="outline-3">

### <span class="section-number-3">3.5</span> Indeksowanie w tle

<div class="outline-text-3" id="text-3-5">

Jeśli program został uruchomiony w parametrem **t**, uruchamiany jest wątek, który uruchamia ponowne indeksowanie katalogu **path-d**, jeśli ostatnie indeksowanie zostało wykonane więcej niż **t** sekund temu. Czas ostatniego indeksowania uwzględnia zarówno indeksowanie na starcie programu, jeśli wystąpiło, indeksowanie zapoczątkowane komendą użytkownika oraz indeksowanie inicjowane po przekroczeniu czasu **t**. W przypadku, gdy program został uruchomiony i istniał już plik z indeksem, to za czas ostatniego indeksowania przyjmujemy datę modyfikacji pliku z indeksem (to może oznaczać uruchomienie indeksowania tuż po starcie programu).

</div>

</div>

</div>

<div id="outline-container-org6848216" class="outline-2">

## <span class="section-number-2">4</span> Ograniczenia

<div class="outline-text-2" id="text-4">

Do operacji na pikach należy zastosować niskopoziomowe funkcje IO POSIX. Do operacji na stdin, out, err można użyć buforowanych funkcji ze standardowego C.

</div>

</div>

</div>

<div id="postamble" class="status">
</div>
