#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>
#include <chrono>

using namespace std;

enum class AnimalType
{
  Lion,
  Tiger
};

struct Animal
{
  AnimalType type;
  int id;
};

mutex cout_mtx;

class Arena
{
public:
  Arena(const vector<Animal> &entry_order)
      : entry_order(entry_order),
        index(0),
        all_entered(false)
  {
  }

  void enter(AnimalType type, int id)
  {
    unique_lock<mutex> lock(mtx);

    cv.wait(lock, [this, type, id]
            { return index < entry_order.size() &&
                     entry_order[index].type == type &&
                     entry_order[index].id == id; });

    {
      lock_guard<mutex> cout_lock(cout_mtx);
      cout << (type == AnimalType::Lion ? "Lion " : "Tiger ") << id << " entered." << endl;
    }

    index++;
    cv.notify_all();

    if (index == entry_order.size())
    {
      all_entered = true;
      cv.notify_all();
    }
  }

  void waitForExitSignal()
  {
    unique_lock<mutex> lock(mtx);
    cv.wait(lock, [this]
            { return all_entered; });
  }

private:
  mutex mtx;
  condition_variable cv;
  const vector<Animal> &entry_order;
  size_t index;
  bool all_entered;
};

// Функция для генерации порядка входа (вынесена для тестирования)
vector<Animal> generateEntryOrder(int n)
{
  vector<Animal> lions;
  vector<Animal> tigers;

  for (int i = 0; i < n; ++i)
    lions.push_back({AnimalType::Lion, i});
  for (int i = 0; i < n - 1; ++i)
    tigers.push_back({AnimalType::Tiger, i});

  vector<Animal> entry_order;
  size_t li = 0, ti = 0;
  bool last_was_tiger = false;

  while (li < lions.size() || ti < tigers.size())
  {
    if (!last_was_tiger && ti < tigers.size())
    {
      entry_order.push_back(tigers[ti++]);
      last_was_tiger = true;
    }
    else if (li < lions.size())
    {
      entry_order.push_back(lions[li++]);
      last_was_tiger = false;
    }
    else if (ti < tigers.size())
    {
      entry_order.push_back(tigers[ti++]);
      last_was_tiger = true;
    }
  }

  return entry_order;
}

// ==================== ЮНИТ-ТЕСТЫ ====================

void testAnimalStructure()
{
  cout << "Test 1: Animal Structure... ";

  Animal lion{AnimalType::Lion, 5};
  Animal tiger{AnimalType::Tiger, 3};

  bool test_passed = true;

  if (lion.type != AnimalType::Lion)
  {
    cout << "FAILED - Lion type incorrect" << endl;
    test_passed = false;
  }
  if (lion.id != 5)
  {
    cout << "FAILED - Lion ID incorrect" << endl;
    test_passed = false;
  }
  if (tiger.type != AnimalType::Tiger)
  {
    cout << "FAILED - Tiger type incorrect" << endl;
    test_passed = false;
  }
  if (tiger.id != 3)
  {
    cout << "FAILED - Tiger ID incorrect" << endl;
    test_passed = false;
  }

  if (test_passed)
  {
    cout << "PASSED SUCCESSFULLY" << endl;
  }
}

void testOrderGeneration()
{
  cout << "Test 2: Order Generation... ";

  auto order = generateEntryOrder(3);
  bool test_passed = true;

  // Проверка количества животных
  int lion_count = 0, tiger_count = 0;
  for (const auto &animal : order)
  {
    if (animal.type == AnimalType::Lion)
      lion_count++;
    else
      tiger_count++;
  }

  if (lion_count != 3)
  {
    cout << "FAILED - Expected 3 lions, got " << lion_count << endl;
    test_passed = false;
  }
  if (tiger_count != 2)
  {
    cout << "FAILED - Expected 2 tigers, got " << tiger_count << endl;
    test_passed = false;
  }

  // Проверка что нет двух тигров подряд
  for (size_t i = 1; i < order.size(); ++i)
  {
    if (order[i].type == AnimalType::Tiger && order[i - 1].type == AnimalType::Tiger)
    {
      cout << "FAILED - Found two tigers in a row at position " << i << endl;
      test_passed = false;
      break;
    }
  }

  if (test_passed)
  {
    cout << "PASSED SUCCESSFULLY" << endl;
  }
}

