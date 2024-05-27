#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *archivo;
FILE *salida;


typedef enum {
  L_CORCHETE,
  R_CORCHETE,
  L_LLAVE,
  R_LLAVE,
  COMA,
  DOS_PUNTOS,
  LITERAL_CADENA,
  LITERAL_NUM,
  PR_TRUE,
  PR_FALSE,
  PR_NULL,
  EOF_TOKEN
} TokenType;

const char *tokenNames[] = {"L_CORCHETE",     "R_CORCHETE",  "L_LLAVE",
                            "R_LLAVE",        "COMA",        "DOS_PUNTOS",
                            "LITERAL_CADENA", "LITERAL_NUM", "PR_TRUE",
                            "PR_FALSE",       "PR_NULL",     "EOF_TOKEN"};

typedef struct {
  TokenType type;
  char *value;
} Token;

Token token; 
char *objetonombre;
int count = 1;
char *xd;
int band = 1;

Token analizador(FILE *archivo);
void parseJson(FILE *archivo);
void parseElement(FILE *archivo);
void parseArray(FILE *archivo);
void parseElementList(FILE *archivo);
void parseObject(FILE *archivo);
void parseAttributesList(FILE *archivo);
void parseAttribute(FILE *archivo);
void aprima(FILE *archivo);
void eprima(FILE *archivo);
void parseAttributeName(FILE *archivo);
void parseAttributeValue(FILE *archivo);
void match(TokenType expected, Token token);
void escribirToken(FILE *output, const char *token);

Token analizador(FILE *archivo) {
  Token token;
  int c = fgetc(archivo);

  while (c == ' ' || c == '\t' || c == '\n') {
      c = fgetc(archivo);
  }

  if (c == EOF) {
    token.type = EOF_TOKEN;
    token.value = NULL;
  } else if (c == '[') {
    token.type = L_CORCHETE;
    token.value = NULL;
  } else if (c == ']') {
    token.type = R_CORCHETE;
    token.value = NULL;
  } else if (c == '{') {
    token.type = L_LLAVE;
    token.value = NULL;
  } else if (c == '}') {
    token.type = R_LLAVE;
    token.value = NULL;
  } else if (c == ',') {
    token.type = COMA;
    token.value = NULL;
  } else if (c == ':') {
    token.type = DOS_PUNTOS;
    token.value = NULL;
  } else if (c == '"') {
    token.type = LITERAL_CADENA;
    char buffer[1024];
    int i = 0;
    while ((c = fgetc(archivo)) != EOF && c != '"') {
      buffer[i++] = (char)c;
      if (i == sizeof(buffer) - 1) {
        printf("Tamaño máximo de cadena excedido");
      }
    }
    buffer[i] = '\0';
    token.value = malloc(strlen(buffer) + 1);
    if (token.value == NULL) {
      printf("Error de asignación de memoria");
    }
    token.value = strcpy(token.value, buffer);
  } else if ((c >= '0' && c <= '9') || c == '-' || c == '+') {
    token.type = LITERAL_NUM;
    char buffer[1024];
    int i = 0;
    buffer[0] = (char)c;
    c = fgetc(archivo);
    i = 1;
    while (c == '1' || c == '2' || c == '3' || c == '4' || c == '5' ||
           c == '6' || c == '7' || c == '8' || c == '9' || c == '0' ||
           c == '.' || c == '+' || c == '-') {
      buffer[i++] = (char)c;
      
      c = fgetc(archivo);
    }
    buffer[i] = '\0';
    token.value = malloc(strlen(buffer) + 1);
    fprintf(salida, "%s", strcpy(token.value, buffer));
    if (c == ',') {
      token.type = COMA;
    }
    token.value = NULL;
  } else if (c == 't' || c == 'T') {
    token.type = PR_TRUE;
    char buffer[4];
    int i = 1;
    while ((c = fgetc(archivo)) != EOF && i < sizeof(buffer) - 1 &&
           (c == 'r' || c == 'R' || c == 'u' || c == 'U' || c == 'e' ||
            c == 'E')) {
      buffer[i++] = (char)c;
    }
    token.value = "true";
  } else if (c == 'f' || c == 'F') {
    token.type = PR_FALSE;
    char buffer[5];
    int i = 1;
    while ((c = fgetc(archivo)) != EOF && i < sizeof(buffer) - 1 &&
           (c == 'a' || c == 'A' || c == 'l' || c == 'L' || c == 's' ||
            c == 'S' || c == 'e' || c == 'E')) {
      buffer[i++] = (char)c;
    }
    token.value = "false";
  } else if (c == 'n' || c == 'N') {
    token.type = PR_NULL;
    char buffer[4];
    int i = 1;
    while ((c = fgetc(archivo)) != EOF && i < sizeof(buffer) - 1 &&
           (c == 'u' || c == 'U' || c == 'l' || c == 'L')) {
      buffer[i++] = (char)c;
    }
    token.value = "null";
  } else {
    printf("Caracter no reconocido %c\n", c);
  }

  return token;
}

