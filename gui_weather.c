#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 640
#define INPUT_MAX 80
#define API_KEY "4f224678d93c46c8ae4164845261703"

typedef struct {
    double tempC;
    int humidity;
    double windKph;
    int isDay;
    int conditionCode;
    char condition[80];
    char error[160];
    int valid;
} WeatherData;

typedef enum {
    THEME_CLEAR,
    THEME_CLOUDY,
    THEME_RAIN,
    THEME_STORM,
    THEME_SNOW,
    THEME_MIST,
    THEME_NIGHT
} ThemeKind;

typedef struct {
    ThemeKind kind;
    int isNightVisual;
    SDL_Color top;
    SDL_Color bottom;
    SDL_Color card;
    SDL_Color accent;
    SDL_Color text;
    SDL_Color muted;
} Theme;

static TTF_Font *open_font_any(const char **paths, int count, int size) {
    int i;
    for (i = 0; i < count; i++) {
        TTF_Font *font = TTF_OpenFont(paths[i], size);
        if (font != NULL) {
            return font;
        }
    }
    return NULL;
}

static int contains_ci(const char *haystack, const char *needle) {
    size_t i;
    size_t nlen;
    if (haystack == NULL || needle == NULL) {
        return 0;
    }
    nlen = strlen(needle);
    if (nlen == 0) {
        return 1;
    }
    for (i = 0; haystack[i] != '\0'; i++) {
        size_t j = 0;
        while (needle[j] != '\0' && haystack[i + j] != '\0' &&
               tolower((unsigned char)haystack[i + j]) ==
                   tolower((unsigned char)needle[j])) {
            j++;
        }
        if (j == nlen) {
            return 1;
        }
    }
    return 0;
}

static SDL_Color shade_color(SDL_Color c, float factor) {
    SDL_Color out;
    if (factor < 0.0f) {
        factor = 0.0f;
    }
    out.r = (Uint8)fminf(255.0f, c.r * factor);
    out.g = (Uint8)fminf(255.0f, c.g * factor);
    out.b = (Uint8)fminf(255.0f, c.b * factor);
    out.a = c.a;
    return out;
}

static ThemeKind theme_from_condition_code(int code, const char *conditionText) {
    switch (code) {
        case 1000:
            return THEME_CLEAR;

        case 1001:
            return THEME_CLOUDY;

        case 1002:
            return THEME_MIST;

        case 1003:
            return THEME_RAIN;

        case 1004:
            return THEME_SNOW;

        case 1005:
            return THEME_RAIN;

        case 1006:
            return THEME_STORM;

        default:
            break;
    }

    if (contains_ci(conditionText, "thunder") || contains_ci(conditionText, "storm")) {
        return THEME_STORM;
    }
    if (contains_ci(conditionText, "snow") || contains_ci(conditionText, "blizzard")) {
        return THEME_SNOW;
    }
    if (contains_ci(conditionText, "rain") || contains_ci(conditionText, "drizzle")) {
        return THEME_RAIN;
    }
    if (contains_ci(conditionText, "mist") || contains_ci(conditionText, "fog") ||
        contains_ci(conditionText, "haze")) {
        return THEME_MIST;
    }
    if (contains_ci(conditionText, "cloud") || contains_ci(conditionText, "overcast")) {
        return THEME_CLOUDY;
    }

    return THEME_CLEAR;
}

