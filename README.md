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

## [3_copyfiles](link)
### Задача 3.1
Простое копирование простых(обычных файлов) файлов open + read + close + write. Копирование содержимого файла. Копируем все что открывается успешно на чтение. Что не открывается и тд, просто выдаем ошибку. Write может выйти с успехом, но при этом записать не все файлы. Единицы - десятки мегабайт (?) размер буфера. Задаем буфер с учетом своих пожеланий, write возвращает сколько он записал.