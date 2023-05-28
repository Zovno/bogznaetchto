#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define CACHE_SIZE 10

typedef struct Node {
    int value;
    int priority;
    struct Node* prev;
    struct Node* next;
} Node;

// Структура для представления очереди с приоритетами
typedef struct {
    Node* front;
    Node* rear;
} TimeQueue;


typedef struct {
    char domain[100];
    char ip[16];
} CacheEntry;

typedef struct {
    CacheEntry hash[500];
    TimeQueue* timeQueue;
    int count;
} Cache;

/*------------------------------------------------------*/

// Функция для создания нового элемента очереди
Node* createNode(int value, int priority) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->value = value;
    newNode->priority = priority;
    newNode->prev = NULL;
    newNode->next = NULL;
    return newNode;
}

// Функция для создания новой очереди с приоритетами
TimeQueue* createTimeQueue() {
    TimeQueue* newTimeQueue = (TimeQueue*)malloc(sizeof(TimeQueue));
    newTimeQueue->front = NULL;
    newTimeQueue->rear = NULL;
    return newTimeQueue;
}

// Функция для добавления элемента в очередь с приоритетами
void add(TimeQueue* timeQueue, int value, int priority) {
    Node* newNode = createNode(value, priority);

    if (timeQueue->front == NULL) {
        // Если очередь пустая
        timeQueue->front = newNode;
        timeQueue->rear = newNode;
    }
    else if (priority < timeQueue->front->priority) {
        // Если новый элемент имеет наименьший приоритет
        newNode->next = timeQueue->front;
        timeQueue->front->prev = newNode;
        timeQueue->front = newNode;
    }
    else {
        Node* current = timeQueue->front;
        while (current->next != NULL && current->next->priority <= priority) {
            current = current->next;
        }
        newNode->next = current->next;
        newNode->prev = current;
        if (current->next != NULL) {
            current->next->prev = newNode;
        }
        else {
            timeQueue->rear = newNode;
        }
        current->next = newNode;
    }
}

// Функция для извлечения элемента с наименьшим приоритетом из очереди
int extractMinValue(TimeQueue* timeQueue) {
    if (timeQueue->front == NULL) {
        printf("Очередь пуста.\n");
        return NULL;
    }

    Node* minNode = timeQueue->front;
    int minValue = minNode->value;

    timeQueue->front = timeQueue->front->next;
    if (timeQueue->front != NULL) {
        timeQueue->front->prev = NULL;
    }
    else {
        timeQueue->rear = NULL;
    }

    free(minNode);
    return minValue;
}

void increasePriority(TimeQueue* timeQueue, int key) {
    Node* current = timeQueue->front;
    while (current != NULL) {
        if (current->value == key)
        {
            int an = current->priority;
            // Удаляем элемент из текущей позиции
            if (current->prev != NULL) {
                current->prev->next = current->next;
            }
            else {
                timeQueue->front = current->next;
            }
            if (current->next != NULL) {
                current->next->prev = current->prev;
            }
            else {
                timeQueue->rear = current->prev;
            }
            // Добавляем элемент в новую позицию с новым приоритетом
            add(timeQueue, current->value, ++an);
            free(current);
            return;
        }
        current = current->next;
    }
    printf("err.\n");
}

void printTimeQueue(TimeQueue* timeQueue) {
    Node* current = timeQueue->front;
    while (current != NULL) {
        printf("Value: %d, Priority: %d\n", current->value, current->priority);
        current = current->next;
    }
}

/*-----------------------конец-----------------------------------*/


unsigned int complexHash(const char* str) {
    unsigned int hash = 5381;

    while (*str) {
        hash = ((hash << 5) + hash) ^ (*str++);
    }

    // Применение дополнительных преобразований для расширения диапазона хэша
    hash = ((hash >> 16) ^ hash) * 0x45D9F3B;
    hash = ((hash >> 16) ^ hash) * 0x45D9F3B;
    hash = (hash >> 16) ^ hash;

    // Масштабирование хэша до диапазона от 0 до 1000
    hash %= 501;

    return hash;
}


// Инициализация кэша
void initializeCache(Cache* cache) {
    cache->timeQueue = createTimeQueue();
    cache->count = 0;
    for (int i = 0; i < 500; i++)
        strcpy(cache->hash[i].domain, "");
}