static Theme choose_theme(const WeatherData *w) {
    Theme t;
    const char *condition = (w->valid ? w->condition : "");
    int isNight = (w->valid && w->isDay == 0);
    ThemeKind mappedKind = w->valid
                               ? theme_from_condition_code(w->conditionCode, condition)
                               : THEME_CLEAR;

    t.kind = mappedKind;
    t.isNightVisual = isNight;
    t.top = (SDL_Color){31, 98, 177, 255};
    t.bottom = (SDL_Color){130, 194, 255, 255};
    t.card = (SDL_Color){20, 35, 62, 210};
    t.accent = (SDL_Color){255, 182, 66, 255};
    t.text = (SDL_Color){241, 247, 255, 255};
    t.muted = (SDL_Color){187, 209, 235, 255};

    if (mappedKind == THEME_STORM) {
        t.kind = THEME_STORM;
        t.top = (SDL_Color){16, 20, 38, 255};
        t.bottom = (SDL_Color){53, 66, 95, 255};
        t.card = (SDL_Color){10, 14, 26, 220};
        t.accent = (SDL_Color){255, 218, 99, 255};
    } else if (mappedKind == THEME_RAIN) {
        t.kind = THEME_RAIN;
        t.top = (SDL_Color){25, 52, 82, 255};
        t.bottom = (SDL_Color){67, 111, 156, 255};
        t.card = (SDL_Color){16, 33, 55, 220};
        t.accent = (SDL_Color){111, 201, 255, 255};
    } else if (mappedKind == THEME_SNOW) {
        t.kind = THEME_SNOW;
        t.top = (SDL_Color){156, 178, 207, 255};
        t.bottom = (SDL_Color){219, 234, 250, 255};
        t.card = (SDL_Color){56, 80, 115, 210};
        t.accent = (SDL_Color){245, 250, 255, 255};
        t.text = (SDL_Color){248, 252, 255, 255};
    } else if (mappedKind == THEME_CLOUDY) {
        t.kind = THEME_CLOUDY;
        t.top = (SDL_Color){91, 109, 130, 255};
        t.bottom = (SDL_Color){154, 171, 189, 255};
        t.card = (SDL_Color){43, 55, 70, 215};
        t.accent = (SDL_Color){205, 221, 235, 255};
    } else if (mappedKind == THEME_MIST) {
        t.kind = THEME_MIST;
        t.top = (SDL_Color){89, 109, 123, 255};
        t.bottom = (SDL_Color){160, 177, 188, 255};
        t.card = (SDL_Color){49, 67, 78, 218};
        t.accent = (SDL_Color){222, 236, 242, 255};
    } else if (contains_ci(condition, "night")) {
        t.kind = THEME_NIGHT;
        t.top = (SDL_Color){12, 18, 48, 255};
        t.bottom = (SDL_Color){39, 56, 104, 255};
        t.card = (SDL_Color){7, 11, 31, 220};
        t.accent = (SDL_Color){247, 222, 132, 255};
    }

    /* WeatherAPI gives local-day info in current.is_day; use it so clear nights are not shown as sunny day themes. */
    if (isNight && t.kind == THEME_CLEAR) {
        t.kind = THEME_NIGHT;
        t.top = (SDL_Color){12, 18, 48, 255};
        t.bottom = (SDL_Color){39, 56, 104, 255};
        t.card = (SDL_Color){7, 11, 31, 220};
        t.accent = (SDL_Color){247, 222, 132, 255};
    }

    if (isNight && t.kind != THEME_NIGHT) {
        t.top = shade_color(t.top, 0.52f);
        t.bottom = shade_color(t.bottom, 0.62f);
        t.card = shade_color(t.card, 0.75f);
        t.accent = shade_color(t.accent, 0.86f);
        t.muted = shade_color(t.muted, 0.90f);
    }

    return t;
}

static void draw_filled_circle(SDL_Renderer *renderer, int cx, int cy, int radius,
                               SDL_Color c) {
    int y;
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    for (y = -radius; y <= radius; y++) {
        int x = (int)sqrt((double)(radius * radius - y * y));
        SDL_RenderDrawLine(renderer, cx - x, cy + y, cx + x, cy + y);
    }
}

static void draw_gradient(SDL_Renderer *renderer, int w, int h, SDL_Color top,
                          SDL_Color bottom) {
    int y;
    for (y = 0; y < h; y++) {
        float k = (float)y / (float)(h - 1);
        Uint8 r = (Uint8)(top.r + (bottom.r - top.r) * k);
        Uint8 g = (Uint8)(top.g + (bottom.g - top.g) * k);
        Uint8 b = (Uint8)(top.b + (bottom.b - top.b) * k);
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderDrawLine(renderer, 0, y, w, y);
    }
}

static void draw_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x,
                      int y, SDL_Color color) {
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect dst;

    if (text == NULL || font == NULL || renderer == NULL || text[0] == '\0') {
        return;
    }

    surface = TTF_RenderUTF8_Blended(font, text, color);
    if (surface == NULL) {
        return;
    }

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        SDL_FreeSurface(surface);
        return;
    }

    dst.x = x;
    dst.y = y;
    dst.w = surface->w;
    dst.h = surface->h;
    SDL_RenderCopy(renderer, texture, NULL, &dst);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

