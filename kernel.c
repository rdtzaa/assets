// kernel.c
void clearScreen();
void endl();
int rowcount=2;

void main() {
  char* str = "FP telah menunggumu";
  int i = 0;

  clearScreen();
  for (i = 0; i < 19; i++) {
    char warna = 0x5;
    putInMemory(0xB000, 0x8000 + i * 2, str[i]);
    putInMemory(0xB000, 0x8001 + i * 2, warna);
  }
  endl();

  while (1);
}

void clearScreen(){
    interrupt(0x10,0x600,0x700,0x0,0x184F);
    rowcount=1;
    interrupt(0x10,0x200,0,rowcount++,0);
}

void endl(){
    interrupt(0x10,0xE00|0x0D,0,0,0);
    interrupt(0x10,0x0200,0,0,(rowcount << 8) | 0);
    rowcount++;
}