// Поиск в кэше
int searchCache(Cache* cache, char* domain, char* ip) {

        if (strcmp(cache->hash[complexHash(domain)].domain, domain) == 0) {
            strcpy(ip, cache->hash[complexHash(domain)].ip);
            // Обновление положения записи в кэше (алгоритм LRU)
            increasePriority(cache->timeQueue, complexHash(domain));
            return 1;
        }

    return 0;
}

void printCache(const Cache* cache) {

    printTimeQueue(cache->timeQueue);

    for (int i = 0; i < 500; i++)
    {
        if (strcmp(cache->hash[i].domain, ""))
        {
            printf("Domain: %s, ip: %s, key: %d\n", cache->hash[i].domain, cache->hash[i].ip, i);
        }
    }
}

// Обновление кэша
void updateCache(Cache* cache, char* domain, char* ip) {
    if (cache->count < CACHE_SIZE) {
        // Добавление новой записи в кэш
        strcpy(cache->hash[complexHash(domain)].domain, domain);
        strcpy(cache->hash[complexHash(domain)].ip, ip);
        cache->count++;
        add(cache->timeQueue, complexHash(domain), 0);
    }
    else {
        // Замена наименее используемой записи в кэше
        int a = extractMinValue(cache->timeQueue);
        strcpy(cache->hash[a].domain, "");
        strcpy(cache->hash[a].ip, "");
        a = complexHash(domain);
        add(cache->timeQueue, a, 0);

        strcpy(cache->hash[a].domain, domain);
        strcpy(cache->hash[a].ip, ip);
    }
}

// Поиск IP-адреса по доменному имени в файле
int searchIP(char* domain, const char* filename, char** ip) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Ошибка при открытии файла.\n");
        return 0;
    }

    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        char beforeKey[100];
        char afterKey[100];
        int key = 1;
        const char* key1 = "INA";
        const char* key2 = "INCNAME";

        const char* keyPosition = strstr(line, key1);
        if (keyPosition == NULL) {
            key = 2;
            keyPosition = strstr(line, key2);
        }

        if (keyPosition != NULL) {
            size_t keyIndex = keyPosition - line;

            strncpy(beforeKey, line, keyIndex);
            beforeKey[keyIndex] = '\0';

            if (key == 1)
                strcpy(afterKey, line + keyIndex + strlen(key1));
            else
                strcpy(afterKey, line + keyIndex + strlen(key2));
        }
        else {
            strcpy(beforeKey, line);
            afterKey[0] = '\0';
        }
        size_t newlineIndex = strcspn(domain, "\n");
        if (newlineIndex < strlen(domain)) {
            domain[newlineIndex] = '\0';
        }
        if (!strcmp(domain, beforeKey) && key == 1)
        {
            *ip = _strdup(afterKey);
            fclose(file);
            return 1;
        }

        if (!strcmp(domain, beforeKey) && key == 2)
        {
            fclose(file);
            if (searchIP(afterKey, filename, ip))
                return 1;
            return 0;
        }

    }
}


int isValidIP(const char* ip) {
    int numFields = 0;
    int numDigits = 0;
    int fieldValue = 0;

    // Проверка каждого символа в IP-адресе
    for (int i = 0; ip[i] != '\0'; i++) {
        if (ip[i] == '.') {
            if (numDigits == 0 || numDigits > 3 || fieldValue > 255) {
                // Некорректное количество цифр или значение поля больше 255
                return 0;
            }

            numFields++;
            numDigits = 0;
            fieldValue = 0;
        }
        else if (ip[i] >= '0' && ip[i] <= '9') {
            numDigits++;
            fieldValue = fieldValue * 10 + (ip[i] - '0');
        }
        else {
            // Недопустимый символ в IP-адресе
            return 0;
        }
    }

    if (numFields != 3 || numDigits == 0 || numDigits > 3 || fieldValue > 255) {
        // Некорректное количество полей или последнее поле некорректно
        return 0;
    }

    // IP-адрес соответствует формату
    return 1;
}