static void draw_card(SDL_Renderer *renderer, TTF_Font *titleFont, TTF_Font *valueFont,
                      SDL_Rect card, const char *title, const char *value,
                      const Theme *theme, float pulse) {
    SDL_Rect inner;
    SDL_Color glow = theme->accent;
    glow.a = (Uint8)(40 + 25 * (0.5f + 0.5f * sinf(pulse)));

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, glow.r, glow.g, glow.b, glow.a);
    SDL_RenderFillRect(renderer, &card);

    inner.x = card.x + 3;
    inner.y = card.y + 3;
    inner.w = card.w - 6;
    inner.h = card.h - 6;

    SDL_SetRenderDrawColor(renderer, theme->card.r, theme->card.g, theme->card.b,
                           theme->card.a);
    SDL_RenderFillRect(renderer, &inner);

    draw_text(renderer, titleFont, title, inner.x + 18, inner.y + 16, theme->muted);
    draw_text(renderer, valueFont, value, inner.x + 18, inner.y + 54, theme->text);
}

static void draw_weather_fx(SDL_Renderer *renderer, const Theme *theme, int w, int h,
                            float timeSec) {
    int i;
    if (theme->kind == THEME_CLEAR || theme->kind == THEME_NIGHT) {
        int cx = w - 120;
        int cy = 90;
        int r = 36 + (int)(3.0f * sinf(timeSec * 2.2f));
        SDL_Color body = theme->accent;
        if (theme->isNightVisual) {
            SDL_Color moonShadow = shade_color(theme->top, 0.7f);
            int sx;
            int sy;
            draw_filled_circle(renderer, cx, cy, r, body);
            draw_filled_circle(renderer, cx + 12, cy - 5, r - 4, moonShadow);
            for (i = 0; i < 36; i++) {
                sx = (int)fmodf((float)(i * 97) + timeSec * 8.0f, (float)w);
                sy = (int)fmodf((float)(i * 53) + timeSec * 3.0f, (float)(h / 2));
                SDL_SetRenderDrawColor(renderer, 240, 242, 255,
                                       (Uint8)(110 + ((i % 4) * 30)));
                SDL_RenderDrawPoint(renderer, sx, sy);
            }
        } else {
            draw_filled_circle(renderer, cx, cy, r, body);
            SDL_SetRenderDrawColor(renderer, body.r, body.g, body.b, 180);
            for (i = 0; i < 10; i++) {
                float a = timeSec * 0.9f + (float)i * 0.628f;
                int x1 = cx + (int)(cosf(a) * (r + 12));
                int y1 = cy + (int)(sinf(a) * (r + 12));
                int x2 = cx + (int)(cosf(a) * (r + 26));
                int y2 = cy + (int)(sinf(a) * (r + 26));
                SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
            }
        }
    }

    if (theme->kind == THEME_CLOUDY || theme->kind == THEME_RAIN ||
        theme->kind == THEME_STORM || theme->kind == THEME_MIST) {
        for (i = 0; i < 4; i++) {
            float drift = fmodf(timeSec * (18.0f + i * 4.0f) + i * 170.0f,
                                (float)(w + 260));
            int x = (int)drift - 220;
            int y = 70 + i * 32;
            SDL_Color c = {225, 235, 245, (Uint8)(90 + i * 12)};
            draw_filled_circle(renderer, x + 40, y + 25, 26, c);
            draw_filled_circle(renderer, x + 72, y + 18, 34, c);
            draw_filled_circle(renderer, x + 108, y + 28, 28, c);
        }
    }

    if (theme->kind == THEME_RAIN || theme->kind == THEME_STORM) {
        SDL_SetRenderDrawColor(renderer, 146, 205, 255, 145);
        for (i = 0; i < 80; i++) {
            int x = (int)fmodf((float)(i * 41) + timeSec * 190.0f, (float)w);
            int y = (int)fmodf((float)(i * 59) + timeSec * 260.0f, (float)h);
            SDL_RenderDrawLine(renderer, x, y, x - 8, y + 17);
        }
    }

    if (theme->kind == THEME_SNOW) {
        SDL_Color flake = {250, 253, 255, 225};
        for (i = 0; i < 90; i++) {
            int x = (int)fmodf((float)(i * 53) + timeSec * (18.0f + (i % 7)), (float)w);
            int y = (int)fmodf((float)(i * 37) + timeSec * (25.0f + (i % 9)), (float)h);
            draw_filled_circle(renderer, x, y, 2 + (i % 2), flake);
        }
    }

    if (theme->kind == THEME_STORM) {
        float flash = sinf(timeSec * 7.0f);
        if (flash > 0.95f) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 220, 180);
            SDL_Rect bolt = {w - 220, 120, 8, 70};
            SDL_RenderFillRect(renderer, &bolt);
            bolt.x -= 20;
            bolt.y += 30;
            bolt.h = 80;
            SDL_RenderFillRect(renderer, &bolt);
        }
    }

    if (theme->kind == THEME_MIST) {
        for (i = 0; i < 6; i++) {
            int y = 120 + i * 70 + (int)(7 * sinf(timeSec * 0.8f + i));
            SDL_Rect band = {0, y, w, 22};
            SDL_SetRenderDrawColor(renderer, 240, 248, 252, 30 + i * 8);
            SDL_RenderFillRect(renderer, &band);
        }
    }
}

