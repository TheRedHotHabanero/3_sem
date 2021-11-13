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

## [6_readdir](link)
### Задача 6.1
opendit + readdir + closedir (выводит только имя текущего каталога и чет еще). Выводит в консоль список всех записей в текущей дериктории (включая скрытые файлы, родительский каталог и текущий каталог).
### Задача 6.2
Реализовать 6.1 с учетом argv[1]. Open + fdopendir, opendir + dirfd + readdir + fstat + close. Принимает абсолютный путь к каталогу и выводит в консоль список записей в нем (включая скрытые файлы, родительский каталог и текущий каталог). Если не передавать программе путь к файлу, по умолчанию будет выведен список записей в текущем каталоге.
### Задача 6.3
Решить 6.2 через open+getdents64 +close. Помним про размер буфера, смысл в ускорении чтения больших каталогов. Принимает абсолютный путь к каталогу и с помощью рекурентного обхода дерева каталогов выводит в консоль все записи до самого нижнего уровня. Если не передавать путь, будет выведен список записей в текущем каталоге.
### Задача 6.4
Добыть нормальное описание + рекурсивная 6.2