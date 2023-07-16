// windatp4.cpp : Definiuje punkt wejścia dla aplikacji.
//

#include "framework.h"
#include "windatp4.h"

#define MAX_LOADSTRING 100
const int DLUGOSC_PIETRA = 105; //długość pomiędzy pietrami;
const int CZAS = 33;
const int ODLEGLOSC_POMIEDZY_OSOBAMI = 25;
const int PREDKOSC = 5;
const int PARTER = 525;
const int ILOSC_PIETER = 5;
const int MAX_WAGA = 600;
const int DRZWI_LIMIT = 80;
const enum STANY_OSOBY { OSOBA_STOP, OSOBA_KOLEJKA, OSOBA_W_WINDZIE, OSOBA_RUCH, OSOBA_PO_WINDZIE, OSOBA_USUN };
const enum STANY_WINDY { WINDA_IDLE, WINDA_STOP, WINDA_DRZWI, WINDA_RUCH };
const int PRZYCISKI_ID[20]{ 12,13,14,15,21,23,24,25,31,32,34,35,41,42,43,45,51,52,53,54 };

class Osoba {
public:
    int x;
    int y;
    int waga;
    int cel;
    int goal_x;
    char kierunek;
    STANY_OSOBY stan;
    Osoba(int x, int y, int cel) {
        this->x = x;
        this->y = y;
        this->cel = cel;
        kierunek = 's'; //s - stop g- gora  d - dol l - lewo p - prawow
        waga = 70;
        stan = OSOBA_STOP;
        goal_x = x;
    }
    void ObierzCel(int goal_x) {
        this->goal_x = goal_x;
    }
    void UstawKierunek(char kierunek) {
        this->kierunek = kierunek;
    }
    void Ruch() {
        switch (kierunek)
        {
        case 'g':
            y -= PREDKOSC;
            break;
        case 'd':
            y += PREDKOSC;
            break;
        case 'l':
            x -= PREDKOSC;
            if (goal_x >= x) {
                if (stan == OSOBA_PO_WINDZIE) stan = OSOBA_USUN;
                else stan = OSOBA_W_WINDZIE;
                kierunek = 's';
            }
            break;
        case 'p':
            x += PREDKOSC;

            if (goal_x <= x) {
                if (stan == OSOBA_PO_WINDZIE) stan = OSOBA_USUN;
                else stan = OSOBA_W_WINDZIE;
                kierunek = 's';
            }
            break;
        case 's': return;
        default:
            break;
        }
    }
};
class Winda {
public:
    int y;
    int pietro;
    int cel;
    int waga;
    int pozycja_drzwi;
    bool kierunek_gora;
    int ostatnia_aktywnosc;
    STANY_WINDY stan;
    vector<int> kolejka;

