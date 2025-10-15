# Arena Animal Simulation

Программа моделирует вход животных (львов и тигров) на арену в строго определенном порядке с использованием многопоточности.

## 🎯 Функциональность

- Создает `n` львов и `n-1` тигров
- Формирует порядок входа: тигр → лев → тигр → лев → ...
- Запускает потоки для каждого животного
- Синхронизирует вход животных согласно заданному порядку
- После входа всех животных запускает их одновременный выход

## 📥 Входные параметры

- `n` - количество львов (целое число ≥ 1)
- Количество тигров автоматически устанавливается как `n-1`

## 📤 Выходные данные

- Консольный вывод процесса входа и выхода животных
- Сообщения в формате: `"Lion X entered/exited"`, `"Tiger X entered/exited"`

## 🔐 Критические секции

1. **Мьютекс `mtx` в классе `Arena`** - защищает доступ к:
   - переменной `index` (текущая позиция в порядке входа)
   - флагу `all_entered`
   - условию входа животных

2. **Мьютекс `cout_mtx`** - синхронизирует вывод в консоль

## 🧵 Потоки

- **Главный поток** - инициализация и управление программой
- **Потоки животных** - по одному для каждого льва и тигра:
  - Ожидают своей очереди на вход
  - Выполняют вход при выполнении условия
  - Ожидают сигнал выхода
  - Имитируют задержку выхода (10-100 мс)
  - Выполняют выход

## 🚀 Запуск

```bash
g++ -std=c++11 -pthread -o arena main.cpp
./arena
```

Программа запросит ввод значения `n`, после чего запустит симуляцию.


```console.log
=== QUICK PRE-TESTS ===
Test 2: Order Generation... PASSED SUCCESSFULLY
=== QUICK PRE-TESTS FINISHED ===

Enter n (number of lions, n>=1): 10

--- Simulation started ---

Tiger 0 entered.
Lion 0 entered.
Tiger 1 entered.
Lion 1 entered.
Tiger 2 entered.
Lion 2 entered.
Tiger 3 entered.
Lion 3 entered.
Tiger 4 entered.
Lion 4 entered.
Tiger 5 entered.
Lion 5 entered.
Tiger 6 entered.
Lion 6 entered.
Tiger 7 entered.
Lion 7 entered.
Tiger 8 entered.
Lion 8 entered.
Lion 9 entered.
Lion 1 exited.
Lion 9 exited.
Lion 4 exited.
Tiger 1 exited.
Lion 7 exited.
Tiger 4 exited.
Tiger 6 exited.
Tiger 8 exited.
Tiger 3 exited.
Lion 3 exited.
Lion 8 exited.
Lion 6 exited.
Lion 5 exited.
Lion 0 exited.
Tiger 2 exited.
Tiger 7 exited.
Tiger 0 exited.
Tiger 5 exited.
Lion 2 exited.

--- Simulation finished ---
```

```console.log
./main.exe --test
=== RUNNING UNIT TESTS ===

Test 1: Animal Structure... PASSED SUCCESSFULLY
Test 2: Order Generation... PASSED SUCCESSFULLY
Test 3: Edge Cases... PASSED SUCCESSFULLY
Test 4: Arena Initialization... PASSED SUCCESSFULLY

=== UNIT TESTS COMPLETED ===

Press Enter to exit...
```
