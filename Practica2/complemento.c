#include <stdio.h>
/**
 *     XOR     
 *  0 - 1 = 1  
 *  1 - 1 = 0  
 **/

int complementoEntero(int entero){
  int mascaraXOR = 0xFFFFFFFF;
  return entero ^ mascaraXOR;  
}

void imprimirByte(char byte){
  int mascara = 0x80;
  for (int i = 0; i < 8; i++) {
    printf("%d", (byte & mascara) >> 7);
    byte <<= 1;
  }
}

void imprimirEntero(int entero){ 
  int mascara = 0xFF000000;
  for(int i = 0; i < 4; i++){
    imprimirByte((entero & mascara) >> 24);
    printf(" ");
    entero <<= 8;
  }
}

int main(){
  int numero = 0;
  int complemento = 0;

  printf("Numero: ");
  scanf("%d", &numero);
  getchar();

  complemento = complementoEntero(numero);
  imprimirEntero(complemento);
}