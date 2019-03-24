/* Komentarze zostały przetłumaczone przez Macieja Michalca
 * na podstawie oryginalnej wersji ze strony incise.org */

/* TinyWM zostało napisane przez Nicka Welcha <mack@incise.org>, 2005.
 *
 * To oprogramowanie jest w domenie publicznej
 * i jest dostarczane bez żadnych gwarancji */


/* Głównym celem tinywm jest służyć jako bardzo podstawowy przykład tego, jak
 * programować rzeczy związane z systemem X Window i/lub pozwolić na zrozumienie
 * menadżerów okien, dlatego postanowiłem umieścić w kodzie komentarze 
 * objaśniające, ale naprawdę nienawidzę przedzierać się przez kod zawierający
 * za dużo komentarzy - z tego powodu tinywm ma być tak zwięzły jak to 
 * tylko możliwe. Zbyt wiele komentarzy nie pasuje do tego. Chciałem, aby 
 * tinywm.c było czymś, na co tylko spojrzysz i powiesz: "wow, to wszystko? 
 * świetnie!", więc po prostu skopiowałem kod do annotated.c i skomentowałem
 * obszernie. Ahh, ale teraz muszę dokonywać wszystkich zmian w kodzie 
 * dwukrotnie! Oh, dobrze. Mógłbym zawsze używać jakiegoś skryptu do wycinania 
 * komentarzy i zapisywania tego do tinywm.c ... nah. 
 */

/* większość rzeczy związanych z X-ami będzie zaincludowanych z pliku xlib.h, 
 * ale kilka elementów wymaga innych nagłówków, takich jak Xmd.h, keysym.h, etc.
 */
#include <X11/Xlib.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

