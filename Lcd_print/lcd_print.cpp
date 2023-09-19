#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <MyLCD.h>
#include <protocol.h>
#include <Arduino.h>


struct DisplayData {
    char topLine[64];
    float value1;
    float value2;
    float value3;
};

struct TextCounter {
    unsigned long startTime;
    int incrementValue;
};

TextCounter textCounter;

void Lcd_inciliation() {
    lcd_init(LCD_DISP_ON_BLINK); // инициализация дисплея
    lcd_home(); // домой курсор
    lcd_led(0); // вкл подсветки
    textCounter.startTime = millis(); // Запоминаем время запуска программы
}

void fillBuffer1(const char* source, char* buffer, size_t bufferSize, int incrementValue) {
    int startIndex = incrementValue % strlen(source); // Определяем начальный индекс на основе incrementValue
    int endIndex = startIndex + 16;

    for (int i = 0; i < 16; ++i) {
        buffer[i] = source[(startIndex + i) % strlen(source)];
    }
    buffer[16] = '\0'; // Установка нулевого символа в конце буфера
}

void fillBuffer2(float value1, float value2, float value3, char* buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize, "%.2f.%.2f.%.2f", value1, value2, value3);
}

void printLcd(const struct message* decode_message) {
    unsigned long currentTime = millis(); // Текущее время
    // Проверяем, прошло ли 500 мс с момента последнего увеличения incrementValue
    if (currentTime - textCounter.startTime >= 500) {
        textCounter.incrementValue++; // Увеличиваем incrementValue на 1
        textCounter.startTime = currentTime; // Обновляем время
    }

    struct DisplayData displayData;
    strncpy(displayData.topLine, decode_message->str, sizeof(displayData.topLine) - 1);
    displayData.topLine[sizeof(displayData.topLine) - 1] = '\0';
    displayData.value1 = decode_message->numbers[0];
    displayData.value2 = decode_message->numbers[1];
    displayData.value3 = decode_message->numbers[2];

    // Буферы для заполнения данных
    char buffer1[17];
    char buffer2[17];

    // Заполнение буфера 1
    fillBuffer1(displayData.topLine, buffer1, sizeof(buffer1), textCounter.incrementValue);

    // Заполнение буфера 2
    fillBuffer2(displayData.value1, displayData.value2, displayData.value3, buffer2, sizeof(buffer2));

    // Создание массива для вывода на дисплей
    char displayArray[32];
    strncpy(displayArray, buffer1, 16); // Копирование первых 16 символов из buffer1 в displayArray
    strncpy(displayArray + 16, buffer2, 16); // Копирование первых 16 символов из buffer2 в displayArray, начиная с позиции 16

    // Вывод данных на экран
    lcd_gotoxy(0, 0);
    lcd_puts(displayArray);

    lcd_gotoxy(0, 1);
    lcd_puts(displayArray + 16); // Вывод второй половины displayArray
}

void dder() {
    Lcd_inciliation();

    uint8_t encode_message[100];
    size_t len_encode_message = 0;
    struct message message;
    

    // Получаем данные из протокола
    protocol_encode(message, encode_message, &len_encode_message);

    // Декодируем данные из протокола
    struct message decode_message;
    protocol_decode(encode_message, len_encode_message, &decode_message);

    // Выводим данные на экран
    printLcd(&decode_message);
}