    vector<Osoba> osobywwindzie;
    vector<Osoba>osobynapietrach[5];
    Winda() {
        this->y = PARTER;
        this->pietro = 0;
        this->cel = 0;
        this->stan = WINDA_IDLE;
        this->waga = 0;
        this->pozycja_drzwi = 0;
        this->kierunek_gora = true;
        this->ostatnia_aktywnosc = 0;

    }
    void NastepnaAkcja() {
        switch (stan) {
        case WINDA_STOP:
            StopWindy();
            break;
        case WINDA_RUCH:
            RuchWindy();
            break;
        case WINDA_DRZWI:
            DrzwiWindy();
            break;
        case WINDA_IDLE:
            break;
        }
    }
    void RuchWindy() {
        if (y > PodajWysokoscPietra(cel)) {
            KierunekPasazerow('g');
            y -= PREDKOSC;
        }
        else if (y < PodajWysokoscPietra(cel)) {
            KierunekPasazerow('d');
            y += PREDKOSC;
        }
        else {
            pietro = cel;
            stan = WINDA_DRZWI;
        }

    }
    int PodajWysokoscPietra(int pietro) {
        return 525 - 105 * (pietro);
    }
    void KierunekPasazerow(char k) {
        for (auto& osoba : osobywwindzie) {
            osoba.UstawKierunek(k);
        }
    }
    void DodajDoKolejki(int przystanek) {
        kolejka.push_back(przystanek);
        SortujKolejke();
    }
    void SortujKolejke() {
        //algorytm SCAN https://www.geeksforgeeks.org/scan-elevator-disk-scheduling-algorithms/
        vector<int> dol, gora;
        int head = pietro;
        bool check = false;

        for (auto req : kolejka) {
            if (req < head)
                dol.push_back(req);
            if (req > head)
                gora.push_back(req);
            if (req == head) {
                check = true;
            }
        }

        // sorting left and right vectors
        std::sort(dol.begin(), dol.end());
        std::sort(gora.begin(), gora.end());

        // run the while loop two times.
        // one by one scanning right
        // and left of the head
        kolejka.clear();
        int run = 2;
        while (run--) {
            if (!kierunek_gora) {
                for (int i = dol.size() - 1; i >= 0; i--) {
                    int cur_track = dol[i];
                    // appending current track to seek sequence
                    kolejka.push_back(cur_track);
                    // accessed track is now the new head
                    head = cur_track;
                }
                kierunek_gora = true;
            }
            else if (kierunek_gora) {
                for (int i = 0; i < gora.size(); i++) {
                    int cur_track = gora[i];
                    // appending current track to seek sequence
                    kolejka.push_back(cur_track);
                    // accessed track is now new head
                    head = cur_track;
                }
                kierunek_gora = false;
            }
        }
        //priorytetowy req dla osob na tym samym pietrze co winda
        if (check) kolejka.insert(kolejka.begin(), pietro);
    }
    void StopWindy() {
        //inicjalizator ruchu dla pasazerow wchodzących
        if (waga >= MAX_WAGA) return;
        for (int kolejka_osob = 0; kolejka_osob < osobynapietrach[pietro].size(); kolejka_osob++) {
            Osoba& osoba = osobynapietrach[pietro][kolejka_osob];
            if (osoba.kierunek == 's' && osoba.stan == OSOBA_STOP) {
                if (pietro % 2) {
                    osoba.UstawKierunek('l');
                    osoba.ObierzCel(osoba.x - 250 + osobywwindzie.size() * 10);
                }
                else {
                    osoba.UstawKierunek('p');
                    osoba.ObierzCel(osoba.x + 250 - osobywwindzie.size() * 10);
                }
                osoba.stan = OSOBA_KOLEJKA;
            }
            if (osoba.stan == OSOBA_W_WINDZIE) {
                waga += osoba.waga;
                DodajDoKolejki(osoba.cel);
                osobywwindzie.push_back(osoba);
            }

        }
        //usuwanie osob wchodzących do windy i osob które wyszły z pietra Erase-Idiom Algorithm
        osobynapietrach[pietro].erase(std::remove_if(osobynapietrach[pietro].begin(), osobynapietrach[pietro].end(), [](Osoba const& p) {return p.stan == OSOBA_W_WINDZIE || p.stan == OSOBA_USUN; }), osobynapietrach[pietro].end());

        //osoby wychodzace z windy
        for (auto& osoba : osobywwindzie) {
            if (osoba.cel == pietro) {
                if (pietro % 2) {
                    osoba.UstawKierunek('p');
                    osoba.ObierzCel(720);
                }
                else {
                    osoba.UstawKierunek('l');
                    osoba.ObierzCel(0);
                }
                waga -= osoba.waga;
                osoba.stan = OSOBA_PO_WINDZIE;
                osobynapietrach[pietro].push_back(osoba);
            }
        }
        //usuwanie osob wychodzaczych z windy
        osobywwindzie.erase(std::remove_if(osobywwindzie.begin(), osobywwindzie.end(), [&](Osoba const& p) {return p.stan == OSOBA_PO_WINDZIE; }), osobywwindzie.end());
        if (waga >= MAX_WAGA) return;
        if (osobynapietrach[pietro].empty()) stan = WINDA_DRZWI;
        return;
    }
    void KierunekWindy() {
        if (kierunek_gora) {
            int max = *max_element(kolejka.begin(), kolejka.end());
            if (max == pietro) kierunek_gora = false;
        }
        else {
            int min = *min_element(kolejka.begin(), kolejka.end());
            if (min == pietro) kierunek_gora = true;
        }
    }
    void DrzwiWindy() {
        KierunekPasazerow('s');
        bool check = false;
        for (auto& i : osobywwindzie) {
            if (i.cel == pietro) check = true;
        }
        //drzwi sie otwieraja
        if (!osobynapietrach[pietro].empty() || check) {
            if (pozycja_drzwi >= 80) {
                stan = WINDA_STOP;
            }
            else pozycja_drzwi += PREDKOSC;
        }
        else {
            //drzwi sie zamykaja
            if (pozycja_drzwi <= 0) {
                KierunekWindy();
                stan = WINDA_RUCH;

                if (!kolejka.empty()) {
                    kolejka.erase(kolejka.begin());
                    if (!kolejka.empty()) {

                        cel = kolejka.front();
                    }
                    else {
                        stan = WINDA_IDLE;
                    }
                }
            }
            else pozycja_drzwi -= PREDKOSC;
        }

    }
};
// Zmienne globalne:
HINSTANCE hInst;                                // bieżące wystąpienie
WCHAR szTitle[MAX_LOADSTRING];                  // Tekst paska tytułu
WCHAR szWindowClass[MAX_LOADSTRING];            // nazwa klasy okna głównego

