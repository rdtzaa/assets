[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/V7fOtAk7)
|    NRP     |      Name      |
| :--------: | :------------: |
| 5025221000 | Student 1 Name |
| 5025221000 | Student 2 Name |
| 5025221000 | Student 3 Name |

# Praktikum Modul 4 _(Module 4 Lab Work)_

</div>

### Daftar Soal _(Task List)_

- [Task 1 - FUSecure](/task-1/)

- [Task 2 - LawakFS++](/task-2/)

- [Task 3 - Drama Troll](/task-3/)

- [Task 4 - LilHabOS](/task-4/)

### Laporan Resmi Praktikum Modul 4 _(Module 4 Lab Work Report)_

# _LawakFS++ - A Cursed Filesystem with Censorship and Strict Access Policies_
Teja adalah seorang penggemar sepak bola yang sangat bersemangat. Namun, akhir-akhir ini, tim kesayangannya selalu tampil kurang memuaskan di setiap pertandingan. Kekalahan demi kekalahan membuat Teja muak dan kesal. "Tim lawak!" begitu umpatnya setiap kali timnya gagal meraih kemenangan. Kekecewaan Teja yang mendalam ini menginspirasi sebuah ide: bagaimana jika ada sebuah filesystem yang bisa menyensor hal-hal "lawak" di dunia ini?

Untuk mengatasi hal tersebut, kami membuat filesystem terkutuk bernama **LawakFS++** yang mengimplementasikan kebijakan akses yang ketat, filtering konten dinamis, dan kontrol akses berbasis waktu untuk file tertentu. Filesystem ini dirancang sebagai read-only dan akan menerapkan perilaku khusus untuk akses file, termasuk logging dan manajemen konfigurasi.

### a. Ekstensi File Tersembunyi

Setelah beberapa hari menggunakan filesystem biasa, Teja menyadari bahwa ekstensi file selalu membuat orang-orang bisa mengetahui jenis file dengan mudah. "Ini terlalu mudah ditebak!" pikirnya. Dia ingin membuat sistem yang lebih misterius, di mana orang harus benar-benar membuka file untuk mengetahui isinya.

Semua file yang ditampilkan dalam FUSE mountpoint harus **ekstensinya disembunyikan**.

- **Contoh:** Jika file asli adalah `document.pdf`, perintah `ls` di dalam direktori FUSE hanya menampilkan `document`.
- **Perilaku:** Meskipun ekstensi disembunyikan, mengakses file (misalnya, `cat /mnt/your_mountpoint/document`) harus dipetakan dengan benar ke path dan nama aslinya (misalnya, `source_dir/document.pdf`).

#### Solusi
```c
static int complete_name(const char *dirname, const char *basename, char *realname) {
    DIR *dp = opendir(dirname);
    if (!dp) return -1;

    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        if (strncmp(de->d_name, basename, strlen(basename)) == 0 &&
            de->d_name[strlen(basename)] == '.') {
            strcpy(realname, de->d_name);
            closedir(dp);
            return 0;
        }
    }

    closedir(dp);
    return -1;
}

static void fullpath(char fpath[PATH_MAX], const char *path) {
    snprintf(fpath, PATH_MAX, "%s%s", source_dir, path);
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) 
{
    DIR *dp;
    struct dirent *de;
    (void) offset;
    (void) fi;

    char fpath[PATH_MAX];
    fullpath(fpath, path);
    dp = opendir(fpath);
    if (dp == NULL) return -errno;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    while ((de = readdir(dp)) != NULL) {
        if (strstr(de->d_name, secret_key) != NULL && !is_work_hours()) {
            continue;
        }
        if (de->d_type == DT_REG) {
            char *dot = strrchr(de->d_name, '.');
            if (dot) {
                char name[256];
                strncpy(name, de->d_name, dot - de->d_name);
                name[dot - de->d_name] = '\0';
                filler(buf, name, NULL, 0);
            } else {
                filler(buf, de->d_name, NULL, 0);
            }
        } else {
            filler(buf, de->d_name, NULL, 0);
        }
    }

    closedir(dp);
    return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi) 
{
    if (secret_basename(path) && !is_work_hours()) return -ENOENT;

    char fpath[PATH_MAX];
    char realname[256];
    char base[256];
    strcpy(base, path + 1);
    if (complete_name(source_dir, base, realname) == 0) {
        snprintf(fpath, PATH_MAX, "%s/%s", source_dir, realname);
    } else {
        return -ENOENT;
    }

    int fd = open(fpath, O_RDONLY);
    if (fd == -1) return -errno;

    fi->fh = fd;
    return 0;
}

static int xmp_getattr(const char *path, struct stat *stbuf) 
{
    if (secret_basename(path) && !is_work_hours()) return -ENOENT;

    int res;
    char fpath[PATH_MAX];

    if (strcmp(path, "/") == 0) {
        fullpath(fpath, path);
    } else {
        char realname[256];
        char base[256];
        strcpy(base, path + 1);
        if (complete_name(source_dir, base, realname) == 0) {
            snprintf(fpath, PATH_MAX, "%s/%s", source_dir, realname);
        } else {
            return -ENOENT;
        }
    }

    res = lstat(fpath, stbuf);
    return res == -1 ? -errno : 0;
}
```
#### Penjelasan
```c
static int complete_name(const char *dirname, const char *basename, char *realname)
```
- `opendir(dirname)` disini kita akan membuka direktori asal yang sudah di mount.
- Lalu kita jalankan `while` untuk mengecek tiap-tiap file yang ada di direktori tersebut.
- Disini kita compare `d_name` dan `basename` untuk mencari file yang cocok karena ketika kita menggunakan `cat`, kita hanya menuliskan basename dan tidak disertai ekstensi nya.
- Jika ada yang cocok akan meng-copy nama file sebenarnya ke dalam variabel `realname` dan mengembalikan nilai 0 dan diakhiri `closedir()` untuk menutup direktori yang dibuka tadi.