void parseJson(FILE *archivo) {
  token = analizador(archivo);
  parseElement(archivo);
  fprintf(salida, "</%s>",objetonombre);
  match(EOF_TOKEN, analizador(archivo));
  if(band == 1){
    printf("Json válido\n");
  }else{
    printf("Json inválido\n");
  }
}

void parseElement(FILE *archivo) {
  switch (token.type) {
  case L_LLAVE:
    match(L_LLAVE, token);
    if(count != 1){
      fprintf(salida, "<item>\n");
    }
    parseObject(archivo);
    break;
  case L_CORCHETE:
    match(L_CORCHETE, token);
    parseArray(archivo);
    break;
  default:
    break;
  }
}

void parseArray(FILE *archivo) {
  token = analizador(archivo);
  parseElementList(archivo);
  switch (token.type) {
  case R_CORCHETE:
    match(R_CORCHETE, token);
    token = analizador(archivo);
    break;
  }
}

void parseElementList(FILE *archivo) {
  parseElement(archivo);
  eprima(archivo);
}

void eprima(FILE *archivo) {
  switch (token.type) {
  case COMA:
    match(COMA, token);
    fprintf(salida, "");
    token = analizador(archivo);
    parseElement(archivo);
    eprima(archivo);
    break;
  default:
    break;
  }
}

void parseObject(FILE *archivo) {
  token = analizador(archivo);
  switch (token.type) {
  case R_LLAVE:
    match(R_LLAVE, token);
    token = analizador(archivo);
    break;
  default:
    parseAttributesList(archivo);
    break;
  }
  
}

void parseAttributesList(FILE *archivo) {
  parseAttribute(archivo);
  aprima(archivo);
}

void parseAttribute(FILE *archivo) {
  parseAttributeName(archivo);
  match(DOS_PUNTOS, token);
  parseAttributeValue(archivo);
  if (strcmp(xd, "") != 0) {
    fprintf(salida, "</%s>\n", xd);
    xd = "";
  }
}

void aprima(FILE *archivo) {
  switch (token.type) {
  case COMA:
    match(COMA, token);
    fprintf(salida, "");
    token = analizador(archivo);
    parseAttribute(archivo);
    aprima(archivo);
    break;
  default:
    match(R_LLAVE, token);
    token = analizador(archivo);
    fprintf(salida, "</item>\n");
    break;
  }
}

void parseAttributeName(FILE *archivo) {
  match(LITERAL_CADENA, token);
  xd = token.value;
  if(count == 1){
    objetonombre = token.value;
    fprintf(salida, "<%s>\n", xd);
    count++;
  }else{
    fprintf(salida, "<%s>", xd);
  }
  token = analizador(archivo);
}

void parseAttributeValue(FILE *archivo) {
  token = analizador(archivo);

  switch (token.type) {
  case LITERAL_CADENA:
    match(LITERAL_CADENA, token);
    fprintf(salida, "%s", token.value);
    token = analizador(archivo);
    break;
  case LITERAL_NUM:
    match(LITERAL_NUM, token);
    token = analizador(archivo);
    break;
  case PR_TRUE:
    match(PR_TRUE, token);
    fprintf(salida, "%s", token.value);
    token = analizador(archivo);
    break;
  case PR_FALSE:
    match(PR_FALSE, token);
    fprintf(salida, "%s", token.value);
    token = analizador(archivo);
    break;
  case PR_NULL:
    match(PR_NULL, token);
    fprintf(salida, "%s", token.value);
    token = analizador(archivo);
    break;
  default:
    parseElement(archivo);
    break;
  }
}

void match(TokenType expected, Token token) {
  if (token.type != expected) {
    printf("Token inesperado. Se esperaba %s pero se encontró %s\n",
           tokenNames[expected], tokenNames[token.type]);
    band = 0;
  }
}


void escribirToken(FILE *output, const char *token) {
  fprintf(output, "%s ", token);
}

int main(void) {
  archivo = fopen("fuente.txt", "r");
  salida = fopen("output.xml", "w");
  if (archivo == NULL || salida == NULL) {
    perror("Error al abrir el archivo");
    return 1;
  }

  parseJson(archivo);

  fclose(archivo);
  fclose(salida);

  return 0;
}