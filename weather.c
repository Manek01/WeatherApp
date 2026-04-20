#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

int main(){

char city[50];
char buffer[10000];

printf("Enter city name: ");
scanf("%s", city);

/* API Call */
char command[500];

sprintf(command,
"curl -s \"https://api.weatherapi.com/v1/current.json?key=4f224678d93c46c8ae4164845261703&q=%s&aqi=yes\" > weather.json",
city);

system(command);

/* Read JSON File */

FILE *fp = fopen("weather.json","r");
fread(buffer,1,10000,fp);
fclose(fp);

/* Parse JSON */

cJSON *json = cJSON_Parse(buffer);

cJSON *current = cJSON_GetObjectItem(json,"current");

double temp = cJSON_GetObjectItem(current,"temp_c")->valuedouble;
double wind = cJSON_GetObjectItem(current,"wind_kph")->valuedouble;
int humidity = cJSON_GetObjectItem(current,"humidity")->valueint;

/* Print Output */

printf("\n----- Weather Info -----\n");

printf("Temperature : %.1f C\n", temp);
printf("Humidity    : %d %%\n", humidity);
printf("Wind Speed  : %.1f km/h\n", wind);

printf("------------------------\n");

cJSON_Delete(json);

return 0;
}