// Przekaż dalej deklaracje funkcji dołączone w tym module kodu:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: W tym miejscu umieść kod.
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    // Inicjuj ciągi globalne
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDATP4, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Wykonaj inicjowanie aplikacji:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDATP4));

    MSG msg;

    // Główna pętla komunikatów:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNKCJA: MyRegisterClass()
//
//  PRZEZNACZENIE: Rejestruje klasę okna.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDATP4));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDATP4);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

void Drawing(HDC hdc) {

    Graphics graphics(hdc);
    Pen pen(Color(255, 0, 255, 0), 3);
    Pen pen1(Color(255, 0, 0, 255), 5);
    Pen pen2(Color(255, 255, 0, 255), 3);
    Pen body(Color(255, 217, 153, 75), 5);

    for (int i = 0; i < 5; i++) {
        if (i % 2) {
            graphics.DrawLine(&pen, 480, DLUGOSC_PIETRA * (i + 1), 720, DLUGOSC_PIETRA * (i + 1));
        }
        else {
            graphics.DrawLine(&pen, 10, DLUGOSC_PIETRA * (i + 1), 230, DLUGOSC_PIETRA * (i + 1));
        }
    }

    
    //sciany stale
    graphics.DrawLine(&pen1, 480, 10, 480, 105);
    graphics.DrawLine(&pen1, 480, 210, 480, 315);
    graphics.DrawLine(&pen1, 480, 420, 480, 527);

    graphics.DrawLine(&pen1, 230, 105, 230, 210);
    graphics.DrawLine(&pen1, 230, 315, 230, 420);
    graphics.DrawLine(&pen1, 228, 528, 483, 528);

    
}