static int is_safe_city_char(char c) {
    return isalnum((unsigned char)c) || c == ' ' || c == '-' || c == '.' || c == ',';
}

static void url_encode_city(const char *in, char *out, size_t outSize) {
    size_t i;
    size_t j = 0;
    for (i = 0; in[i] != '\0' && j + 4 < outSize; i++) {
        unsigned char c = (unsigned char)in[i];
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            out[j++] = (char)c;
        } else if (c == ' ') {
            out[j++] = '%';
            out[j++] = '2';
            out[j++] = '0';
        } else {
            snprintf(out + j, outSize - j, "%%%02X", c);
            j += 3;
        }
    }
    out[j] = '\0';
}

static int fetch_weather(const char *city, WeatherData *w) {
    char encodedCity[INPUT_MAX * 3];
    char command[1024];
    char buffer[16000];
    FILE *fp;
    cJSON *json;
    cJSON *current;
    cJSON *temp;
    cJSON *humidity;
    cJSON *wind;
    cJSON *isDay;
    cJSON *conditionObj;
    cJSON *conditionText;
    cJSON *conditionCode;
    size_t i;
    size_t size;

    w->valid = 0;
    w->isDay = 1;
    w->conditionCode = 1000;
    w->error[0] = '\0';

    if (city == NULL || city[0] == '\0') {
        snprintf(w->error, sizeof(w->error), "Please enter a city name.");
        return 0;
    }

    for (i = 0; city[i] != '\0'; i++) {
        if (!is_safe_city_char(city[i])) {
            snprintf(w->error, sizeof(w->error),
                     "City contains unsupported characters.");
            return 0;
        }
    }

    url_encode_city(city, encodedCity, sizeof(encodedCity));

    snprintf(command, sizeof(command),
             "curl -s \"https://api.weatherapi.com/v1/current.json?key=%s&q=%s&aqi=yes\" > weather.json",
             API_KEY, encodedCity);

    if (system(command) != 0) {
        snprintf(w->error, sizeof(w->error), "Failed to call API via curl.");
        return 0;
    }

    fp = fopen("weather.json", "r");
    if (fp == NULL) {
        snprintf(w->error, sizeof(w->error), "Could not open weather.json.");
        return 0;
    }

    size = fread(buffer, 1, sizeof(buffer) - 1, fp);
    fclose(fp);
    buffer[size] = '\0';

    json = cJSON_Parse(buffer);
    if (json == NULL) {
        snprintf(w->error, sizeof(w->error), "JSON parse failed.");
        return 0;
    }

    current = cJSON_GetObjectItemCaseSensitive(json, "current");
    temp = current ? cJSON_GetObjectItemCaseSensitive(current, "temp_c") : NULL;
    humidity = current ? cJSON_GetObjectItemCaseSensitive(current, "humidity") : NULL;
    wind = current ? cJSON_GetObjectItemCaseSensitive(current, "wind_kph") : NULL;
    isDay = current ? cJSON_GetObjectItemCaseSensitive(current, "is_day") : NULL;
    conditionObj = current ? cJSON_GetObjectItemCaseSensitive(current, "condition") : NULL;
    conditionText =
        conditionObj ? cJSON_GetObjectItemCaseSensitive(conditionObj, "text") : NULL;
    conditionCode =
        conditionObj ? cJSON_GetObjectItemCaseSensitive(conditionObj, "code") : NULL;

    if (!cJSON_IsNumber(temp) || !cJSON_IsNumber(humidity) || !cJSON_IsNumber(wind) ||
        !cJSON_IsNumber(isDay) || !cJSON_IsString(conditionText) ||
        !cJSON_IsNumber(conditionCode)) {
        cJSON_Delete(json);
        snprintf(w->error, sizeof(w->error), "Unexpected API response format.");
        return 0;
    }

    w->tempC = temp->valuedouble;
    w->humidity = humidity->valueint;
    w->windKph = wind->valuedouble;
    w->isDay = isDay->valueint;
    w->conditionCode = conditionCode->valueint;
    snprintf(w->condition, sizeof(w->condition), "%s", conditionText->valuestring);
    w->valid = 1;

    cJSON_Delete(json);
    return 1;
}

