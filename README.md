# 🌦️ Weather App in C

## 📌 Overview

This is a Weather Application built in **C language** that fetches real-time weather data using an API and displays it in both:

- 🖥️ Console (CLI version)
- 🎨 GUI (SDL2-based modern interface)

The project demonstrates API handling, JSON parsing, and GUI development in C.

---

## 🚀 Features

### 🔹 Console Version
- Enter city name
- Fetch real-time weather
- Displays:
  - 🌡️ Temperature
  - 💧 Humidity
  - 🌬️ Wind Speed

### 🔹 GUI Version (Advanced)
- Modern dashboard UI
- Animated weather effects (rain, snow, etc.)
- Dynamic themes (day/night, condition-based)
- Input box + button interaction

---

## 🛠️ Technologies Used

- C Programming Language
- cJSON Library (for JSON parsing)
- Weather API (weatherapi.com)
- curl (API request)
- SDL2 + SDL2_ttf (GUI)

---

## ⚙️ How It Works

1. User enters a city name
2. Program sends API request using `curl`
3. Response is saved in `weather.json`
4. JSON data is parsed using **cJSON**
5. Data is displayed in console or GUI

---

## 📂 Project Structure
WeatherApp/
│── weather.c # Console version
│── gui_weather.c # GUI version (SDL2)
│── cJSON.c
│── cJSON.h
│── weather.json # API response file
│── weather.exe # Console executable
│── weather_gui.exe # GUI executable

---

## 🎯 Learning Outcomes

- API integration in C  
- JSON parsing using cJSON  
- System command execution  
- GUI programming using SDL2  
- Event handling & animations  

---

## 🚀 Future Improvements

- 📊 Weekly forecast  
- 📍 Auto location detection  
- ⏱️ Hourly timeline  
- 🎨 Better animations & transitions  
- 🌐 Cross-platform packaging  

---

## 👨‍💻 Authors

- Manek Yadav  
- Pankaj Lomror  
- Shiva Kumar Meena  
- Dev Yadav  
- Bhagyashree Jangid  
- Sonakshi  