//
//   FUNKCJA: InitInstance(HINSTANCE, int)
//
//   PRZEZNACZENIE: Zapisuje dojście wystąpienia i tworzy okno główne
//
//   KOMENTARZE:
//
//        W tej funkcji dojście wystąpienia jest zapisywane w zmiennej globalnej i
//        jest tworzone i wyświetlane okno główne programu.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Przechowuj dojście wystąpienia w naszej zmiennej globalnej

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    //pietro 5
    HWND hwndButton54 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"4",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        10,         // x position 
        10,         // y position 
        20,        // Button width
        20,        // Button height
        hWnd,     // Parent window
        (HMENU)54,       // No menu. 
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);      // Pointer not needed.

    HWND hwndButton53 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"3",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        10,         // x position 
        10 + 20,         // y position 
        20,        // Button width
        20,        // Button height
        hWnd,     // Parent window
        (HMENU)53,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);      // Pointer not needed.

    HWND hwndButton52 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"2",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        10,         // x position 
        10 + 40,         // y position 
        20,        // Button width
        20,        // Button height
        hWnd,     // Parent window
        (HMENU)52,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);      // Pointer not needed.

    HWND hwndButton51 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"1",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        10,         // x position 
        10 + 60,         // y position 
        20,        // Button width
        20,        // Button height
        hWnd,     // Parent window
        (HMENU)51,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);      // Pointer not needed.
    //pietro 4
    int x = 700, y = 120;
    HWND hwndButton45 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"5",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        x,         // x position 
        y,         // y position 
        20,        // Button width
        20,        // Button height
        hWnd,     // Parent window
        (HMENU)45,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);      // Pointer not needed.

    HWND hwndButton43 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"3",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        x,         // x position 
        y + 20,         // y position 
        20,        // Button width
        20,        // Button height
        hWnd,     // Parent window
        (HMENU)43,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);      // Pointer not needed.

    HWND hwndButton42 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"2",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        x,         // x position 
        y + 40,         // y position 
        20,        // Button width
        20,        // Button height
        hWnd,     // Parent window
        (HMENU)42,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);      // Pointer not needed.

    HWND hwndButton41 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"1",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        x,         // x position 
        y + 60,         // y position 
        20,        // Button width
        20,        // Button height
        hWnd,     // Parent window
        (HMENU)41,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);      // Pointer not needed.
    //pietro 3
    int x1 = 10, y1 = y + 60 + 30;
    HWND hwndButton35 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"5",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        x1,         // x position 
        y1,         // y position 
        20,        // Button width
        20,        // Button height
        hWnd,     // Parent window
        (HMENU)35,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);      // Pointer not needed.

    HWND hwndButton34 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"4",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        x1,         // x position 
        y1 + 20,         // y position 
        20,        // Button width
        20,        // Button height
        hWnd,     // Parent window
        (HMENU)34,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);      // Pointer not needed.

    HWND hwndButton32 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"2",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        x1,         // x position 
        y1 + 40,         // y position 
        20,        // Button width
        20,        // Button height
        hWnd,     // Parent window
        (HMENU)32,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);      // Pointer not needed.

    HWND hwndButton31 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"1",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        x1,         // x position 
        y1 + 60,         // y position 
        20,        // Button width
        20,        // Button height
        hWnd,     // Parent window
        (HMENU)31,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);      // Pointer not needed.

    //pietro 2
    int x2 = 700, y2 = y1 + 60 + 30;
    HWND hwndButton25 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"5",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        x2,         // x position 
        y2,         // y position 
        20,        // Button width
        20,        // Button height
        hWnd,     // Parent window
        (HMENU)25,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);      // Pointer not needed.

    HWND hwndButton24 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"4",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        x2,         // x position 
        y2 + 20,         // y position 
        20,        // Button width
        20,        // Button height
        hWnd,     // Parent window
        (HMENU)24,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);      // Pointer not needed.

    HWND hwndButton23 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"3",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        x2,         // x position 
        y2 + 40,         // y position 
        20,        // Button width
        20,        // Button height
        hWnd,     // Parent window
        (HMENU)23,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);      // Pointer not needed.

    HWND hwndButton21 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"1",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        x2,         // x position 
        y2 + 60,         // y position 
        20,        // Button width
        20,        // Button height
        hWnd,     // Parent window
        (HMENU)21,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);      // Pointer not needed.
    //pietro 1
    int x3 = 10, y3 = y2 + 60 + 30;
    HWND hwndButton15 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"5",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        x3,         // x position 
        y3,         // y position 
        20,        // Button width
        20,        // Button height
        hWnd,     // Parent window
        (HMENU)15,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);      // Pointer not needed.

    HWND hwndButton14 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"4",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        x3,         // x position 
        y3 + 20,         // y position 
        20,        // Button width
        20,        // Button height
        hWnd,     // Parent window
        (HMENU)14,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);      // Pointer not needed.

    HWND hwndButton13 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"3",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        x3,         // x position 
        y3 + 40,         // y position 
        20,        // Button width
        20,        // Button height
        hWnd,     // Parent window
        (HMENU)13,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);      // Pointer not needed.

    HWND hwndButton12 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"2",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        x3,         // x position 
        y3 + 60,         // y position 
        20,        // Button width
        20,        // Button height
        hWnd,     // Parent window
        (HMENU)12,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);      // Pointer not needed.

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  FUNKCJA: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PRZEZNACZENIE: Przetwarza komunikaty dla okna głównego.
//
//  WM_COMMAND  - przetwarzaj menu aplikacji
//  WM_PAINT    - Maluj okno główne
//  WM_DESTROY  - opublikuj komunikat o wyjściu i wróć
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Analizuj zaznaczenia menu:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Tutaj dodaj kod rysujący używający elementu hdc...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Procedura obsługi komunikatów dla okna informacji o programie.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
