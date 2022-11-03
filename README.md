# low-level-programming-labs

ИТМО, ПИиКТ 3 курс. Низкоуровневое программирование

## Сборка примера

```bash
git clone https://github.com/deevroman/low-level-programming-labs.git && cd low-level-programming-labs
# сборка модуля
cmake -B examples/llp -DCMAKE_BUILD_TYPE=Release llp
cmake --build llp/build --config Release
# сборка примера
cmake -B examples/llp -DCMAKE_BUILD_TYPE=Release llp
cmake --build examples/build --config Release
# запуск
./examples/bin/*/*
```

### Уже собранное под все платформы
https://github.com/deevroman/low-level-programming-labs/releases

### Рецепт 
https://github.com/deevroman/low-level-programming-labs/blob/master/.github/workflows/cmake.yml

---

<img src="img/mudroe-tainstvennoe-derevo-mem.jpg" width="50">

Вариант 1. Форма данных: документное дерево


<table>
<tr>
	<td> Организация элементов данных
	<td> Дерево узлов несущих свойства
<tr>
	<td> Способ реализации отношений
	<td> Материализованы в представлении родитель-ребенок
<tr>
	<td> Примеры
	<td> Json, Xml, registry, прикладной уровень файловых систем
<tr>
	<td> Состав схемы данных
	<td> Виды узлов, виды значений в узлах
<tr>
	<td> Состав модели фильтра данных
	<td> Условия по содержимому элементов данных и отношениям между ними
<tr>
	<td> Примеры языков запросов
	<td> XPath, XQuery
</table>

Настоятельно рекомендуется:

- В начале работы подготовить окружение разработчика, включающее отладчик, поддерживающий визуализацию структур данных и
  возможность отладки программ, выполняющихся под управлением ОС семейств Windows и *NIX.
- Сборку проекта осуществлять с помощью кроссплатформенных средств автоматизации, таких как мэйкфайлы.
- Следовать общим принципам грамотной разработки ПО, таким как SOLID, DRY, и др., грамотно использовать непрозрачные
  типы данных (opaque data types), разделять публичную и приватную функциональность модулей.

Оформление отчетов
По каждому из заданий должен быть представлен отчет, содержащий следующие части:

1. Цели – описание цели задания (см. текст задания)
2. Задачи – путь достижения цели, что именно нужно было сделать для выполнения задания (план хода
   вашей работы)
3. Описание работы – внешнее описание созданной программы, состава модулей, способов её
   использования, примеры входной и выходной информации (модули, интерфейсы, тесты)
4. Аспекты реализации – внутреннее описание созданной программы, особенности алгоритмов,
   примеры кода
5. Результаты – что было сделано для выполнения задач кратко по пунктам
   (созданные артефакты, результаты тестов, количественные оценки)
6. Выводы – что было достигнуто в отношении цели задания.
   (что показали тесты, почему, как это было достигнуто, чему научились, качественные оценки)

## Задание 1

Создать модуль, реализующий хранение в одном файле данных (выборку, размещение и гранулярное обновление) информации
общим объёмом от 10GB соответствующего варианту вида.
Порядок выполнения:

### 1. Спроектировать структуры данных для представления информации в оперативной памяти

- Для порции данных, состоящий из элементов определённого рода (см форму данных), поддержать тривиальные значения по
  меньшей мере следующих типов: четырёхбайтовые целые числа и числа с плавающей точкой, текстовые строки произвольной
  длины, булевские значения
- Для информации о запросе

### 2. Спроектировать представление данных с учетом схемы для файла данных и реализовать базовые операции для работы с ним:

- Операции над схемой данных (создание и удаление элементов схемы)
- Базовые операции над элементами данными в соответствии с текущим состоянием схемы (над
  узлами или записями заданного вида)
    - Вставка элемента данных
    - Перечисление элементов данных
    - Обновление элемента данных
    - Удаление элемента данных

### 3. Используя в сигнатурах только структуры данных из п.1, реализовать публичный интерфейс со следующими операциями над файлом данных:

* Добавление, удаление и получение информации о элементах схемы данных, размещаемых в файле данных, на уровне,
  соответствующем виду узлов или записей
* Добавление нового элемента данных определённого вида
* Выборка набора элементов данных с учётом заданных условий и отношений со смежными
* элементами данных (по свойствам/полями/атрибутам и логическим связям соответственно)
* Обновление элементов данных, соответствующих заданным условиям
* Удаление элементов данных, соответствующих заданным условиям

### 4. Реализовать тестовую программу для демонстрации работоспособности решения

* Параметры для всех операций задаются посредством формирования соответствующих структур данных
* Показать, что при выполнении операций, результат выполнения которых не отражает отношения между элементами данных,
  потребление оперативной памяти стремится к O(1) независимо от общего объёма фактического затрагиваемых данных
* Показать, что операция вставки выполняется за O(1) независимо от размера данных, представленных в файле
* Показать, что операция выборки без учёта отношений (но с опциональными условиями) выполняется за O(n), где n –
  количество представленных элементов данных выбираемого вида
* Показать, что операции обновления и удаления элемента данных выполняются не более чем за O(n*m) > t -> O(n+m), где n –
  количество представленных элементов данных обрабатываемого вида, m – количество фактически затронутых элементов данных
* Показать, что размер файла данных всегда пропорционален количеству фактически размещённых элементов данных
* Показать работоспособность решения под управлением ОС семейств Windows и *NIX

### 5. Результаты тестирования по п.4 представить в виде отчёта

* В части 3 привести описание структур данных, разработанных в соответствии с п.1
* В части 4 описать решение, реализованное в соответствии с пп.2-3
* В часть 5 включить графики на основе тестов, демонстрирующие амортизированные показатели ресурсоёмкости по п. 4