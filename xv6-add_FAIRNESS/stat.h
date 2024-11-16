#ifndef STAT_H
#define STAT_H

// Definições de tipo de arquivo
#define T_DIR  1   // Directory
#define T_FILE 2   // File
#define T_DEV  3   // Device

// Definição da estrutura 'stat'
struct stat {
  short type;  // Tipo de arquivo
  int dev;     // Dispositivo de disco do sistema de arquivos
  unsigned int ino;    // Número do inode
  short nlink; // Número de links para o arquivo
  unsigned int size;   // Tamanho do arquivo em bytes
};

#endif // STAT_H
