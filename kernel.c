#include "std_lib.h"
#include "kernel.h"

// Insert global variables and function prototypes here
void printString(char* str);
void readString(char* buf);
void clearScreen();
int pipe(char* input, char parts[][256]);
int echoCheck(char* buf);
int grepCheck(char* buf);
int wcCheck(char* buf);
int iniDiaGrep(char*ygecho,char*yggrep,char* grepres,int pr);
void iniDiaWc(char* ygdikasi);
void printInt(int num);
void endl();

int rowcount=2,sigma=1;
char str[128],parted[4][256],grepres[256];
char vecho[]="echo ";
char vgrep[]="grep ";
char vc[]="wc";
char buf[128];

int main() {
    int count;
    char* echoed;char* greped;
    clearScreen();
    printString("LilHabOS - D03\n");

    while (true) {
        printString("$> ");
        readString(buf);
        printString("\n");

        if(strlen(buf)>0){
            // Insert your functions here, you may not need to modify the rest of the main() function
            count=pipe(buf, parted);
            if (echoCheck(parted[0]) && count == 1) {
                char* echoed = parted[0] + 5;
                while (*echoed == ' ') echoed++;
                printString(echoed);
                endl();
            }
            else if(count==2){
                if(echoCheck(parted[0])&&grepCheck(parted[1])){
                    echoed=parted[0]+5;
                    while (*echoed == ' ') echoed++;
                    greped=parted[1]+5;
                    while (*greped == ' ') greped++;
                    iniDiaGrep(echoed,greped,grepres,1);
                    endl();
                } 
                if (echoCheck(parted[0])&&wcCheck(parted[1])){
                    echoed=parted[0]+5;
                    while (*echoed == ' ') echoed++;
                    iniDiaWc(echoed);
                    endl();
                }
            } else if (count==3){
                if(echoCheck(parted[0])&&grepCheck(parted[1])&&wcCheck(parted[2])){
                    echoed=parted[0]+5;
                    while (*echoed == ' ') echoed++;
                    greped=parted[1]+5;
                    while (*greped == ' ') greped++;
                    if(iniDiaGrep(echoed,greped,grepres,0)){endl();iniDiaWc(grepres);}
                    endl();
                }
            }else if(strcmp(buf,"clear")){
                clearScreen();
            } else {
                printString("Unkown Command");
                endl();
            }
        }
    }
}

// Insert function here
void endl(){
    interrupt(0x10,0xE00|0x0D,0,0,0);
    interrupt(0x10,0x0200,0,0,(rowcount << 8) | 0);
    rowcount++;
}

void printString(char* str) {
    int i=0;
    while(str[i] != '\0'){
        if(strcmp(str[i],'\n')){endl();break;}
        interrupt(0x10,0xE00|str[i],0,0,0);i++;
    }
}

void readString(char* buf) {
    int i=0;char c;
    while(1){
        c = interrupt(0x16, 0x0, 0, 0, 0); 
        if(c==0x0D){  
            buf[i]='\0';
            interrupt(0x10,0xE00|0x0D,0,0,0);
            break;
        } else if(c == 0x8){ 
            if(i>0){
                interrupt(0x10,0xE00|0x8,0,0,0); 
                interrupt(0x10,0xE00|0x20,0,0,0); 
                interrupt(0x10,0xE00|0x8,0,0,0); 
                i--;
            }
        } else {
            buf[i]=c;
            interrupt(0x10,0xE00|c,0,0,0); 
            i++;
        }
    }
}

void clearScreen(){
    interrupt(0x10,0x600,0x700,0x0,0x184F);
    rowcount=1;
    interrupt(0x10,0x200,0,rowcount++,0);
}

int echoCheck(char* buf){
    int i=0;
    while (*buf == ' ') buf++;
    for(i;i<5;i++) if(buf[i]!=vecho[i]) return 0;
    return 1;
}

int grepCheck(char* buf){
    int i=0;
    while (*buf == ' ') buf++;
    for(i;i<5;i++) if(buf[i]!=vgrep[i]) return 0;
    return 1;
}

int wcCheck(char* buf){
    int i=0;
    while (*buf == ' ') buf++;
    for(i;i<2;i++) if(buf[i]!=vc[i]) return 0;
    return 1;
}  

int pipe(char* input, char parts[][256]){
    int i=0,j=0,k=0;
    while(input[i]!='\0'){
        if(input[i]=='|'){
            parts[k][j]='\0';
            k++;j=0;i++;
            while(input[i]==' ')i++;
        } else parts[k][j++]=input[i++];
    }
    parts[k][j]='\0';
    return k + 1;
}

int iniDiaGrep(char* ygecho, char* yggrep, char* grepres,int pr) {
    int i,wstart,outIndex,matched,wi,gi,gwi,m,n,wj,match;
    char word[64],grepword[64];
    i = wstart = outIndex = matched = 0;

    while (1) {
        wi = 0;
        while (ygecho[i] == ' ') i++;
        if (ygecho[i] == '\0') break;

        wstart = i;
        while (ygecho[i] != ' ' && ygecho[i] != '\0') word[wi++]= ygecho[i++];
        
        word[wi] = '\0';

        gi=0,match=0;
        while(1){
            gwi = 0;
            while(yggrep[gi] == ' ') gi++;
            if(yggrep[gi] == '\0') break;

            while(yggrep[gi] != ' ' && yggrep[gi] != '\0') grepword[gwi++]=yggrep[gi++];
            grepword[gwi] = '\0';

            m = 0;
            while(word[m] != '\0'){
                n = 0;
                while(word[m + n] == grepword[n] && grepword[n] != '\0') n++;
                if (grepword[n] == '\0'){match=1;break;}
                m++;
            }

            if (match) break;
        }

        if (match){
            wj = 0;
            while (word[wj] != '\0') grepres[outIndex++] = word[wj++];
            grepres[outIndex++] = ' ';
            matched = 1;
        }
    }

    if(matched){
        if (outIndex > 0) grepres[outIndex - 1] = '\0'; 
        else grepres[0] = '\0';
        if(pr==1)printString(grepres);
        return 1;
    } else {
        grepres[0] = '\0';
        if(pr==1)printString("NULL");
        return 0;
    }
}

void iniDiaWc(char* ygdikasi) {
    int i=0,words=0,chars=0,inWord=0;
    if(strcmp(ygdikasi,"NULL")==1){
    }else {
        while(ygdikasi[i]!='\0'){
            chars++;
            if(ygdikasi[i]!=' '&&inWord==0){
                words++;
                inWord = 1;
            } else if (ygdikasi[i]==' ') inWord = 0;
            i++;
        }

        if (chars > 0 && ygdikasi[chars - 1] == ' ') chars--;


        printInt(1);
        printString(" ");
        printInt(words);
        printString(" ");
        printInt(chars);
    }
}

void printInt(int num){
    int i = 0, j, temp,a;
    char strr[100],tempStr[100];
    if (num == 0) {
        strr[i++] = '0';
        strr[i] = '\0';
        printString(strr);
        return;
    }
    while(num != 0){
        a=mod(num,10);
        tempStr[i++]=a+'0';
        num/=10;
    }
    tempStr[i]='\0';

    for(j=0;j<i;j++) strr[j]=tempStr[i - j - 1];
    strr[j]='\0';
    printString(strr);
}