static void draw_input_bar(SDL_Renderer *renderer, TTF_Font *font, int w, char *city,
                           int focused, const Theme *theme) {
    SDL_Rect inputRect = {40, 26, w - 230, 48};
    SDL_Rect buttonRect = {w - 170, 26, 130, 48};
    SDL_Color inBorder = focused ? theme->accent : theme->muted;
    SDL_Color btn = theme->accent;
    SDL_Color btnText = {16, 24, 35, 255};
    char cityLabel[120];
    SDL_Color cityColor = theme->text;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 10, 16, 28, 120);
    SDL_RenderFillRect(renderer, &inputRect);
    SDL_SetRenderDrawColor(renderer, inBorder.r, inBorder.g, inBorder.b, 225);
    SDL_RenderDrawRect(renderer, &inputRect);

    SDL_SetRenderDrawColor(renderer, btn.r, btn.g, btn.b, 250);
    SDL_RenderFillRect(renderer, &buttonRect);

    draw_text(renderer, font, "Fetch", buttonRect.x + 38, buttonRect.y + 13, btnText);

    if (city[0] == '\0') {
        snprintf(cityLabel, sizeof(cityLabel), "City: click here and type");
        cityColor = theme->muted;
    } else {
        snprintf(cityLabel, sizeof(cityLabel), "City: %s", city);
    }
    draw_text(renderer, font, cityLabel, inputRect.x + 12, inputRect.y + 14, cityColor);
}

