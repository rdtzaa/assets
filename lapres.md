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
```bash
str1=$(cat access.log | awk '{print $1}'
```
- ``str1=${...}`` untuk menyimpan data hasil operasi yang berada di dalam tanda kurung kurawal.
- ``cat.access.log`` untuk mencetak isi dari file ``access.log`` berupa log akses sebuah web.
- ``awk '{print $1}'`` untuk mencetak kolom pertama dari hasil dari cetakan ``cat`` sebelumnya yaitu berupa ip address.
```bash
awk '{count[$1]++} END {for (ip in count) print "Jumlah request dari", ip, "=", count[ip]}')
```
Fungsi ``awk`` disini membuat sebuah variabel yang akan increment dengan index yang berasal dari kolom pertama hasil cetakan operasi sebelumnya. Di bagian ``END`` yang akan dijalankan setelah semua baris data dilewati, kita menambahkan looping terhadap index yang terdapat pada variabel ``count`` dengan cara ``(ip in count)`` lalu kita cetak jumlah request setiap ip.
```bash
str2=$(cat access.log | awk '{count[$9]++} END {for (code in count) print "Jumlah status code", code, "=", count[code]}')
```
- ``str2=${....}`` untuk menyimpan data hasil operasi yang berada di dalam tanda kurung kurawal.
- ``cat.access.log`` untuk mencetak isi dari file ``access.log`` berupa log akses sebuah web.
- ``awk`` disini diikuti operasi seperti di variabel ``str1`` yaitu kita membuat variabel counter tetapi disini index nya adalah kolom ke sembilan dari file log yaitu status code nya. Di bagian ``END`` hampir sama dengan variabel ``str1`` juga, kita akan melakukan looping terhadap index-index yang ada dalam variabel ``count`` dengan cara ``(code in count)`` lalu kita cetak jumlah dari setiap status code.
#### Output
![image](https://github.com/rdtzaa/assets/blob/e603d07387e822ee2f74d0e51ffaf069a5e84929/Sistem%20Operasi/rudi-a.png)
### Poin B
Karena banyaknya status code error, Rudi ingin tahu siapa yang menemukan error tersebut. Setelah melihat-lihat, ternyata IP komputer selalu sama. Dengan bantuan [peminjaman_komputer.csv](https://drive.google.com/file/d/1-aN4Ca0M3IQdp6xh3PiS_rLQeLVT1IWt/view?usp=drive_link), Rudi meminta kamu untuk membuat sebuah program bash yang akan menerima inputan tanggal dan IP serta menampilkan siapa pengguna dan membuat file backup log aktivitas, dengan format berikut:

- **Tanggal** (format: `MM/DD/YYYY`)

- **IP Address** (format: `192.168.1.X`, karena menggunakan jaringan lokal, di mana `X` adalah nomor komputer)

- Setelah pengecekan, program akan memberikan **message pengguna dan log aktivitas** dengan format berikut:

  ```
  Pengguna saat itu adalah [Nama Pengguna Komputer]
  Log Aktivitas [Nama Pengguna Komputer]
  ```

  atau jika data tidak ditemukan:

  ```
  Data yang kamu cari tidak ada
  ```

- File akan disimpan pada directory “/backup/[Nama file]”, dengan format nama file sebagai berikut

  ```
  [Nama Pengguna Komputer]_[Tanggal Dipilih (MMDDYYY)]_[Jam saat ini (HHMMSS)].log
  ```

- Format isi log

  ```
  [dd/mm/yyyy:hh:mm:ss]: Method - Endpoint - Status Code
  ```
#### Solusi
```bash
#!/bin/bash

convert_date_format() {
    temp_date=$1
    month=${temp_date:0:2}
    day=${temp_date:3:2}
    year=${temp_date:6:4}

    months=("Jan")
    month_name=${months[$((month-1))]}
    formatted_date="$day/$month_name/$year"

    echo "$formatted_date"
}

echo -e "Masukkan tanggal (MM/DD/YYYY)"
read tanggal
echo -e "Masukkan IP Address (192.168.1.X)"
read ip_address

number_pc=$(echo "$ip_address" | awk -F'.' '{print $4}')

echo "Nomor komputer berdasarkan IP adalah: $number_pc"

nama_pengguna=$(grep "$tanggal,$number_pc" peminjaman_computer.csv | awk -F',' '{print $3}')

if [ -z "$nama_pengguna" ]; then
    echo "Nama pengguna tidak ditemukan"
else
    echo "Pengguna saat itu adalah $nama_pengguna"
    echo "Log Aktivitas $nama_pengguna"
    formatted_date=$(convert_date_format "$tanggal")
    waktu=$(date | awk '{print $4}' | awk -F':' '{print $1$2$3}')
    dir="${nama_pengguna}_$(echo $tanggal | awk -F'/' '{print $1$2$3}')_$waktu.log"
    mkdir -p "backup"
    awk -v tgl=$formatted_date -v ip=$ip_address '
        $1 == ip && $4 ~ tgl {
        date = substr($4, 2, 20)
        metohod = substr($6, 2)
        print "[" date "]:", metohod, "-", $7, "-", $9}' access.log > backup/$dir
    echo "$dir berhasil dibuat"
fi
```
#### Penjelasan
```bash
convert_date_format() {
    temp_date=$1
    month=${temp_date:0:2}
    day=${temp_date:3:2}
    year=${temp_date:6:4}

    months=("Jan")
    month_name=${months[$((month-1))]}
    formatted_date="$day/$month_name/$year"

    echo "$formatted_date"
}
```
Inisialisasi fungsi ``convert_date_format()`` untuk mengganti format tanggal dari ``MM/DD/YYYY`` menjadi ``DD/nama bulan/YYYY``.
- ``month=${temp_date:0:2}``, assign variabel ``month`` dengan cara mengambil 2 huruf mulai index 0 berupa bulan dari sebuah input tanggal fungsi tersebut.
- ``day=${temp_date:3:2}``, assign variabel ``day`` dengan cara mengambil 2 huruf mulai index 3 berupa hari dari sebuah input tanggal fungsi tersebut.
- ``year=${temp_date:6:4}``, assign variabel ``year`` dengan cara mengambil 4 huruf mulai index 6 berupa tahun dari sebuah input tanggal fungsi tersebut.
- ``months=("Jan")`` variabel array untuk convert bulan dari angka menjadi nama bulan.
- ``month_name=${months[$((month-1))]}``, operasi untuk convert angka bulan dengan cara memasukkan angka bulan yang sudah kita simpan di variabel ``month`` ke dalam index lalu kita kurangi satu karena array dimulai dari index 0 sedangkan angka bulan Januari adalah "1".
- ``formatted_date="$day/$month_name/$year"`` assign ke sebuah variabel dengan format yang sesuai yaitu ``DD/nama_bulab/YYYY``
- ``echo "$formatted_date"``, fungsi untuk mencetak hasil dari convert tanggal.
```bash
echo -e "Masukkan tanggal (MM/DD/YYYY)"
read tanggal
echo -e "Masukkan IP Address (192.168.1.X)"
read ip_address
```
- ``echo`` bagian tersebut merupakan bagian untuk menerima input diawal dengan tambahan ``-e`` agar ketika di dalam terminal inputan kita dapat berada di samping cetakan ``echo``.
- ``read`` disini berfungsi untuk menyimpan hasil input dari user kedalam suatu variabel.
- Input yang diterima disini terdapat tanggal dengan format ``(MM/DD/YYYY)`` dan ip address dengan format ``(192.168.1.X)``
```bash
number_pc=$(echo "$ip_address" | awk -F'.' '{print $4}')
```
- Inisialisasi variabel ``number_pc`` yang menyimpan nomor komputer berdasarkan ip address.
- ``echo`` mencetak isi variabel ip address.
- ``awk`` dengan pembatas tanda titik mengambil kolom ke empat yang merupakan angka ip terakhir sebagai nomor komputer.
```bash
nama_pengguna=$(grep "$tanggal,$number_pc" peminjaman_computer.csv | awk -F',' '{print $3}')
```
- ``nama_pengguna`` sebuah variabel yang digunakan untuk menyimpan nama pengguna komputer berdasarkan ip address dan tanggal peminjaman.
- ``grep`` disini akan mengambil baris data dari ``peminjaman_computer.csv`` yang memiliki pola ``"$tanggal,$number_pc"`` sehingga kita dapat mencetak nama pengguna yang berada di kolom ke 3 file csv tersebut.
```bash
if [ -z "$nama_pengguna" ]; then
    echo "Nama pengguna tidak ditemukan"
else
    echo "Pengguna saat itu adalah $nama_pengguna"
    echo "Log Aktivitas $nama_pengguna"
```
``if [ -z .... ]`` berfungsi untuk melihat apakah variabel tersebut kosong atau tidak. Sesuai ketentuan, kita akan mencetak "Nama pengguna tidak ditemukan" karena variabel ``nama_pengguna`` kosong yang artinya tidak ada kecocokan di file ``peminjaman_computer.csv`` begitu juga sebaliknya akan mengeluarkan output sesuai ketentuan soal diatas.
```bash
waktu=$(date | awk '{print $4}' | awk -F':' '{print $1$2$3}')
```
Variabel waktu ini akan diisi waktu saat script ini dijalankan dengan menggunakan command ``date`` yang kemudian akan kita ambil bagian waktu dan memisahkan tanda titik dua nya juga dengan ``awk``
```bash
dir="${nama_pengguna}_$(echo $tanggal | awk -F'/' '{print $1$2$3}')_$waktu.log"
```
Dengan format file yang diminta adalah ``[Nama Pengguna Komputer]_[Tanggal Dipilih (MMDDYYY)]_[Jam saat ini (HHMMSS)].log`` maka saya masukkan bagian nama penggunakan komputer dengan variabel ``nama_pengguna``, tanggal dengan variabel ``$tanggal`` yang ditambahkan ``awk`` untuk menghilangkan tanda garis miring, dan jam saat ini dengan variabel ``waktu``.
```bash
mkdir -p "backup"
```
- ``mkdir`` disini untuk membuat folder bernama "backup" yang akan digunakan untuk menyimpan file log yang dicari dan disini ditambah ``-p`` yang mana ini untuk cek apakah directory tersebut ada dan jika ada dia tidak akan membuat folder lagi dan sebaliknya juga.
```bash
awk -v tgl=$formatted_date -v ip=$ip_address '
  $1 == ip && $4 ~ tgl {
  date = substr($4, 2, 20)
  metohod = substr($6, 2)
  print "[" date "]:", metohod, "-", $7, "-", $9}' access.log > backup/$dir
```
- ``awk -v [variabel]`` disini untuk membuat file log berdasarkan format dan kita juga menambahkan suatu variabel ke dalam ``awk``
- ``$1 == ip && $4 ~ tgl`` kondisi ini akan cek apakah kolom pertama dan kolom keempat dari tiap baris data ``access.log`` sudah sesuai dengan ip address dan tanggal yang ditentukan maka akan dilanjut proses yang ada di dalam tanda kurung kurawal setelahnya.
- ``date = substr($4, 2, 20)`` variabel date ini akan diisi dengan tanggal beserta hari sesuai format isi log yang diharapkan yaitu ``[dd/mm/yyyy:hh:mm:ss]``
- ``metohod = substr($6, 2)`` variabel method ini akan diisi oleh metode log akses web seperti ``DELETE``.
- ``print "[" date "]:", metohod, "-", $7, "-", $9}' access.log > backup/$dir`` disini akan mencetak data dengan format yang diharapkan yaitu ``[dd/mm/yyyy:hh:mm:ss]: Method - Endpoint - Status Code`` lalu dimasukkan ke dalam file log sesuai format juga.
#### Input & Output
![image](https://github.com/rdtzaa/assets/blob/e603d07387e822ee2f74d0e51ffaf069a5e84929/Sistem%20Operasi/rudi-b.png)
##### ``Caca_01262025_203527.log``
```
[26/Jan/2025:00:02:06]: GET - /index.html - 200
[26/Jan/2025:00:02:11]: GET - /login - 500
[26/Jan/2025:00:02:13]: PUT - /contact - 200
[26/Jan/2025:00:06:38]: PUT - /about.html - 200
[26/Jan/2025:00:09:24]: DELETE - /about.html - 200
[26/Jan/2025:00:11:13]: PUT - /contact - 200
[26/Jan/2025:00:11:56]: GET - /login - 200
[26/Jan/2025:00:12:48]: POST - /index.html - 200
[26/Jan/2025:00:16:05]: POST - /about.html - 302
[26/Jan/2025:00:17:47]: PUT - /login - 302
[26/Jan/2025:00:20:23]: PUT - /about.html - 404
...
```
### Poin C
Rudi ingin memberikan hadiah kepada temannya yang sudah membantu. Namun karena dana yang terbatas, Rudi hanya akan memberikan hadiah kepada teman yang berhasil menemukan server error dengan ``Status Code 500`` terbanyak. Bantu Rudi untuk menemukan siapa dari ketiga temannya yang berhak mendapat hadiah dan tampilkan jumlah ``Status Code 500`` yang ditemukan
#### Solusi
```bash
#!/bin/bash
declare -A count

str1=$(awk -F' ' 'BEGIN {months["Jan"]="01"}
$9 ~ 500 {
    tgl = substr($4, 2, 11)
    split(tgl, splitted, "/")
    peminjam = months[splitted[2]] "/" splitted[1] "/" splitted[3] " " substr($1, 11, 1)
    count[peminjam]++
}
END {for (i in count) print i " " count[i]}' access.log)

while read tanggal komputer aktivitas; do
    index=$(awk -v tgl=$tanggal -v komp=$komputer -F',' '{if ($1 ~ tgl && $2 ~ komp) print $3}' peminjaman_computer.csv)
    ((count[$index]+=$aktivitas))
    # echo "${count[$index]}"
done <<< "$str1"

for peminjam in "${!count[@]}"; do
    echo "$peminjam mendapatkan Status Code 500 sebanyak ${count[$peminjam]} kali"
    ((total+=count[$peminjam]))
    if [[ $max_count -lt ${count[$peminjam]} ]]; then
        max_count=${count[$peminjam]}
        max_name=$peminjam
    fi
done

echo "Total Status Code yang ditemukan adalah $total kali"
echo "Selamatt!!! $max_name mendapatkan hadiah dari Rudi"
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