// Проверка наличия дублирующей записи в файле
int isDuplicateRecord(char* ip,const char* filename) 
{
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Ошибка при открытии файла.\n");
        return 0;
    }

    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        char beforeKey[100];
        char afterKey[100];
        int key = 1;
        const char* key1 = "INA";
        const char* key2 = "INCNAME";

        const char* keyPosition = strstr(line, key1);
        if (keyPosition == NULL) {
            key = 2;
            keyPosition = strstr(line, key2);
        }

        if (keyPosition != NULL) {
            size_t keyIndex = keyPosition - line;

            strncpy(beforeKey, line, keyIndex);
            beforeKey[keyIndex] = '\0';

            if (key == 1)
                strcpy(afterKey, line + keyIndex + strlen(key1));
            else
                strcpy(afterKey, line + keyIndex + strlen(key2));
        }
        else {
            strcpy(beforeKey, line);
            afterKey[0] = '\0';
        }

        size_t newlineIndex = strcspn(afterKey, "\n");
        if (newlineIndex < strlen(afterKey)) {
            afterKey[newlineIndex] = '\0';
        }

        if (!strcmp(ip, afterKey))
        {
            fclose(file);
            return 1;
        }


    }

    fclose(file);
    return 0;
}

// Добавление новой записи в файл
void addRecordToFile(char* domain, char* ip,const char* filename) {
    FILE* file = fopen(filename, "a");
    if (file == NULL) {
        printf("Ошибка при открытии файла.\n");
        return;
    }

    fprintf(file, "%sINA%s\n", domain, ip);

    fclose(file);
}


char* getInputString() {

    // Определение начального размера буфера
    size_t bufferSize = 16;
    char* buffer = (char*)malloc(bufferSize * sizeof(char));
    if (buffer == NULL) {
        printf("Ошибка выделения памяти.\n");
        return NULL;
    }

    size_t length = 0;
    int c;

    // Чтение символов до символа новой строки или конца файла
    while ((c = getchar()) != '\n' && c != EOF) {
        buffer[length] = c;
        length++;

        // Если достигнут предел размера буфера, увеличиваем его
        if (length == bufferSize) {
            bufferSize *= 2;
            char* newBuffer = (char*)realloc(buffer, bufferSize * sizeof(char));
            if (newBuffer == NULL) {
                free(buffer);
                printf("Ошибка выделения памяти.\n");
                return NULL;
            }
            buffer = newBuffer;
        }
    }

    // Добавление нулевого символа в конец строки
    buffer[length] = '\0';

    // Уменьшение размера буфера до фактической длины строки
    char* resizedBuffer = (char*)realloc(buffer, (length + 1) * sizeof(char));
    if (resizedBuffer == NULL) {
        free(buffer);
        printf("Ошибка выделения памяти.\n");
        return NULL;
    }

    return resizedBuffer;
}

int main() {

    Cache cache;
    initializeCache(&cache);
    printf("Enter dom name: ");
    char* domain = getInputString();
    char* ip = NULL;
    int choise = 0;
    while (true)
    {
        if (choise != 2)
        {
            if (searchCache(&cache, domain, ip)) {
                printf("IP-address cache: %s\n", ip);
            }
            else if (searchIP(domain, "dns_table.txt", &ip)) {
                printf("IP-address file: %.*s\n", (int)strlen(ip), ip);
                updateCache(&cache, domain, ip);
            }
            else {
                printf("IP-address din't found.\n");
                printf("Enter IP-address for add: ");
                ip = getInputString();
                if (isValidIP(ip)) {
                    if (isDuplicateRecord(ip, "dns_table.txt")) {
                        printf("Already exist.\n");
                    }
                    else {
                        addRecordToFile(domain, ip, "dns_table.txt");
                        printf("Good.\n");
                        updateCache(&cache, domain, ip);
                    }
                }
                else {
                    printf("Incorrect.\n");
                }
            }
        }
        printf("1 - next; 0 = exit; 2 - show cache: ");
        scanf("%d", &choise);

        if (choise == 0)
        {
            return 0;
        }
        else if (choise == 1)
        {
            char c;
            while ((c = getchar()) != '\n' && c != EOF);
            char* userInput = getInputString();
            domain = (char*)malloc(strlen(userInput) + 1); 
            strcpy(domain, userInput);
            free(userInput);
        }

        else if (choise == 2)
        {
            printCache(&cache);
        }
    }
    free(domain);
    free(ip);
    return 0;
}