```c
static void fullpath(char fpath[PATH_MAX], const char *path)
```
- Fungsi ini untuk menghasilkan path sebenarnya menuju direktori yang di mount dengan mount point.
```c
static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
```
- Untuk membaca direktori mount point kita menyusun direktori lengkap nya terlebih dahulu dengan fungsi `fullpath()` lalu kita buka dengan `opendir()`.
- `filler` untuk melengkapi direktori, `.` untuk direktori saat ini dan `..` untuk direktori sebelumnya.
- Variabel `dot` disini akan menyimpan nama file tanpa ekstensi dengan fungsi `strstr()` yang hanya mengambil karakter hingga karakter tertentu.
- Setelah didapat nama yang terfilter kita panggil `filler` untuk memunculkan nama-nama file tersebut di direktori mount point dan diakhiri `closedir()`.
```c
static int xmp_open(const char *path, struct fuse_file_info *fi);
static int xmp_getattr(const char *path, struct stat *stbuf);
```
- Kedua fungsi ini memiliki cara kerja yang sama yaitu membutuhkan nama file berekstensi sehingga akan memanggil fungsi `complete_name()` dan digabung ke path direktori asli.
- Untuk `xmp_open` akan menggunakan fungsi `open()` dan `xmp_getattr` akan menggunakan fungsi `lstat`.
#### Output
![image](https://github.com/rdtzaa/assets/blob/06accd89cc5f5b3174ab0da073d385049b12c202/Sistem%20Operasi/task2_a.png)

### b. Akses Berbasis Waktu untuk File Secret

Suatu hari, Teja menemukan koleksi foto-foto memalukan dari masa SMA-nya yang tersimpan dalam folder bernama "secret". Dia tidak ingin orang lain bisa mengakses file-file tersebut kapan saja, terutama saat dia sedang tidur atau tidak ada di rumah. "File rahasia hanya boleh dibuka saat jam kerja!" putusnya dengan tegas.

File yang nama dasarnya adalah **`secret`** (misalnya, `secret.txt`, `secret.zip`) hanya dapat diakses **antara pukul 08:00 (8 pagi) dan 18:00 (6 sore) waktu sistem**.

- **Pembatasan:** Di luar rentang waktu yang ditentukan, setiap percobaan untuk membuka, membaca, atau bahkan melakukan list file `secret` harus menghasilkan error `ENOENT` (No such file or directory).
- **Petunjuk:** Kamu perlu mengimplementasikan kontrol akses berbasis waktu ini dalam operasi FUSE `access()` dan/atau `getattr()` kamu.
#### Solusi
```c
int secret_basename(const char *path) {
    const char *base = strrchr(path, '/');
    base = base ? base + 1 : path;
    return strcasecmp(base, secret_key) == 0;
}

int is_work_hours() {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    int hour = t->tm_hour;
    return (hour >= access_start_hour && hour < access_end_hour);
}
```
#### Penjelasan
```c
int secret_basename(const char *path)
```
- Fungsi ini akan mengambil basename dari file yang dioperasikan dalam command terminal yang kemudian akan dicocokkan dengan `strcasecmp` yang akan menghasilkan 1 jika cocok dan 0 jika tidak.
- `strcasecmp` kita gunakan agar tidak terpengaruh dengan case-sensitive.
```c
int is_work_hours()
```
- Kita gunakan fungsi `time()` dan mengambil angka hour dari struct yang dihasilkan lalu kita cek apakah angka hour berada diantara jam akses yang ditentukan.

Fungsi-fungsi diatas akan dipanggil diawal di fungsi `xmp_getattr`, `xmp_access`, `xmp_open`, `xmp_readdir`, dan `xmp_read` yang mana fungsi-fungsi tersebut yang mengatasi ketika user akan membaca suatu file atau mengecek suatu file seperti `ls` dan `cat`.
#### Output
- Akses berhasil:
![image](https://github.com/rdtzaa/assets/blob/06accd89cc5f5b3174ab0da073d385049b12c202/Sistem%20Operasi/akses_acc.png)

- Akses gagal:
![image](https://github.com/rdtzaa/assets/blob/06accd89cc5f5b3174ab0da073d385049b12c202/Sistem%20Operasi/akses_acc.png)

### c. Filtering Konten Dinamis

Kekesalan Teja terhadap hal-hal "lawak" semakin memuncak ketika dia membaca artikel online yang penuh dengan kata-kata yang membuatnya kesal. Tidak hanya itu, gambar-gambar yang dia lihat juga sering kali tidak sesuai dengan ekspektasinya. "Semua konten yang masuk ke sistem saya harus difilter dulu!" serunya sambil mengepalkan tangan.

Ketika sebuah file dibuka dan dibaca, isinya harus **secara dinamis difilter atau diubah** berdasarkan tipe file yang terdeteksi:

| Tipe File      | Perlakuan                                                                                 |
| :------------- | :---------------------------------------------------------------------------------------- |
| **File Teks**  | Semua kata yang dianggap lawak (case-insensitive) harus diganti dengan kata `"lawak"`.    |
| **File Biner** | Konten biner mentah harus ditampilkan dalam **encoding Base64** alih-alih bentuk aslinya. |

> **Catatan:** Daftar "kata-kata lawak" untuk filtering file teks akan didefinisikan secara eksternal, seperti yang ditentukan dalam persyaratan **e. Konfigurasi**.
#### Solusi
```c
void filter_text(char *buf) {
    char *lower_buf = strdup(buf);
    if (!lower_buf) return;
    to_lower_str(lower_buf);

    for (int i = 0; i < filter_word_count; i++) {
        char *pos = lower_buf;
        size_t word_len = strlen(filter_words[i]);
        size_t replace_len = strlen("lawak");

        while ((pos = strstr(pos, filter_words[i])) != NULL) {
            size_t offset = pos - lower_buf;
            memcpy(buf + offset, "lawak", replace_len);
            if (replace_len < word_len) {
                memset(buf + offset + replace_len, ' ', word_len - replace_len);
            }
            memcpy(pos, "lawak", replace_len);

            pos += replace_len;
        }
    }
    free(lower_buf);
}

static const char base64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char *src, size_t len, char *out) {
    size_t i, j;
    for (i = 0, j = 0; i < len;) {
        uint32_t octet_a = i < len ? src[i++] : 0;
        uint32_t octet_b = i < len ? src[i++] : 0;
        uint32_t octet_c = i < len ? src[i++] : 0;

        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;

        out[j++] = base64_table[(triple >> 3 * 6) & 0x3F];
        out[j++] = base64_table[(triple >> 2 * 6) & 0x3F];
        out[j++] = base64_table[(triple >> 1 * 6) & 0x3F];
        out[j++] = base64_table[(triple >> 0 * 6) & 0x3F];
    }
    for (i = 0; i < ((3 - len % 3) % 3); i++)
        out[j - 1 - i] = '=';
    out[j] = '\0';
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) 
{
    if (secret_basename(path) && !is_work_hours()) return -ENOENT;
    off_t file_size = lseek(fi->fh, 0, SEEK_END);
    lseek(fi->fh, 0, SEEK_SET);

    unsigned char *file_buf = malloc(file_size + 1);
    if (!file_buf) return -ENOMEM;

    ssize_t read_bytes = read(fi->fh, file_buf, file_size);
    if (read_bytes < 0) {
        free(file_buf);
        return -errno;
    }
    file_buf[read_bytes] = 0;

    int is_text = 1;
    for (ssize_t i = 0; i < read_bytes && i < 1024; i++) {
        if ((file_buf[i] < 32 || file_buf[i] > 126) && file_buf[i] != '\n' && file_buf[i] != '\r' && file_buf[i] != '\t') {
            is_text = 0;
            break;
        }
    }

    int ret = 0;

    if (is_text) {
        filter_text((char *)file_buf);

        size_t len = strlen((char *)file_buf);
        if (offset < (off_t)len) {
            if (offset + size > len) size = len - offset;
            memcpy(buf, file_buf + offset, size);
            ret = size;
        } else {
            ret = 0;
        }
    } else {
        size_t encoded_len = 4 * ((read_bytes + 2) / 3);
        char *encoded_buf = malloc(encoded_len + 1);
        if (!encoded_buf) {
            free(file_buf);
            return -ENOMEM;
        }

        base64_encode(file_buf, read_bytes, encoded_buf);

        size_t enc_len = strlen(encoded_buf);
        if (offset < (off_t)enc_len) {
            if (offset + size > enc_len) size = enc_len - offset;
            memcpy(buf, encoded_buf + offset, size);
            ret = size;
        } else {
            ret = 0;
        }
        free(encoded_buf);
    }

    free(file_buf);

    if (ret >= 0) {
        log_access("READ", path);
    }

    return ret;
}
```
#### Penjelasan
```c
void filter_text(char *buf)
```
- `lower_buf` dibuat dari `buf` dengan semua huruf diubah menjadi huruf kecil dengan fungsi `to_lower_str`.
- Kita gunakan `for` loop untuk setiap kata dalam `filter_words` untuk mencari posisi tiap kata tersebut dalam `lower_buf` menggunakan `strstr()`. Jika cocok, `memcpy` mengganti isi asli di buf dengan "lawak". Jika kata aslinya lebih panjang dari "lawak", sisanya diisi dengan spasi.

```c
void base64_encode(const unsigned char *src, size_t len, char *out)
```
- Membaca tiap 3 byte (24-bit) lalu membaginya menjadi 4 grup 6-bit.
- Lalu, setiap grup digunakan untuk mengambil 1 karakter dari `base64_table` sesuai dengan index yang didapat.
- Jika ukuran file tidak kelipatan 3, ditambahkan karakter `=` sebagai pengisi agar menjadi kelipatan 3.
```c
static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
```
- Diawali dengan mengecek `secret` dan `access_hour`
- Lalu membuat variabel untuk ukuran buffer dengan `lseek`
- Gunakan `read` untuk mendapatkan ukuran byte dan mengcopy isi ke dalam buffer yang sudah dibuat.
- Selanjutnya gunakan looping kembali untuk mengecek apakah hasil buffer adalah sebuah text atau gambar
- Nah jika merupakan teks kita langsung panggil fungsi `filter_text()` dan `memcpy` ke `buf` untuk ditampilkan di terminal, sebaliknya jika itu adalah gambar maka kita panggil `base64_encode` dan hasil encode nya kita `memcpy` ke buf untuk ditampilkan.
#### Output
![image](https://github.com/rdtzaa/assets/blob/06accd89cc5f5b3174ab0da073d385049b12c202/Sistem%20Operasi/filter_text_gambar.png)

### d. Logging Akses

Sebagai seorang yang paranoid, Teja merasa perlu untuk mencatat setiap aktivitas yang terjadi di filesystemnya. "Siapa tahu ada yang mencoba mengakses file-file penting saya tanpa izin," gumamnya sambil menyiapkan sistem logging. Dia ingin setiap gerakan tercatat dengan detail, lengkap dengan waktu dan identitas pelakunya.

Semua operasi akses file yang dilakukan dalam LawakFS++ harus **dicatat** ke file yang terletak di **`/var/log/lawakfs.log`**.

Setiap entri log harus mematuhi format berikut:

```
[YYYY-MM-DD HH:MM:SS] [UID] [ACTION] [PATH]
```

Di mana:

- **`YYYY-MM-DD HH:MM:SS`**: Timestamp operasi.
- **`UID`**: User ID pengguna yang melakukan aksi.
- **`ACTION`**: Jenis operasi FUSE (misalnya, `READ`, `ACCESS`, `GETATTR`, `OPEN`, `READDIR`).
- **`PATH`**: Path ke file atau direktori dalam FUSE mountpoint (misalnya, `/secret`, `/images/photo.jpg`).

> **Persyaratan:** Kamu **hanya diwajibkan** untuk mencatat operasi `read` dan `access` yang berhasil. Logging operasi lain (misalnya, write yang gagal) bersifat opsional.
#### Solusi
```c
static void log_access(const char *action, const char *path) {
    FILE *logfile = fopen("/var/log/lawakfs.log", "a");
    if (!logfile) return;

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestr[20];
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", tm_info);

    uid_t uid = fuse_get_context()->uid;

    fprintf(logfile, "[%s] [%d] [%s] [%s]\n", timestr, uid, action, path);
    fclose(logfile);
}
```
#### Penjelasan
- Membuka path file log dengan `fopen` dengan akses `a` yaitu jika belum ada akan membuat dan jika sudah ada akan membuka file yang sudah ada.
- Dapatkan catatan waktu dengan `time`.
- Dapatkan uid dengan `fuse_get_context` agar mendapatkan uid yang mengakses mount point tersebut
- `fprintf` untuk mencatat ke dalam file log dengan action READ atau ACCESS dan diakhiri `fclose` untuk menutup file.
- Fungsi ini akan dipanggil di akhir fungsi `xmp_access` dan `xmp_read`.
#### Output
![image](https://github.com/rdtzaa/assets/blob/06accd89cc5f5b3174ab0da073d385049b12c202/Sistem%20Operasi/log.png)

### e. Konfigurasi

Setelah menggunakan filesystemnya beberapa minggu, Teja menyadari bahwa kebutuhannya berubah-ubah. Kadang dia ingin menambah kata-kata baru ke daftar filter, kadang dia ingin mengubah jam akses file secret, atau bahkan mengubah nama file secret itu sendiri. "Saya tidak mau repot-repot kompilasi ulang setiap kali ingin mengubah pengaturan!" keluhnya. Akhirnya dia memutuskan untuk membuat sistem konfigurasi eksternal yang fleksibel.

Untuk memastikan fleksibilitas, parameter-parameter berikut **tidak boleh di-hardcode** dalam source code `lawak.c` kamu. Sebaliknya, mereka harus dapat dikonfigurasi melalui file konfigurasi eksternal (misalnya, `lawak.conf`):

- **Nama file dasar** dari file 'secret' (misalnya, `secret`).
- **Waktu mulai** untuk mengakses file 'secret'.
- **Waktu berakhir** untuk mengakses file 'secret'.
- **Daftar kata-kata yang dipisahkan koma** yang akan difilter dari file teks.

**Contoh konten `lawak.conf`:**

```
FILTER_WORDS=ducati,ferrari,mu,chelsea,prx,onic,sisop
SECRET_FILE_BASENAME=secret
ACCESS_START=08:00
ACCESS_END=18:00
```

FUSE kamu harus membaca dan mem-parse file konfigurasi ini saat inisialisasi.
#### Solusi
```c
int init_conf(const char *config_path) {
    FILE *f = fopen(config_path, "r");
    if (!f) return -1;

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        trim(line);
        if (strncmp(line, "FILTER_WORDS=", 13) == 0) {
            char *p = line + 13;
            char *token = strtok(p, ",");
            while(token && filter_word_count < MAX_FILTER_WORDS) {
                filter_words[filter_word_count] = strdup(token);

                for (char *c = filter_words[filter_word_count]; *c; c++) {
                    *c = tolower(*c);
                }
                filter_word_count++;
                token = strtok(NULL, ",");
            }
        } else if (strncmp(line, "SECRET_FILE_BASENAME=", 21) == 0) {
            strcpy(secret_key, line + 21);
            trim(secret_key);
        } else if (strncmp(line, "ACCESS_START=", 13) == 0) {
            sscanf(line + 13, "%d:%*d", &access_start_hour);
        } else if (strncmp(line, "ACCESS_END=", 11) == 0) {
            sscanf(line + 11, "%d:%*d", &access_end_hour);
        }
    }
    fclose(f);
    return 0;
}

int main(int argc, char *argv[]) {
    umask(0);

    if (init_conf("/home/rdtzaaa/task-2/lawak.conf") != 0) {
        printf("Gagal membaca konfigurasi lawak.conf\n");
        return 1;
    }

    return fuse_main(argc, argv, &xmp_oper, NULL);
}
```
#### Penjelasan
- Fungsi ini akan dipanggil diawal ketika program berjalan.
- Gunakan `fopen` untuk membuka path file log yang dibuat yaitu `/var/log/lawakfs/log`
- Gunakan `trim()` untuk menghapus kemungkinan spasi dalam mengisi config
- Gunakan `while` loop untuk mengecek setiap baris dari config tersebut dan gunakan `strtok` untuk mengatasi kata kunci yang lebih dari satu sehingga membutuhkan tanda koma.
#### Output
![image](https://github.com/rdtzaa/assets/blob/06accd89cc5f5b3174ab0da073d385049b12c202/Sistem%20Operasi/lawakconf.png)
