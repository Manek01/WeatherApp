#include <windows.h>
#include <stdio.h>
#include <string.h>

HWND cityInput;
HWND resultText;

void extractValue(char *data, char *key, char *output) {

    char *pos = strstr(data, key);
    if(pos != NULL) {
        sscanf(pos + strlen(key), "%[^,}]", output);
    }
}

void getWeather() {

    char city[100];
    GetWindowText(cityInput, city, 100);

    char command[400];

    sprintf(command,
    "curl -s \"https://api.weatherapi.com/v1/current.json?key=4f224678d93c46c8ae4164845261703&q=%s\" > weather.json",
    city);

    system(command);

    FILE *file = fopen("weather.json", "r");

    if(!file){
        SetWindowText(resultText,"Error fetching weather");
        return;
    }

    char buffer[10000];
    int size = fread(buffer,1,sizeof(buffer)-1,file);
    buffer[size] = '\0';

    fclose(file);

    char temp[50] = "";
    char humidity[50] = "";
    char wind[50] = "";

    extractValue(buffer,"\"temp_c\":",temp);
    extractValue(buffer,"\"humidity\":",humidity);
    extractValue(buffer,"\"wind_kph\":",wind);

    char result[300];

    sprintf(result,
    "Temperature: %s C\r\nHumidity: %s %%\r\nWind Speed: %s kph",
    temp,humidity,wind);

    SetWindowText(resultText,result);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp){

    switch(msg){

        case WM_CREATE:

        CreateWindow("STATIC","Enter City:",
        WS_VISIBLE | WS_CHILD,
        20,20,100,20,
        hwnd,NULL,NULL,NULL);

        cityInput = CreateWindow("EDIT","",
        WS_VISIBLE | WS_CHILD | WS_BORDER,
        120,20,150,25,
        hwnd,NULL,NULL,NULL);

        CreateWindow("BUTTON","Get Weather",
        WS_VISIBLE | WS_CHILD,
        120,60,120,30,
        hwnd,(HMENU)1,NULL,NULL);

        resultText = CreateWindow("STATIC","Weather Result",
        WS_VISIBLE | WS_CHILD,
        20,120,300,120,
        hwnd,NULL,NULL,NULL);

        break;

        case WM_COMMAND:

        if(LOWORD(wp)==1){
            getWeather();
        }

        break;

        case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd,msg,wp,lp);
}

int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hPrev,LPSTR args,int ncmd){

    WNDCLASS wc = {0};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInst;
    wc.lpszClassName = "WeatherApp";

    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
    "WeatherApp",
    "Weather Application",
    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
    200,200,400,300,
    NULL,NULL,hInst,NULL);

    MSG msg;

    while(GetMessage(&msg,NULL,0,0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}