// kernel.c
void main() {
  char* str = "FP telah menunggumu";
  int i = 0;
  int j = 0;

  for (i = 0; i < 1500; i++) {
    char warna = 0x5;
    putInMemory(0xB000, 0x8000 + i * 2, str[i]);
    putInMemory(0xB000, 0x8001 + i * 2, warna);
    if (i >= 19) {
      putInMemory(0xB000, 0x8000 + i * 2, ' ');
      putInMemory(0xB000, 0x8001 + i * 2, 0x07);
    }
  }

  while (1);
}
