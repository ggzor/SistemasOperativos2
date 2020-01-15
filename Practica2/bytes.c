#include <stdio.h>

void imprimirBinario(char b) {
  int i;
  for (i = 0; i < 8; i++) {
    printf("%d", (b & 0x80) >> 7);
    b <<= 1;
  }
}

int main() {
  int i;
  char a[4];
  int num;

  printf("Número: ");
  scanf("%d", &num);

  // Extracción de bytes
  for (i = 0; i < 4; i++) {
    a[3 - i] = num & 0xFF;
    num >>= 8;
  }

  // Impresión de resultados
  printf("Los bytes son: ");
  for (i = 0; i < 4; i++) {
    imprimirBinario(a[i]);
    printf(" ");
  }
  printf("\n");
}
