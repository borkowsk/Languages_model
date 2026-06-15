#	"JEZYK" czyli LANGUAGE:

## HISTORIA

 *	ver.   2.20c - Angielskojęzyczne komentarze.
 *	ver.   2.20b - ADAPTACJA DO NOWYCH WARUNKÓW KOMPILACJI W clion W ROKU 2026.
 
 ...[kilkanaście lat odstępu]...
 
 *	ver.   2.20 - Wprowadzenie 16 klas do Log-Log histogramu dodanego w wersji 1.401.
               - Zmiana kolejności serii w pliku log.
               - Wprowadzenie  konsolowego trybu pracy (w tle, w ogóle bez grafiki).
 *	ver.   2.11 - początki użycia klas `OptionalParameters` do obsługi parametrów wywołania (potem z nich zrezygnowano).
 *	ver.   2.10 - implementacja procesu zaciągania władzy i wyświetlanie mapy politycznej.
 *	ver.   2.06 - działające sterowanie częstością wyświetlania i `my_area_manager` zadeklarowany.
 *	ver.   2.05 - rozbudowane menu, zwłaszcza nowe opcje wizualizacji, parametr `SRND` i `DUMP`.
 *	ver.   2.04 - Ciagłe zrzucanie sieci jako parametr.
 *	ver.   2.03a - Implementacja zrzutu sieci SW w postaci plików NET.
 *	ver.   2.01-2 - Uruchomienie modelu Small Worlds.
 *	ver.   1.99b - przejście do modelu Small Worlds:
                  - Przygotowanie nowego układu wizualizacji.
                  - Przygotowanie źródeł danych i grafu dla dalekich połączeń.
                  - Implementable algorytmu dynamicznych dalekich połączeń "politycznych".
                  - Włączenie wpływów z dalekich połączeń do implementacji modeli wpływu
                    (nie przetestowane dla skomplikowanych biasów).

 *	ver.   1.53a    - drobne zmiany konieczne do uruchomienia kompilacji pod BDS 2006.

 *	ver.   1.51-2   - drobne zmiany kosmetyczne.

 *	ver.   1.5      - dodanie mapy języków w "TrueColor", i map składowych w kolorach składowych RGB.
                   - Zmiana domyślnych parametrów startowych.
 *	ver.   1.41a                                                                      
                   - dodanie wieku języka danego agenta i seri danych to przedstawiającej
                     na wykresie logarytmicznym.
     
 *	ver.   1.402a
                   - Wprowadzenie 12 klas do LogLog histogramu dodanego w wersji 1.401.
 *	ver.   1.401b
                   - zmiana histogramu LogLog rozmiaru języków na typ fix o 6 klasach,
                     czyli efekt wizualny bardzo podobny, ale inaczej oznakowane klasy.
                   - dodanie fix-histogramu klas rozmiaru języków.
==========================
 *	ver.   1.4      - dodanie mutacji spontanicznych we wszystkich trybach biasu,
                   - wprowadzenie możliwości wyłączenia korelacji przestrzennej (use_spatial_corr) z KODU!!!
                   - wprowadzenie wykresu log-log rozkładu rozmiarów języków (dhistosou.h),
                   - wprowadzenie wpisywania tego histogramu do logu.
                   - uzupełnienie części komunikatów o ustawieniach parametrów (ale cześć została bez).
                   - wprowadzenie parametru DSTB ustalającego rodzaj i stopień rozkładu sił (uzyskiwany przez * lub +).
* TEST:
```
             WIDTH=100 DSTB=-8 CLSS=8 MIPO=3 RSPC=1 VIEW=50 LOGF=10 LOGF=testW100.log
```

 *	ver.   1.35a - ???
 *	ver.   1.34a - usuniecie asercji zabezpieczających przed agentami
                 o zerowej sile i wprowadzenie minimalnej sily (def. = 1).
 *	ver.   1.33a - rekompilacja do nowej biblioteki i dodanie menu.
 *	ver.   1.32a - rekompilacja z nowa wersja biblioteki wizualizacji
                  i wprowadzenie pliku "about_languages.cpp" z kontrolą czasu dla DEBUG.
 *	ver.   1.31a - wprowadzono inicjalizacje języków (poglądów) z trzech plików graficznych w odcieniach szarości.
 *	ver.   1.30a - wprowadzony podwójny bias w wersji następczej, ale równoległy działa źle —
                    wymaga inteligentnego przeważenia liczników.
 *	ver.   1.20a - przygotowanie do wprowadzenia "podwójnego biasu".
 *	ver.   1.01b - poprawiony default dla progu siły.
 *	ver.   1.05b - wbudowana obsługa pracy w batch-u i powtarzania eksperymentu.
 *	ver.   1.10b - wprowadzenie "biasu" dla parametrów języka.
