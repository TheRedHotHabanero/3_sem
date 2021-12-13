# 3_sem
## [1_stat](https://github.com/TheRedHotHabanero/3_sem/tree/master/1_stat)
Аналог утилиты stat. Формат вывода в меру испорченности. Надо знать отличия stat от lstat от statx от fstat от fstatat.
Ботаем маны, достаем недостающие:
manpages
manpages-dev
manpages-posix
manpages-posix-dev

## [2_open_write_close](https://github.com/TheRedHotHabanero/3_sem/tree/master/2_open_write_close)
### Задача 2.1
Разобрана на паре. Записать в указанный файл указанное содержимое. Изучаем open, write, close.
### Задача 2.2
Если нужно записать текст, причем форматированный, на руках только файловый дескриптор. Есть fdopen (больше удобства), есть dprintf( по сути берет 2.1, но open + dprintf %s + close. Решение 2.1 через open+dprintf+close

## [3_copyfiles](https://github.com/TheRedHotHabanero/3_sem/tree/master/3_copy_files)
### Задача 3.1
Простое копирование простых(обычных файлов) файлов open + read + close + write. Копирование содержимого файла. Копируем все что открывается успешно на чтение. Что не открывается и тд, просто выдаем ошибку. Write может выйти с успехом, но при этом записать не все файлы. Единицы - десятки мегабайт (?) размер буфера. Задаем буфер с учетом своих пожеланий, write возвращает сколько он записал.
### Задача 3.2
p_read, p_write. 3.1 через p_read + p_write. Копирование с переставлением блоков в обратном порядке. Узнать о непоследовательном чтении и записи — как лучше, эффективнее и тд. Fstat (?), lseek. Берем образ диска и читаем — уже веселее. Для файловой системы что лежит в корневом каталоге. Найти этот сектр и вывести находящиеся в нем строки. Читаем на вики про файловую систему. Yадо как-нибудь проиллюстрировать эти функции, чтобы понять, что можно попроще, чем pread, pwrite, lseek. Куда, сколько и начиная с какой позиции.
### Задача 3*
Задача 3.1 + несколько строк. А давайте теперь проверять, что обычный файл, а не каталог и тд. Новая сущность — системный вывод mknod + mkfifo. Копирование именованный файлов — FIFO.  readlink/symlink

## [4_metadata_copy](https://github.com/TheRedHotHabanero/3_sem/tree/master/4_metadara_copy)
### Задача 4
Изволь копировать не только данные, но и метаданные. Права, время доступа и модификации — это надо научиться копировать. Доработка задачи 3*. Задача 3* + копирование прав доступа и времен обращения, модификации. Один вариант — передавать прям в вызов open оригинальные права, но может быть такое, что мы скопировали право «только на чтение» и начинаем сразу там писать. Такое скорее всего сработает, но выглядеть будет не очень. Еще вариант fchmod (chmod) + futimens.

## [5_extra_fchown](https://github.com/TheRedHotHabanero/3_sem/tree/master/5_extra_fchown)
### Задача 5*
Номер идентификатор владельца, главной группы владельца. Дорабатываем 4 + скопировать uid, gid, на что может не хватить прав, но пытаемся сделать как-то, а если не получится, то делать предупреждения, а не фатал. Fchown. В предельном случае делаем утилиту rsync. Неплохо бы, чтобы жесткие ссылки оставались жесткими ссылками. Узлы и все такое — если ссылок больше одной. Unordered map для плюсов. Map еще. Чтобы не повторяться при копировании. Fchown + link (linkat) + ACL. Man getfacl(1)+  xattr + hardlinks.
Еще есть chattr, sattr.

## [6_readdir](https://github.com/TheRedHotHabanero/3_sem/tree/master/6_readdir)
### Задача 6.1
opendit + readdir + closedir (выводит только имя текущего каталога и чет еще). Выводит в консоль список всех записей в текущей дериктории (включая скрытые файлы, родительский каталог и текущий каталог). Разобрана на паре
### Задача 6.2
Реализовать 6.1 с учетом argv[1]. Open + fdopendir, opendir + dirfd + readdir + fstat + close. Принимает абсолютный путь к каталогу и выводит в консоль список записей в нем (включая скрытые файлы, родительский каталог и текущий каталог). Если не передавать программе путь к файлу, по умолчанию будет выведен список записей в текущем каталоге. Разобрана на паре.
### Задача 6.3
Решить 6.2 через open+getdents64 +close. Помним про размер буфера, смысл в ускорении чтения больших каталогов. Принимает абсолютный путь к каталогу и с помощью рекурентного обхода дерева каталогов выводит в консоль все записи до самого нижнего уровня. Если не передавать путь, будет выведен список записей в текущем каталоге.
### Задача 6.4
Рекурсивная 6.2

## [7_copydir](https://github.com/TheRedHotHabanero/3_sem/tree/master/7_copydir)
### Задача 7
Cовместить 5 и 6 — полноценный копировщик. Копирует рекурсивно с сохранением максимум метаданных.

## [8_hdparm](https://github.com/TheRedHotHabanero/3_sem/tree/master/8_hdparm)
### Задача 8*
Физическое размещение файла на диске. Filefrag (hdparm - - fibmap). Есть ли общие блоки с другими.

## [9_statvfs](https://github.com/TheRedHotHabanero/3_sem/tree/master/9_statvfs)
### Задача 9
Классика, которую знать надо. Предлагается посмотреть информацию о файловой системе. В качестве параметра путь к каталогу. Statfs или statvfs — кто хороший, а кого не надо использовать, кого мы используем и тд. Вывести размер файловой системы: занятое, свободное и доступное (подмножество свободное) пространство для файловой системы, на которой расположен указанный файл или каталог. Обычно нас интересует каталог
### Задача 9*
Веселее когда разным пользователям доступно разное место. Quota/ quotactl. Есть одноименные маны.

## [10_inotify](https://github.com/TheRedHotHabanero/3_sem/tree/master/10_inotify)
### Задача 10
inotify. Программа, которая создает очередь init, подписывается на каталог add watch и вываливает события с этим каталогом — сообщает о появлении новым файлом: ocreat. Крч не был файл и теперь стал новый файл. А переименование надо отметать. Если переименование = переместили старый файл в новый ктаалог, то это норм.
Выкидыванием кода из мана получается на 99 процентов решение. Старый есть dnotify, какой куда лучше подходит надо б знать программа печатает информацию о появлении новых файлов в указанном каталоге.

## [11_flock](https://github.com/TheRedHotHabanero/3_sem/tree/master/11_flock)
### Задача 11
Программа, которая подсчитывает, сколько раз она запускалась.

## [12_process](link)
### Задача 12
Вывести информацию про текущий процесс (про запущенный экземпляр этой программы). Выводить по порядку как в credentials(7) -- иерархия процессов, группы (по кучкам), getgroups, getrlimits (получает предельные значения ресурса для текущего процесса), getrusageт (использование ресурсов процесса), scheduler (планирощик),  номер процессора выполнения, количество процессоров)