void testEdgeCases()
{
  cout << "Test 3: Edge Cases... ";

  bool test_passed = true;

  // Тест для n=1 (только один лев, тигров нет)
  auto order_n1 = generateEntryOrder(1);
  if (order_n1.size() != 1)
  {
    cout << "FAILED - For n=1 expected 1 animal, got " << order_n1.size() << endl;
    test_passed = false;
  }
  if (order_n1[0].type != AnimalType::Lion)
  {
    cout << "FAILED - For n=1 expected Lion" << endl;
    test_passed = false;
  }

  // Тест для n=2
  auto order_n2 = generateEntryOrder(2);
  if (order_n2.size() != 3)
  { // 2 льва + 1 тигр
    cout << "FAILED - For n=2 expected 3 animals, got " << order_n2.size() << endl;
    test_passed = false;
  }

  if (test_passed)
  {
    cout << "PASSED SUCCESSFULLY" << endl;
  }
}

void testArenaInitialization()
{
  cout << "Test 4: Arena Initialization... ";

  vector<Animal> test_animals = {
      {AnimalType::Tiger, 0},
      {AnimalType::Lion, 0},
      {AnimalType::Tiger, 1}};

  try
  {
    Arena arena(test_animals);
    cout << "PASSED SUCCESSFULLY" << endl;
  }
  catch (const exception &e)
  {
    cout << "FAILED - Exception during initialization: " << e.what() << endl;
  }
}

void runAllTests()
{
  cout << "=== RUNNING UNIT TESTS ===" << endl;
  cout << endl;

  testAnimalStructure();
  testOrderGeneration();
  testEdgeCases();
  testArenaInitialization();

  cout << endl;
  cout << "=== UNIT TESTS COMPLETED ===" << endl;
  cout << endl;
}

// ==================== ОСНОВНАЯ ПРОГРАММА ====================

void animalThread(AnimalType type, int id, Arena &arena)
{
  arena.enter(type, id);
  arena.waitForExitSignal();

  thread_local random_device rd;
  thread_local mt19937 gen(rd());
  uniform_int_distribution<> dis(10, 100);
  this_thread::sleep_for(chrono::milliseconds(dis(gen)));

  {
    lock_guard<mutex> lock(cout_mtx);
    cout << (type == AnimalType::Lion ? "Lion " : "Tiger ") << id << " exited." << endl;
  }
}

int main(int argc, char *argv[])
{
  // Если есть аргумент --test, запускаем только тесты
  if (argc > 1 && string(argv[1]) == "--test")
  {
    runAllTests();
    cout << "Press Enter to exit...";
    cin.ignore();
    cin.get();
    return 0;
  }

  // Или запускаем быстрые тесты перед основной программой
  bool run_quick_tests = true;
  if (run_quick_tests)
  {
    cout << "=== QUICK PRE-TESTS ===" << endl;
    testOrderGeneration();
    cout << "=== QUICK PRE-TESTS FINISHED ===" << endl;
    cout << endl;
  }

  // Оригинальный код основной программы
  cout << "Enter n (number of lions, n>=1): ";
  int n;
  cin >> n;
  if (n < 1)
  {
    cerr << "n must be at least 1.\n";
    return 1;
  }

  auto entry_order = generateEntryOrder(n);
  Arena arena(entry_order);
  vector<thread> threads;

  // Создаем потоки для каждого животного в правильном порядке
  for (const auto &animal : entry_order)
  {
    threads.emplace_back(animalThread, animal.type, animal.id, ref(arena));
  }

  cout << "\n--- Simulation started ---\n\n";

  for (auto &t : threads)
    if (t.joinable())
      t.join();

  cout << "\n--- Simulation finished ---\n";
  cout << "Press Enter to exit...";
  cin.ignore();
  cin.get();

  return 0;
}
