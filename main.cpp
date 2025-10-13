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

int main()
{
  cout << "Enter n (number of lions, n>=1): ";
  int n;
  cin >> n;
  if (n < 1)
  {
    cerr << "n must be at least 1.\n";
    return 1;
  }

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

  Arena arena(entry_order);
  vector<thread> threads;

  for (const auto &a : lions)
    threads.emplace_back(animalThread, a.type, a.id, ref(arena));
  for (const auto &a : tigers)
    threads.emplace_back(animalThread, a.type, a.id, ref(arena));

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
