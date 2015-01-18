komunikacja web <-> server

1. Klient webowy może wysłać 4 rodzaje poleceń: START, STOP, GET_DATA, LIST_CLIENTS - każdy z nich to jeden bajt: START to 1, STOP = 2, GET_DATA = 3, LIST_CLIENTS. 
2. W odpowiedzi na START, STOP, GET_DATA otrzymuje jeden bajt odpowiedzi: 0 = OK, 1 = ERROR. 
3. Klient webowy nie otrzymuje żadnych danych od agenta po GET_DATA - to tylko request zapisania tych danych do bazy. 
4. W odpowiedzi na LIST_CLIENTS klient webowy otrzymuje najpierw dwa bajty - tekstowo liczba klientów (np. "02", "13", "00"), czyli N. Następnie otrzymuje N wiadomości po 20 bajtów w postaci: dwuznakowe id klienta, spacja, tekstowo ip w standardowym zapisie. Generalnie te linie można prosto drukować na ekran. 