## [13_waiting](link)
### Задача 13.1
Форкнуть дочерний процесс, подождать его завершение и вывести информацию о родителе и ребенке + посмотреть разные ситуации завершения процесса. По сигналу, по ^c по ^z, посмотреть, когда вызывается кор дамп. getpid(), getppid(), getpgid(0), getsid(0), waitpid (дожидается изменения состояния дочернего процесса). Пример в man 2 waitpid
## Задача 13.2
Программа порождает дочерний процесс, он дожидается завершения родительского процесса. Мониторить parent_pid если он изменился, то родитель умер busybox. parent pid на 1 меньше текущего, когда родитель жив. Когда parent умирает, pid дочернего меняется. ptrace -- . pidfd_open -- самый короткий, но linux specific, можно его. wait нельзя использовать. fork - создает новый процесс. clone (2) - копия исходного процесса. fork + execve. posix_spawn

## [14_pipe](link)
## Задача 14.1
Разобрана на семинаре. dup2(oldfd, newfd) -- в дескриптор newfd (первый свободный) подсунуть копию файла oldfd
## Задача 14.2
Разобрана на семинаре. execve, execlp.
## Задача 14.3
Разобрана на семинаре. Вывести то же самое, что last | wc -l. pipe. Реализовать использование других программ из своей (на основе 14.1). Соединить stdout одной программы с stdin другой программы(команды пишут на вход друг другу)
## Задача 14.4
Синтез из предыдущих трех. poll/epoll/на худой конец select(это старый). Написать программу, которая в дочернем процессе запускает gzip, работая с ним через два пайпа; померять скорость, с которой gzip сжимает случайные данные. gzip -- сжать. gzip -d -- разжать. clock_gettime -- получить разность времен, поделить на него количество закинутых данных -- получится скорость. Предлагается использовать два pipe и кормить тот же gzip случайными данными (с выводом средней скорости сжатия и коэффициента сжатия), ловя взаимоблокировки и научившись их правильно обрабатывать, т.к. в любой момент gzip может как висеть на чтение входного pipe (ему нужно больше данных, а в буфере они уже закончились), так и на записи в выходной (он выдал порцию сжатых данных, а родительский процесс их до сих пор не прочёл).
## Задача 14.0
14.1/2/3 были по сути разобраны на семинаре. Предлагается это собрать воедино и, например, использовать программу gzip для сжатия/распаковки данных на лету с выводом, например, информации о скорости в байтах в секунду.

