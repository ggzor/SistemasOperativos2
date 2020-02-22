#include <grp.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/stat.h>

#include <stdio.h>
#include <time.h>
#include <string.h>

void mostrarArchivo(struct dirent *archivo, const char *rutaCompleta);

int main(int argc, char **argv) {
  DIR *directorio;
  struct dirent *archivo;

  int longitud, inicioArchivo;
  char rutaCompleta[1000];

  if (argc > 1) {
    directorio = opendir(argv[1]);

    if (directorio != NULL) {
      printf("%-9s %-10s %-4s %-11s %-8s %-10s %-10s %-5s %s\n", 
             "INODO", "PERMISOS", "REFS", "PROPIETARIO", "GRUPO", "TAMAÑO", "FECHA", "HORA", "NOMBRE");
    
      inicioArchivo = longitud = strlen(argv[1]);
      strcpy(rutaCompleta, argv[1]);
      if (rutaCompleta[longitud - 1] != '/') {
        rutaCompleta[longitud] = '/';
        inicioArchivo++;
      }

      while ((archivo = readdir(directorio))) {
        strcpy(rutaCompleta + inicioArchivo, archivo->d_name);
        mostrarArchivo(archivo, rutaCompleta);
      }

      closedir(directorio);
    } else {
      printf("Error: No se pudo abrir el directorio.\n");
    }
  } else {
    printf("Error: No se proporcionó un directorio para mostrar.\n");
  }
}

void imprimirPermisos(__mode_t modo) {
  static int banderas[] = { 
    S_IRUSR, S_IWUSR, S_IXUSR,
    S_IRGRP, S_IWGRP, S_IXGRP,
    S_IROTH, S_IWOTH, S_IXOTH
   };
  int i;

  for (i = 0; i < 9; i++) {
    if ((modo & banderas[i]) != 0) {
      switch (i % 3)
      {
      case 0:
        putchar('r');
        break;
      case 1:
        putchar('w');
        break;
      case 2:
        putchar('x');
        break;
      }
    } else {
      putchar('-');
    }
  }
}

char obtenerCaracterTipo(unsigned char tipo) {
  char c;

  if (tipo == DT_REG)
    c = '-';
  else if (tipo == DT_DIR)
    c = 'd';
  else if (tipo == DT_FIFO)
    c = 'p';
  else if (tipo == DT_SOCK)
    c = 's';
  else if (tipo == DT_CHR)
    c = 'c';
  else if (tipo == DT_BLK)
    c = 'b';
  else if (tipo == DT_LNK)
    c = 'l';
  else if (tipo == DT_UNKNOWN)
    c = 'U';
  else 
    c = 'X';

  return c;
}

void mostrarArchivo(struct dirent *archivo, const char *rutaCompleta) {
  struct stat st;
  struct passwd *usuario;
  struct group *group;
  struct tm *fecha;
  char tipoArchivo = '\0';
  char archivoFuente[100];

  // Recuperar información del archivo
  stat(rutaCompleta, &st);
  // Obtener nombre del propietario
  usuario = getpwuid(st.st_uid);
  //Obtener nombre del grupo
  group = getgrgid(st.st_gid);
  // Obtener fecha en distintos campos
  fecha = localtime(&st.st_mtime);

  // Imprimir el inodo
  printf("%-9lu ", st.st_ino);

  // Imprimir el tipo de archivo  
  tipoArchivo = obtenerCaracterTipo(archivo->d_type);
  printf("%c", tipoArchivo);
  
  // Imprimir permisos
  imprimirPermisos(st.st_mode);
  printf(" ");

  // Imprimir la cantidad de referencias
  printf("%4lu ", st.st_nlink);

  // Imprimir el propietario
  printf("%-11s ", usuario->pw_name);

  // Imprimir el grupo 
  printf("%-8s ", group->gr_name);
  
  //Imprimir el tamaño 
  printf("%6ld " , st.st_size);

  // Imprimir la fecha
  printf("%02d/%02d/%4d ", fecha->tm_mday, fecha->tm_mon + 1, fecha->tm_year + 1900);

  // Imprimir la hora
  printf("  %02d:%02d ", fecha->tm_hour, fecha->tm_min);

  // Imprimir nombre
  printf(" %s", archivo->d_name);
  if (tipoArchivo == 'l') {
    readlink(rutaCompleta, archivoFuente, 100); 
    printf(" -> ");
    if (access(archivoFuente, F_OK) != -1)
      printf("\e[32m");
    else
      printf("\e[31m");
    printf("%s", archivoFuente);
    printf("\033[0m");
  }
 
  printf("\n");
}