int main()
{
    Display * dpy;
    Window root;
    XWindowAttributes attr;

    /* używamy tego, żeby zapisać stan wskaźnika na początku 
     * przesuwania/zmieniania rozmiaru
     */
    XButtonEvent start;

    XEvent ev;


    /* zwróć kod błędu jeśli nie możemy się połączyć */
    if(!(dpy = XOpenDisplay(0x0))) return 1;

    /* zazwyczaj często będziesz się odnosił do nadrzędnego okna. Jest to 
     * trochę naiwne podejście, które będzie działać tylko na domyślnym 
     * ekranie. Większość ludzi ma tylko jeden ekran, ale nie wszyscy. Jeśli
     * uruchamiasz tryb wielomonitorowy bez xineramy, to prawdopodobnie masz
     * kilka ekranów [zapewne chodzi o konfigurację, w której nie można 
     * przenosić okna między ekranami, bo nie stanowią one jednego
     * wirtualnego ekranu  - przyp. tłum.] (nie jestem pewien implementacji
     * specyficznej dla producentów, np. nvidii)
     *
     * wiele, prawdopodobnie większość menadżerów okien obsługuje tylko jeden
     * ekran, więc w rzeczywistości nie jest to takie "naiwne"
     * 
     * jeśli chcesz uzyskać dostęp do nadrzędnego okna konkretnego ekranu,
     * możesz użyć funkcji RootWindow(), ale użytkownik też może kontrolować
     * który ekran jest naszym domyślnym: jeśli ustawi zmienną $DISPLAY
     * na ":0.foo", to naszym domyślnym numerem ekranu jest to, co zostanie
     * wstawione jako "foo". 
     */
    root = DefaultRootWindow(dpy);

    /* mógłbyś też załączyć keysym.h i użyć stałej XK_F1 zamiast wywołania
     * XStringToKeysym, ale ta metoda jest bardziej "dynamiczna". Wyobraź
     * sobie, że masz pliki konfiguracyjne, specyfikujące bindingi klawiszy. 
     * Zamiast parsowania nazw klawiszy i posiadania ogromnej tabeli lub 
     * czegoś co mapuje stringi do stałch XK_*, możesz po prostu wziąć nazwę
     * klawisza i przekazać ją do XStringToKeysym. Funkcja ta zwróci ci w zamian
     * odpowiedni keysym (symbol klawisza) albo powie ci, że nazwa klawisza
     * jest nieprawidłowa.
     *
     * keysym to niezależna od platformy numeryczna reprezentacja klawisza, np.
     * "F1", "a", "b", "L", "5", "Shift" etc. Keycode jest numeryczną
     * reprezentacją klawisza na klawiaturze, wysłaną przez sterownik klawiatury
     * (albo coś w tym stylu - nie jestem ekspertem od hardware'u/sterowników)
     * do serwera X. Nigdy nie chcesz więc hardcodować keycodów, ponieważ mogą 
     * one i będą różnić się między systemami. 
     */
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("F1")), Mod1Mask, root,
            True, GrabModeAsync, GrabModeAsync);

    /* XGrabKey i XGrabButton to podstawowe sposoby powiedzenia "kiedy ta 
     * kombinacja modyfikatorów i klawiszy/przycisków zostanie naciśnięta,
     * wyślij mi te zdarzenia", więc możemy bezpiecznie założyć, że otrzymamy
     * eventy Alt+F1, Alt+Przycisk1, Alt+Przycisk3, ale nie żadne inne. Możesz
     * użyć pojedynczych uchwytów, jak tych do kombinacji klawiszy/myszy lub
     * skorzystać z XSelectInput wraz z KeyPressMask/ButtonPressMask/etc, aby
     * przechwycić wszystkie zdarzenia danego typu i filtrować je w miarę
     * otrzymywania. 
     */
    XGrabButton(dpy, 1, Mod1Mask, root, True, ButtonPressMask, GrabModeAsync,
            GrabModeAsync, None, None);
    XGrabButton(dpy, 3, Mod1Mask, root, True, ButtonPressMask, GrabModeAsync,
            GrabModeAsync, None, None);

    for(;;)
    {
        /* to jest najbardziej podstawowy sposób iterowania po eventach X;
         * możesz być bardziej elastyczny, używając XPending() lub 
         * ConnectionNumber() wraz z select() (lub poll() albo czymkolwiek co
         * ci pasuje). 
         */
        XNextEvent(dpy, &ev);

        /* to jest nasz binding do przywracania okien. Widziałem jak kiedyś na 
         * ratpoison wiki ktoś stwierdził, że to jest głupie, jednak chciałem 
         * gdzieś tu wcisnąć jakieś bindowanie klawiatury i to było najlepsze
         * rozwiązanie. 
         * 
         * przez chwilę byłem nieco zmieszany w kwestii .window vs. .subwindow
         * ale trochę czytania manuali wyjaśniło ją. Nasze uchwyty działają
         * w obrębie okna nadrzędnego, więc gdy interesują nas tylko eventy 
         * dla okien potomnych, patrzymy na .subwindow. Kiedy subwindow jest
         * równe None, oznacza to, że oknem, dla którego przechwycono event
         * jest okno, dla którego stworzyliśmy uchwyt - w tym przypadku, okno
         * nadrzędne. 
         */
        if(ev.type == KeyPress && ev.xkey.subwindow != None)
            XRaiseWindow(dpy, ev.xkey.subwindow);
        else if(ev.type == ButtonPress && ev.xbutton.subwindow != None)
        {
            /* teraz przejmujemy kontrolę nad wskaźnikiem, szukając zdarzeń
             * zwolnienia przycisku oraz ruchu. 
             */ 
            XGrabPointer(dpy, ev.xbutton.subwindow, True,
                    PointerMotionMask|ButtonReleaseMask, GrabModeAsync,
                    GrabModeAsync, None, None, CurrentTime);

            /* "zapamiętujemy" pozycję wskaźnika na początku przesunięcia lub 
             * zmiany rozmiaru oraz rozmiar/pozycję okna. W ten sposób kiedy
             * wskaźnika się przesuwa, możemy porównać go z naszymi danymi
             * początkowymi i odpowiednio dokonać przeniesienia/resize'owania
             */
            XGetWindowAttributes(dpy, ev.xbutton.subwindow, &attr);
            start = ev.xbutton;
        }
        /* jedyny sposób, abyśmy mogli otrzymać zdarzenie powiadamiania 
         * o ruchu, to jeśli dokonaliśmy już przechwycenia wskaźnika oraz 
         * jesteśmy w trybie przenoszenia/resize'owania, więc załóżmy to */
        else if(ev.type == MotionNotify)
        {
            int xdiff, ydiff;

            /* tutaj "kompresujemy" eventy powiadomień o ruchu. Jeśli 10 z nich
             * jest oczekujących, należy patrzyć nie na którykolwiek z nich,
             * tylko na najnowszy. W pewnych sytuacjach - jeśli okno jest 
             * naprawdę duże lub po prostu wszystko dzieje się wolno - 
             * niezastosowanie się do tej zasady może spowodować wiele opóźnień
             * przeciągania ("drag lag")
             *
             * dla menadżerów okien z funkcjonalnościami w stylu przełączania 
             * pulpitów może być użyteczne, aby kompresować eventy EnterNotify,
             * aby nie uzyskać "migotania focusowania" 
             */
            while(XCheckTypedEvent(dpy, MotionNotify, &ev));

            /* teraz używamy tego, co zapamiętaliśmy na początku przesunięcia
             * lub resize'owania i porównujemy z obecną pozycją wskaźnika, aby
             * określić jaki powinien być nowy rozmiar okna lub jego pozycja.
             * 
             * jeśli początkowo naciśniętym przyciskiem był przycisk 1, 
             * to przesuwamy, w przeciwnym przypadku zmieniamy rozmiar
             * 
             * upewniamy się też, że wyliczając wymiary okna nie uzyskaliśmy
             * wartości negatywnych, kończąc z jakimiś śmiesznymi wymiarami
             * typu 65000 pixeli szerokości (często w towarzystwie wielu zamian
             * i spowolnień).
             *
             * jeszcze gorzej jeśli mamy "szczęście" i trafimy na wysokość lub
             * szerokość równe zero, generując błąd serwera X. Ustalamy zatem
             * minimalną wysokość/szerokość okna na 1 pixel. 
             */
            xdiff = ev.xbutton.x_root - start.x_root;
            ydiff = ev.xbutton.y_root - start.y_root;
            XMoveResizeWindow(dpy, ev.xmotion.window,
                attr.x + (start.button==1 ? xdiff : 0),
                attr.y + (start.button==1 ? ydiff : 0),
                MAX(1, attr.width + (start.button==3 ? xdiff : 0)),
                MAX(1, attr.height + (start.button==3 ? ydiff : 0)));
        }
        /* podobnie jak w przypadku powiadomień o ruchu, jedynym przypadkiem
         * otrzymania zwolnienia przycisku jest moment podczas przesuwania albo
         * zmiany rozmiaru dzięki naszemu uchwytowi wskaźnika. To zdarzenie
         * kończy proces przesuwania/resize'owania.
         */
        else if(ev.type == ButtonRelease)
            XUngrabPointer(dpy, CurrentTime);
    }
}
