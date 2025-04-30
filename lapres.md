# Task 4 _(Cella’s Manhwa)_
Cella, si ratu scroll Facebook, tiba-tiba terinspirasi untuk mengumpulkan informasi dan foto dari berbagai **manhwa favoritnya**. Namun, kemampuan ngoding Cella masih cetek, jadi dia butuh bantuanmu untuk membuatkan skrip otomatis agar semua berjalan mulus. Tugasmu adalah membantu Cella mengolah data manhwa dan heroine-nya.

Berikut adalah daftar manhwa bergenre shoujo/josei yang paling disukai Cella:

|    No     |      Manhwa      |
| :--------: | :------------: |
| 1 | Mistaken as the Monster Duke's Wife |
| 2 | The Villainess Lives Again |
| 3 | No, I Only Charmed the Princess! |
| 4 | Darling, Why Can't We Divorce? |

Untuk menyelesaikan beberapa persoalan setelah ini, saya membuat beberapa fungsi tambahan untuk mempermudah dalam memecahkan setiap permasalahan nanti. Berikut fungsi-fungsi nya:
##### ``Fungsi mengambil data dari API``
```c
struct memory {
    char *memory;
    size_t size;
};

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct memory *mem = (struct memory *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

char* fetch_json(char* url) {
    CURL *curl;
    CURLcode res;
    struct memory chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    return chunk.memory;
}
```
- Pada kode diatas, terdapat struct ``memory`` yang berfungsi menyimpan isi jSON dari API yang kita miliki. Disini, kita menggunakan library ``cJSON`` untuk mendapatkan informasi dari API tersebut. ``CURLOPT_WRITEFUNCTION`` disini berfungsi mengambil berupa memory dari jSON yang kita dapat kemudian kita salin ke struct kita dengan ``CURL_WRITEDATA`` lalu setelah itu kita mengembalikan nilai memory saja karena yang kita perlukan hanya isi dari jSON tersebut.
##### ``Fungsi zip suatu file``
```c
void buat_zip(char* name, char* dest) {
    pid_t pid = fork();
    if (pid == 0) {
        char *args[] = {"zip", "-jq", name, dest, NULL};
        printf("%s terzip\n", dest);
        execv("/usr/bin/zip", args);
        exit(1);
    } else {
        wait(NULL);
    }
}
```
- Fungsi diatas ini berguna untuk membuat zip dari suatu file sesuai dengan parameter yang ada. ``name`` disini bisa berupa nama zip sekaligus letak direktori nya lalu ``dest`` disini berupa file yang akan di-zip menuju variabel ``name``.
##### ``Fungsi membuat direktori``
```c
void buat_direktori(char* folder) {
    pid_t pid = fork();
    if (pid == 0) {
        char *args[] = {"mkdir", "-p", folder, NULL};
        execv("/bin/mkdir", args);
        exit(1);
    } else {
        wait(NULL);
    }
}
```
- Fungsi ini diatas ini berguna untuk membuat direktori baru berdasarkan parameter yang ada yaitu berupa ``folder`` yang bisa berisi nama folder atau beserta di direktori mana ingin membuat direktori baru tersebut.
##### ``Fungsi mengganti spasi menjadi underscore(_) dan menghilangkan karakter khusus``
```c
void format_txt(char *a, char *b) {
    int i, j = 0;
    for (i = 0; a[i] != '\0'; i++) {
        if ((a[i] >= 'a' && a[i] <= 'z') || (a[i] >= 'A' && a[i] <= 'Z') || (a[i] >= '0' && a[i] <= '9')) {
            b[j] = a[i];
            j++;
        } else if (a[i] == ' ') {
            b[j] = '_';
            j++;
        }
    }
    b[j] = '\0';
    strcat(b, ".txt");
}
```
- Fungsi ini akan berguna di beberapa poin persoalan yang berguna untuk mengganti spasi dengan underscore (_) lalu menghilangkan karakter khusus dari sebuat string.
### Poin A (Summoning the Manhwa Stats)
Cella ingin mengambil data detail dari **manhwa** menggunakan [API Jikan](https://docs.api.jikan.moe/). Informasi yang diambil:

- Judul
- Status
- Tanggal rilis
- Genre
- Tema
- Author

Setelah data berhasil diambil, hasilnya harus disimpan ke dalam file teks, dengan nama file disesuaikan dengan **judul versi bahasa Inggris** (tanpa karakter khusus dan spasi diganti dengan underscore). Semua file teks disimpan dalam folder `Manhwa`.

#### Solusi
```c
void task1(char *json) {
    cJSON *json_data = cJSON_Parse(json);
    cJSON *data = cJSON_GetObjectItem(json_data, "data");

    char *status = cJSON_GetObjectItem(data, "status")->valuestring;
    char *title = cJSON_GetObjectItem(data, "title_english")->valuestring;

    cJSON *published = cJSON_GetObjectItem(data, "published");
    int year = 0, month = 0, day = 0;
    if (published) {
        cJSON *prop = cJSON_GetObjectItem(published, "prop");
        cJSON *from = cJSON_GetObjectItem(prop, "from");
        year = cJSON_GetObjectItem(from, "year")->valueint;
        month = cJSON_GetObjectItem(from, "month")->valueint;
        day = cJSON_GetObjectItem(from, "day")->valueint;
    }

    char genres[512], authors[512], themes[512];
    genres[0] = '\0';
    authors[0] = '\0';
    themes[0] = '\0';
    cJSON *genres_tmp = cJSON_GetObjectItem(data, "genres");
    cJSON *authors_tmp = cJSON_GetObjectItem(data, "authors");
    cJSON *themes_tmp = cJSON_GetObjectItem(data, "themes");

    cJSON *item;
    cJSON_ArrayForEach(item, genres_tmp) {
        char *name = cJSON_GetObjectItem(item, "name")->valuestring;
        if (strlen(genres) > 0) {
            strcat(genres, ", ");
        }
        strcat(genres, name);
    }

    cJSON_ArrayForEach(item, authors_tmp) {
        char *name = cJSON_GetObjectItem(item, "name")->valuestring;
        if (strlen(authors)) strcat(authors, ", ");
        strcat(authors, name);
    }

    cJSON_ArrayForEach(item, themes_tmp) {
        char *name = cJSON_GetObjectItem(item, "name")->valuestring;
        if (strlen(themes)) strcat(themes, ", ");
        strcat(themes, name);
    }

    char dir[256] = "Manhwa/";
    char filename[128];
    buat_direktori("Manhwa");
    format_txt(title, filename);
    strcat(dir, filename);

    FILE *f = fopen(dir, "w");
    if (f) {
        fprintf(f, "Title: %s\n", title);
        fprintf(f, "Status: %s\n", status);
        fprintf(f, "Release: %d-%02d-%02d\n", year, month, day);
        fprintf(f, "Genres: %s\n", genres);
        fprintf(f, "Theme: %s\n", themes);
        fprintf(f, "Authors: %s\n", authors);
        fclose(f);
    }

    cJSON_Delete(json_data);
}
```
#### Penjelasan
```c
cJSON *json_data = cJSON_Parse(json);
cJSON *data = cJSON_GetObjectItem(json_data, "data");
```
- ``cJSON_Parse(json)`` untuk menyimpan data JSON yang sudah dipisah-pisah perbagian sehingga data JSON dapat dioperasikan untuk mencari bagian tertentu.
- ``cJSON_GetObjectItem(json_data, "data")`` untuk mengambil bagian dengan kata kunci ``data`` yang berisi data dari manhwa itu sendiri.
```c
char *status = cJSON_GetObjectItem(data, "status")->valuestring;
char *title = cJSON_GetObjectItem(data, "title_english")->valuestring;
```
- Menginisialisai variabel ``char *`` yang berfungsi menyimpan alamat memori yang berupa string.
- ``cJSON_GetObjectItem(data, <kata_kunci>)->valuestring``, fungsi dari library cJSON ini kita akan memfilter kembali JSON yang sudah di uraikan tadi menjadi lebih spesifik dengan kata kunci tertentu, lalu ``->valuestring`` kita dapat mengambil data nya dengan tambahan pointer ke value nya. Disini kita akan mengambil judul versi bahasa Inggris dan status manhwa tersebut.
```c
cJSON *published = cJSON_GetObjectItem(data, "published");
int year = 0, month = 0, day = 0;
if (published) {
    cJSON *prop = cJSON_GetObjectItem(published, "prop");
    cJSON *from = cJSON_GetObjectItem(prop, "from");
    year = cJSON_GetObjectItem(from, "year")->valueint;
    month = cJSON_GetObjectItem(from, "month")->valueint;
    day = cJSON_GetObjectItem(from, "day")->valueint;
}
```
Bagian ini diawali kita menginisialisasi variabel integer untuk menyimpan tahun, bulan, dan hari. Kita juga membuat variabel ``cJSON`` yang mengambil bagian ``published`` yang mana di dalam bagian tersebut terdapat tanggal terbit manhwa tersebut.
- ``cJSON *prop = cJSON_GetObjectItem(published, "prop")`` dan ``cJSON *from = cJSON_GetObjectItem(prop, "from");`` dua fungsi ini akan memperkecil yaitu fokus pada isi ``prop`` yang berisi tanggal pertama kali publish.
- ``cJSON_GetObjectItem(from, <kata_kunci>)->valueint``, fungsi ini kita gunakan untuk mengambil isi dari objek sesuai kata kunci yaitu berupa year, month, atau day yang berupa integer.
```c
char genres[512], authors[512], themes[512];
genres[0] = '\0';
authors[0] = '\0';
themes[0] = '\0';
cJSON *genres_tmp = cJSON_GetObjectItem(data, "genres");
cJSON *authors_tmp = cJSON_GetObjectItem(data, "authors");
cJSON *themes_tmp = cJSON_GetObjectItem(data, "themes");
```
Bagian ini berupa inisialisasi beberapa variabel berupa ``cJSON`` dan ``char`` yang mana masing-masing akan kita operasikan.
```c
cJSON_ArrayForEach(item, genres_tmp) {
    char *name = cJSON_GetObjectItem(item, "name")->valuestring;
    if (strlen(genres) > 0) {
        strcat(genres, ", ");
    }
    strcat(genres, name);
}
```
Fungsi ``cJSON_ArrayForEach(item, genres_tmp)`` ini akan mengecek setiap array yang ada di variabel ``genres_temp`` yang sementara elemen-elemen yang ada didalamnya akan disimpan di variabel ``item``. Lalu, kita lanjutkan mengambil objek berupa ``name`` yang merupakan isi nama-nama dari genre, author, ataupun tema manhwa tersebut. Disini kita tambahkan kondisi ``strlen(genres) > 0`` karena manhwa memungkinkan memiliki lebih dari satu objek nama dari setiap genre, author, ataupun manhwa. Kita gunakan ``strcat()`` untuk secara teratur menambah ke akhir string. Fungsi ini kita gunakan juga untuk mengambil data untuk authors dan themes.
```c
char dir[256] = "Manhwa/";
char filename[128];
buat_direktori("Manhwa");
format_txt(title, filename);
strcat(dir, filename);

FILE *f = fopen(dir, "w");
if (f) {
    fprintf(f, "Title: %s\n", title);
    fprintf(f, "Status: %s\n", status);
    fprintf(f, "Release: %d-%02d-%02d\n", year, month, day);
    fprintf(f, "Genres: %s\n", genres);
    fprintf(f, "Theme: %s\n", themes);
    fprintf(f, "Authors: %s\n", authors);
    fclose(f);
}
```
- Kita inisialisasi variabel ``dir`` yang awalnya hanya berisi ``Manhwa/`` karena untuk isi selanjutnya akan kita isi dengan fungsi ``format_txt()`` yang saya buat diawal untuk mengubah nama judul sesuai kriteria yaitu tanpa spasi dan karakter khusus.
- Disini kita juga menggunakan fungsi ``buat_direktori()`` untuk memudahkan kita dalam membuat direktori baru tanpa ``mkdir()``.
- ``fopen(dir, "w")`` disini berguna untuk membuka file .txt yang nama file nya sudah sesuai kriteria dengan access "write" yaitu akan mengganti isi dari txt dengan isi yang baru.
- ``fprintf()`` disini kita gunakan untuk mencatatkan suatu output ke file yang sudah kita buka tadi lalu tidak lupa kita tutup access file nya dengan ``fclose()``.
#### Output
![image](https://github.com/rdtzaa/assets/blob/1b4bb24a0e62baaa6dd4f1c22c2a62c2dfe56213/Sistem%20Operasi/modul-2-task3.png)
### Poin B
Cella ingin agar setiap file `.txt` tadi di-**zip** satu per satu dan disimpan ke dalam folder baru bernama `Archive`. Yang dimana nama masing masing dari zip diambil dari **huruf kapital nama file**.
#### Solusi
```c
void task2(char *json) {
    cJSON *json_data = cJSON_Parse(json);
    cJSON *data = cJSON_GetObjectItem(json_data, "data");
    buat_direktori("Archive");

    char *title = cJSON_GetObjectItem(data, "title_english")->valuestring;
    char dir[256] = "Manhwa/";
    char filename[128];
    char zip[128] = "Archive/";
    int idx = 8;

    format_txt(title, filename);
    strcat(dir, filename);

    for (int i = 0; filename[i] != '\0'; i++) {
        if (filename[i] >= 'A' && filename[i] <= 'Z') {
            zip[idx] = filename[i];
            idx++;
        }
    }
    zip[idx] = '\0';
    strcat(zip, ".zip");

    buat_zip(zip, dir);

    cJSON_Delete(json_data);
}
```
#### Penjelasan
```c
    cJSON *json_data = cJSON_Parse(json);
    cJSON *data = cJSON_GetObjectItem(json_data, "data");
    buat_direktori("Archive");
    char *title = cJSON_GetObjectItem(data, "title_english")->valuestring;
```
Sama halnya dengan poin A disini kita perlu menguraikan dulu data dari JSON yang sudah kita dapatkan lalu kita ambil bagian ``data`` saja. Disini diikut fungsi ``buat_direktori("Archive)`` karena pada poin B ini kita akan menyimpan ke direktori ``Archive``. Lalu kita pertama-tama akan mengambil judul versi bahasa Inggris terlebih dahulu.
```c
char dir[256] = "Manhwa/";
char filename[128];
char zip[128] = "Archive/";
int idx = 8;
```
- ``dir`` variabel ini akan kita gunakan untuk menunjuk direktori dari file .txt yang sudah kita punya.
- ``zip``, variabel ini kita gunakan untuk menyimpan direktori dari zip yang akan kita buat.
- ``filename`` akan kita gunakan untuk menyimpan dari nama zip yang akan dibuat.
- ``idx`` disini sebagai index dari karakter di variabel ``zip``.
```bash
number_pc=$(echo "$ip_address" | awk -F'.' '{print $4}')
```
- Inisialisasi variabel ``number_pc`` yang menyimpan nomor komputer berdasarkan ip address.
- ``echo`` mencetak isi variabel ip address.
- ``awk`` dengan pembatas tanda titik mengambil kolom ke empat yang merupakan angka ip terakhir sebagai nomor komputer.
```c
format_txt(title, filename);
strcat(dir, filename);
```
- Dengan fungsi ``format_txt()`` yang akan mengubah nama judul sesuai dengan kriteria lalu kita gabungkan dengan ``strcat()`` ke variabel ``dir``.
```c
for (int i = 0; filename[i] != '\0'; i++) {
    if (filename[i] >= 'A' && filename[i] <= 'Z') {
        zip[idx] = filename[i];
        idx++;
    }
}
zip[idx] = '\0';
strcat(zip, ".zip");
```
Looping disini untuk mengisi variabel ``zip`` sesuai dengan kriteria nama zip yang diminta yaitu hanya huruf kapital dari judul versi bahasa Inggris dengan cara kita menambahkan kondisi ``filename[i] >= 'A' && filename[i] <= 'Z'``. Lalu di akhir kita akan menambahkan ``.zip`` ke akhir variabel ``dir``.
```c
buat_zip(zip, dir);
```
Fungsi ini akan membuat zip dari file .txt yang berada di folder ``Manhwa/`` dan akan di zip ke folder ``Archive/``.
#### Output
![image](https://github.com/rdtzaa/assets/blob/53d6eb666317ec2684290fab5187f9157e05fc39/Sistem%20Operasi/modul-2-task3_b.png)
### Poin C
Setiap manhwa memiliki heroine alias **Female Main Character (FMC)**. Cella ingin mengunduh gambar heroine dari internet, dengan jumlah unduhan sesuai dengan **bulan rilis manhwa**.

**Contoh:**

- Jika rilis bulan Februari → unduh **2 foto**
- Jika rilis bulan Desember → unduh **12 foto**
- Format nama file: `Heroine_1.jpg`, `Heroine_2.jpg`, dst.

Selain itu, Cella ingin melakukan pengunduhan **sesuai urutan** daftar manhwa yang tertera pada deskripsi di atas, dan proses pengunduhan harus menggunakan **thread**, karena Cella malas menunggu. Sebagai contohnya, gambar heroine dari manhwa Mistaken as the Monster Duke's Wife harus diunduh terlebih dahulu dan tidak boleh didahului oleh gambar heroine dari manhwa lainnya.

Seluruh gambar akan disimpan dalam folder Heroines. Di dalam folder Heroines, akan terdapat subfolder dengan nama depan atau nama panggilan heroine dari masing-masing manhwa.

Struktur folder yang diinginkan:

```
Heroines/
├── Alisha/
│   ├── Alisha_1.jpg
│   └── Alisha_2.jpg
└── Dorothea/
    ├── Dorothea_1.jpg
    └── Dorothea_2.jpg
```
#### Solusi
```c
struct thread {
    char *folder;
    char *name;
    char *url;
    int jumlah;
};

pthread_mutex_t download;

void *download_heroine(void *arg) {
    pthread_mutex_lock(&download);
    struct thread *data = (struct thread *)arg;
    for (int i = 1; i <= data->jumlah; i++) {
        char filename[256];
        snprintf(filename, sizeof(filename), "%s/%s_%d.jpg", data->folder, data->name, i);
        pid_t pid = fork();
        if (pid == 0) {
            char *args[] = {"curl", "-s", "-L", data->url, "-o", filename, NULL};
            printf("%s terdownload\n", filename);
            execv("/usr/bin/curl", args);
            exit(1);
        } else {
            wait(NULL);
        }
    }
    pthread_mutex_unlock(&download);
    return NULL;
}

void task3_all(char *json[], int jumlah_manhwa) {
    buat_direktori("Heroines");

    char *fmc[4] = {"Dellis", "Artizea", "Adelia", "Ophelia"};
    char *url[4] = {
        "https://cdn.anime-planet.com/characters/primary/lia-dellis-1-285x399.webp?t=1741126489",
        "https://static.wikia.nocookie.net/the-villainess-lives-twice/images/e/e1/ArtizeaRosan.jpg/revision/latest?cb=20210407162325",
        "https://i.pinimg.com/736x/96/bc/1c/96bc1c48cfa6ce0579495eca31ebf775.jpg",
        "https://cdn.anime-planet.com/characters/primary/ophelia-lizen-1-285x399.webp?t=1744234317"
    };

    pthread_t threads[jumlah_manhwa];

    for (int i = 0; i < jumlah_manhwa; i++) {
        cJSON *json_data = cJSON_Parse(json[i]);
        cJSON *data = cJSON_GetObjectItem(json_data, "data");

        int month = 1;
        cJSON *published = cJSON_GetObjectItem(data, "published");
        cJSON *prop = cJSON_GetObjectItem(published, "prop");
        cJSON *from = cJSON_GetObjectItem(prop, "from");
        month = cJSON_GetObjectItem(from, "month")->valueint;


        char folder[128];
        snprintf(folder, sizeof(folder), "Heroines/%s", fmc[i]);
        buat_direktori(folder);

        struct thread *data_thread = malloc(sizeof(struct thread));
        data_thread->folder = strdup(folder);
        data_thread->name = strdup(fmc[i]);
        data_thread->url = strdup(url[i]);
        data_thread->jumlah = month;

        pthread_create(&threads[i], NULL, download_heroine, data_thread);
        cJSON_Delete(json_data);
    }

    for (int i = 0; i < jumlah_manhwa; i++) {
        pthread_join(threads[i], NULL);
    }
    pthread_mutex_destroy(&download);
}
```
#### Penjelasan
```bash
str1=$(awk -F' ' 'BEGIN {months["Jan"]="01"}
$9 ~ 500 {
    tgl = substr($4, 2, 11)
    split(tgl, splitted, "/")
    peminjam = months[splitted[2]] "/" splitted[1] "/" splitted[3] " " substr($1, 11, 1)
    count[peminjam]++
}
END {for (i in count) print i " " count[i]}' access.log)
```
- Inisialisasi variabel ``str1`` yang akan diisi log dengan ``Status Code 500`` dikelompokkan berdasarkan tanggal dan nomor komputer.
- ``$9 ~ 500`` mencari baris yang memenuhi kondisi untuk melanjutkan fungsi.
- ``tgl = substr($4, 2, 11)`` dan ``split(tgl, splitted, "/")`` untuk memisahkan perbagian tanggal dari ``access.log`` dengan menghilangkan tanda garis miring.
- ``peminjam = months[splitted[2]] "/" splitted[1] "/" splitted[3] " " substr($1, 11, 1)`` membuat variabel peminjam sebagai index dengan format ``MM/DD/YYYY [Nomor Komputer]``
- ``count[peminjam]++`` sebuah counter dengan index peminjam.
- ``for (i in count) print i " " count[i]}' access.log`` mencetak tanggal, nomor komputer, dan jumlah aktivitas.
```
while read tanggal komputer aktivitas; do
    index=$(awk -v tgl=$tanggal -v komp=$komputer -F',' '{if ($1 ~ tgl && $2 ~ komp) print $3}' peminjaman_computer.csv)
    ((count[$index]+=$aktivitas))
    # echo "${count[$index]}"
done <<< "$str1"
```
- ``while read tanggal komputer aktivitas; do ... <<< "$str1"`` membaca tiap baris dari variabel ``str1`` dengan pemisah default nya adalah tanda spasi yang kemudian dimasukkan ke variabel tanggal, komputer, dan aktivitas.
- ``index=$(awk -v tgl=$tanggal -v komp=$komputer -F',' '{if ($1 ~ tgl && $2 ~ komp) print $3}' peminjaman_computer.csv)`` membuat index berupa nama peminjam yang dicocokkan dengan tanggal dan nomor komputer yang ada di dalam data ``peminjaman_computer.csv``
- ``((count[$index]+=$aktivitas))`` counter untuk menambah aktivitas berdasarkan nama peminjam.
```bash
for peminjam in "${!count[@]}"; do
    echo "$peminjam mendapatkan Status Code 500 sebanyak ${count[$peminjam]} kali"
    ((total+=count[$peminjam]))
    if [[ $max_count -lt ${count[$peminjam]} ]]; then
        max_count=${count[$peminjam]}
        max_name=$peminjam
    fi
done
```
- Disini kita melakukan looping terhadap index count dengan cara ``${count[@]}``
- ``echo`` disini untuk mencetak banyaknya ``Status Code 500`` yang ditemukan setiap orang melalui variabel peminjam
- ``((total+=count[$peminjam]`` untuk menjumlah semua ``Status Code 500`` yang ditemukan.
- ``if``, kondisi ini untuk menentukan siapa yang paling banyak menemukan sehingga berhak mendapat hadiah.
```bash
echo "Total Status Code yang ditemukan adalah $total kali"
echo "Selamatt!!! $max_name mendapatkan hadiah dari Rudi"
```
Menampilkan total dan nama teman Rudi yang berhak mendapat hadiah.
#### Output
![image](https://github.com/rdtzaa/assets/blob/e603d07387e822ee2f74d0e51ffaf069a5e84929/Sistem%20Operasi/rudi-c.png)
### Kendala
Pada poin C, saya sempat kesulitan untuk menentukan counter setiap ip karena ternyata di data ``peminjaman_komputer.csv`` setiap teman Rudi bisa berpindah komputer. Oleh karena itu, saya mengelompokkan terlebih dahulu berdasarkan tanggal dan IP lalu tanggal dan IP dicocokkan dengan data ``peminjaman_komputer.csv`` dan index counter adalah nama teman-teman nya.
