#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parseOctet(const char** p, int* outVal) {
    const char* s = *p;
    if (!isdigit((unsigned char)*s)) return 0;
    int val = 0, digits = 0;
    while (isdigit((unsigned char)*s) && digits < 4) {
        val = val * 10 + (*s - '0');
        s++;
        digits++;
    }
    if (digits > 3) return 0;
    if (val > 255)  return 0;
    *p = s;
    *outVal = val;
    return 1;
}

static int parsePort(const char** p, int* outVal) {
    const char* s = *p;
    if (!isdigit((unsigned char)*s)) return 0;
    int val = 0, digits = 0;
    while (isdigit((unsigned char)*s) && digits < 6) {
        val = val * 10 + (*s - '0');
        s++;
        digits++;
    }
    if (digits > 5)  return 0;
    if (val > 65535) return 0;
    *p = s;
    *outVal = val;
    return 1;
}

static int parseIPv4At(const char* start, unsigned long* outAddress, int* outPort, const char** outEnd) {
    const char* p = start;
    int octets[4];

    for (int i = 0; i < 4; i++) {
        if (i > 0) {
            if (*p != '.') return 0;
            p++;
        }
        if (!parseOctet(&p, &octets[i])) return 0;
    }

    int port = -1;
    if (*p == ':') {
        const char* q = p + 1;
        if (!parsePort(&q, &port)) return 0;
        p = q;
    }

    if (isdigit((unsigned char)*p) || *p == '.') return 0;

    *outAddress = ((unsigned long)octets[0] << 24) |
                  ((unsigned long)octets[1] << 16) |
                  ((unsigned long)octets[2] <<  8) |
                  ((unsigned long)octets[3]);
    *outPort = port;
    *outEnd  = p;
    return 1;
}

int extractIPv4(const char* str, unsigned long* outAddress, int* outPort) {
    *outAddress = 0;
    *outPort = -1;
    if (str == NULL) return 0;

    const char* p = str;
    while (*p != '\0') {
        if (!isdigit((unsigned char)*p)) {
            p++;
            continue;
        }

        const char* tokenStart = p;
        const char* q = p;
        while (*q && (isdigit((unsigned char)*q) || *q == '.')) q++;

        if (*q == ':' && isdigit((unsigned char)*(q + 1))) {
            q++;
            while (isdigit((unsigned char)*q)) q++;
        }
        const char* tokenEnd = q;

        unsigned long addr;
        int port;
        const char* parseEnd;
        if (parseIPv4At(tokenStart, &addr, &port, &parseEnd) && parseEnd == tokenEnd) {
            *outAddress = addr;
            *outPort = port;
            return 1;
        }

        p = tokenEnd;
    }
    return 0;
}

static char* readLine(void) {
    size_t capacity = 256;
    size_t len = 0;
    char* buf = malloc(capacity);
    if (!buf) return NULL;

    int c;
    while ((c = getchar()) != EOF && c != '\n') {
        if (len + 1 >= capacity) {
            capacity *= 2;
            char* newBuf = realloc(buf, capacity);
            if (!newBuf) {
                free(buf);
                return NULL;
            }
            buf = newBuf;
        }
        buf[len++] = (char)c;
    }

    if (c == EOF && len == 0) {
        free(buf);
        return NULL;
    }

    buf[len] = '\0';
    return buf;
}

int main(void) {
    printf("Enter a string (or 'END' to quit):\n");

    while (1) {
        printf("> ");
        fflush(stdout);

        char* u_in = readLine();
        if (u_in == NULL) {
            break;
        }

        if (strcmp(u_in, "END") == 0) {
            free(u_in);
            break;
        }

        unsigned long outAddress;
        int outPort;

        if (extractIPv4(u_in, &outAddress, &outPort)) {
            printf("Extracted IPv4 address: %lu.%lu.%lu.%lu (decimal value: %lu, port: ",
                   (outAddress >> 24) & 0xFF,
                   (outAddress >> 16) & 0xFF,
                   (outAddress >>  8) & 0xFF,
                   (outAddress      ) & 0xFF,
                   outAddress);
            if (outPort == -1) {
                printf("none)\n");
            } else {
                printf("%d)\n", outPort);
            }
        } else {
            printf("Invalid input: no valid IPv4 address found\n");
        }

        free(u_in);
    }

    printf("Program terminated.\n");
    return 0;
}