int main(void) {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *fontSmall;
    TTF_Font *fontMedium;
    TTF_Font *fontLarge;
    WeatherData weather = {0};
    Theme theme;
    SDL_Event event;
    int running = 1;
    int inputFocused = 1;
    char city[INPUT_MAX] = "";
    int cityLen = (int)strlen(city);
    Uint32 now;
    float t;
    int w;
    int h;
    SDL_Rect cards[3];
    char tempText[64];
    char humidityText[64];
    char windText[64];
    const char *regularPaths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "C:/Windows/Fonts/arial.ttf"
    };
    const char *boldPaths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf",
        "C:/Windows/Fonts/arialbd.ttf"
    };

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    window = SDL_CreateWindow("Weather Dashboard",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH,
                              WINDOW_HEIGHT,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    fontSmall = open_font_any(regularPaths, 3, 22);
    fontMedium = open_font_any(regularPaths, 3, 28);
    fontLarge = open_font_any(boldPaths, 3, 48);
    if (fontSmall == NULL || fontMedium == NULL || fontLarge == NULL) {
        fprintf(stderr, "Font load failed. Install DejaVu Sans fonts.\n");
        if (fontSmall != NULL) {
            TTF_CloseFont(fontSmall);
        }
        if (fontMedium != NULL) {
            TTF_CloseFont(fontMedium);
        }
        if (fontLarge != NULL) {
            TTF_CloseFont(fontLarge);
        }
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    fetch_weather(city, &weather);
    SDL_StartTextInput();

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mx = event.button.x;
                int my = event.button.y;
                SDL_GetWindowSize(window, &w, &h);
                if (mx >= w - 170 && mx <= w - 40 && my >= 26 && my <= 74) {
                    fetch_weather(city, &weather);
                } else {
                    inputFocused = (mx >= 40 && mx <= w - 190 && my >= 26 && my <= 74);
                    if (inputFocused && event.button.clicks >= 2) {
                        city[0] = '\0';
                        cityLen = 0;
                    }
                }
            } else if (event.type == SDL_TEXTINPUT && inputFocused) {
                if (cityLen < INPUT_MAX - 1) {
                    size_t available = (size_t)(INPUT_MAX - 1 - cityLen);
                    strncat(city, event.text.text, available);
                    cityLen = (int)strlen(city);
                }
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                    fetch_weather(city, &weather);
                } else if (event.key.keysym.sym == SDLK_BACKSPACE && cityLen > 0 && inputFocused) {
                    city[cityLen - 1] = '\0';
                    cityLen--;
                } else if (event.key.keysym.sym == SDLK_ESCAPE && inputFocused) {
                    city[0] = '\0';
                    cityLen = 0;
                } else if (event.key.keysym.sym == SDLK_l && inputFocused &&
                           (event.key.keysym.mod & KMOD_CTRL)) {
                    city[0] = '\0';
                    cityLen = 0;
                }
            }
        }

        SDL_GetWindowSize(window, &w, &h);
        now = SDL_GetTicks();
        t = (float)now / 1000.0f;

        theme = choose_theme(&weather);

        draw_gradient(renderer, w, h, theme.top, theme.bottom);
        draw_weather_fx(renderer, &theme, w, h, t);
        draw_input_bar(renderer, fontSmall, w, city, inputFocused, &theme);

        draw_text(renderer, fontLarge,
                  weather.valid ? "Live Weather" : "Weather Unavailable",
                  42, 98, theme.text);

        if (weather.valid) {
            char conditionLine[120];
            snprintf(conditionLine, sizeof(conditionLine), "Condition: %s", weather.condition);
            draw_text(renderer, fontMedium, conditionLine, 44, 162, theme.muted);
        } else {
            draw_text(renderer, fontMedium,
                      weather.error[0] != '\0' ? weather.error : "Waiting for weather data",
                      44, 162, theme.muted);
        }

        cards[0] = (SDL_Rect){44, 240, (w - 108) / 3, 210};
        cards[1] = (SDL_Rect){cards[0].x + cards[0].w + 10, 240, (w - 108) / 3, 210};
        cards[2] = (SDL_Rect){cards[1].x + cards[1].w + 10, 240, (w - 108) / 3, 210};

        if (weather.valid) {
            snprintf(tempText, sizeof(tempText), "%.1f C", weather.tempC);
            snprintf(humidityText, sizeof(humidityText), "%d %%", weather.humidity);
            snprintf(windText, sizeof(windText), "%.1f km/h", weather.windKph);
        } else {
            snprintf(tempText, sizeof(tempText), "--");
            snprintf(humidityText, sizeof(humidityText), "--");
            snprintf(windText, sizeof(windText), "--");
        }

        draw_card(renderer, fontSmall, fontLarge, cards[0], "Temperature", tempText, &theme,
                  t * 2.0f + 0.3f);
        draw_card(renderer, fontSmall, fontLarge, cards[1], "Humidity", humidityText, &theme,
                  t * 2.0f + 1.7f);
        draw_card(renderer, fontSmall, fontLarge, cards[2], "Wind", windText, &theme,
                  t * 2.0f + 3.1f);

        draw_text(renderer, fontSmall,
                  "Tip: Click city box and type. Enter/Fetch to load. Esc or Ctrl+L clears.",
                  44, h - 42, theme.muted);

        SDL_RenderPresent(renderer);
    }

    SDL_StopTextInput();
    TTF_CloseFont(fontSmall);
    TTF_CloseFont(fontMedium);
    TTF_CloseFont(fontLarge);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
