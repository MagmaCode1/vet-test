#include <Windows.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <random>
#include <string>
#include <vector>
bool running = true;
HDC hdc = NULL;
HWND hwnd = NULL;
HDC offscreenHDC = CreateCompatibleDC(hdc);
HBITMAP hBitmapCompatible;
RECT clrect;
float width;
float height;
POINT head;
int tailmap[41][41] = {0};
int length = 4;
float scale = 1;
int direction_x = 0;
int direction_y = 0;
bool moveing = false;
POINT apple;
void updateConstants() {
    GetClientRect(hwnd, &clrect);
    width = clrect.right - clrect.left;
    height = clrect.bottom - clrect.top;
    if (height < width)
        scale = height / 410;
    else
        scale = width / 410;
    hBitmapCompatible = CreateCompatibleBitmap(hdc, width, height);
}
void edraw() {
    SelectObject(offscreenHDC, hBitmapCompatible);
    HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
    HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
    HBRUSH greyBrush = CreateSolidBrush(RGB(155, 155, 155));
    HBRUSH redBrush = CreateSolidBrush(RGB(255, 0, 0));
    SelectObject(offscreenHDC, greyBrush);
    Rectangle(offscreenHDC, clrect.left, clrect.top, clrect.right, clrect.bottom);
    SelectObject(offscreenHDC, blackBrush);
    Rectangle(offscreenHDC, 0 * scale, 0 * scale, 410 * scale, 410 * scale);
    SelectObject(offscreenHDC, whiteBrush);
    //draw tail
    for (int x = 40; x >= 0; x--) {
        for (int y = 40; y >= 0; y--) {
            if (tailmap[x][y] > 0) {
                Rectangle(offscreenHDC, (x * 10) * scale, (y * 10) * scale, (x * 10 + 10) * scale, (y * 10 + 10) * scale);
                tailmap[x][y]--;
            }
        }
    }
    //draw head
    SelectObject(offscreenHDC, greyBrush);
    Rectangle(offscreenHDC, (head.x * 10) * scale, (head.y * 10) * scale, (head.x * 10 + 10) * scale, (head.y * 10 + 10) * scale);
    SelectObject(offscreenHDC, redBrush);
    Rectangle(offscreenHDC, (apple.x * 10) * scale, (apple.y * 10) * scale, (apple.x * 10 + 10) * scale, (apple.y * 10 + 10) * scale);
    BitBlt(hdc, 0, 0, width, height, offscreenHDC, 0, 0, SRCCOPY);
    DeleteObject(blackBrush);
    DeleteObject(greyBrush);
    DeleteObject(redBrush);
    DeleteObject(whiteBrush);
}
POINT randomPoint() {
    std::random_device seed;
    std::mt19937 gen(seed());
    std::uniform_int_distribution<>distr(0, 40);
    POINT out;
    out.x = distr(gen);
    out.y = distr(gen);
    return out;
}
void kill() {
    head = randomPoint();
    apple = randomPoint();
    moveing = false;
    direction_x = 0;
    direction_y = 0;
    length = 4;
    for (int x = 40; x >= 0; x--) {
        for (int y = 40; y >= 0; y--) {
            if (tailmap[x][y] > 0) {
                tailmap[x][y] = 0;
            }
        }
    }
    Sleep(1000);
}
void checkColisions(){
    if (head.x == apple.x && head.y == apple.y){
        length++;
        apple = randomPoint();
    }
    if (tailmap[head.x][head.y] > 0)
        kill();
    if (head.x < 0)
        kill();
    if (head.y < 0)
        kill();
    if (head.x > 40)
        kill();
    if (head.y > 40)
        kill();
}
void mainloop() {
    updateConstants();
    head = randomPoint();
    apple = randomPoint();
    while (running == true) {
        MSG msg = { 0 };
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (GetAsyncKeyState('W') & 0x8000 && direction_y != 1) {
            direction_y = -1;
            direction_x = 0;
            moveing = true;
        }
        else {
            if (GetAsyncKeyState('S') & 0x8000 && direction_y != -1) {
                direction_y = 1;
                direction_x = 0;
                moveing = true;
            }
            else {
                if (GetAsyncKeyState('A') & 0x8000 && direction_x != 1) {
                    direction_x = -1;
                    direction_y = 0;
                    moveing = true;
                }
                else {
                    if (GetAsyncKeyState('D') & 0x8000 && direction_x != -1) {
                        direction_x = 1;
                        direction_y = 0;
                        moveing = true;
                    }
                }
            }   
        }
        if (moveing == true)
            tailmap[head.x][head.y] = length;
        head.x = head.x + direction_x;
        head.y = head.y + direction_y;
        checkColisions();
        updateConstants();
        edraw();
        Sleep(100);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const TCHAR* CLASS_NAME = TEXT("My Window Class");
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);
    hwnd = CreateWindow(CLASS_NAME, TEXT("My Window"), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 425, 450, NULL, NULL, hInstance, NULL);
    if (hwnd == NULL) {
        return 0;
    }
    hdc = GetDC(hwnd);
    ShowWindow(hwnd, nCmdShow);
    mainloop();
    return 0;
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_DESTROY:
        running = false;
        ReleaseDC(hwnd, hdc);
        DeleteDC(offscreenHDC);
        ReleaseDC(hwnd, offscreenHDC);
        DeleteObject(hBitmapCompatible);
        PostQuitMessage(0);
        break;
    case WM_SIZE:
        updateConstants();
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}