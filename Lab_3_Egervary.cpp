#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <set>
#include <optional>
#include <exception>
#include "io.h"

using namespace std;

class AssignmentManager
{
public:
	void resize(int sizeTask, int sizeWorker)
	{
		taskAssignments.resize(sizeTask);
		workerAssignments.resize(sizeWorker);
	}
	optional<int> taskGetAssigned(int task)
	{
		return taskAssignments[task];
	}

	optional<int> workerGetAssigned(int worker)
	{
		return workerAssignments[worker];
	}

	void assign(int task, int worker)
	{

		if (optional<int> assignedWorker = taskAssignments[task])
			workerAssignments[*assignedWorker].reset();
		if (optional<int> assignedTask = workerAssignments[worker])
			taskAssignments[*assignedTask].reset();

		taskAssignments[task] = worker;
		workerAssignments[worker] = task;
	}

private:
	vector<optional<int>> taskAssignments;
	vector<optional<int>> workerAssignments;
};

class Egervary
{
public:
	void read(istream& in)
	{
		int sizeTask;
		in >> sizeTask;
		skipToEndl(in);

		tasks.resize(sizeTask);
		for (string& s : tasks)
			getline(in, s);

		int sizeWorker;
		in >> sizeWorker;
		skipToEndl(in);

		workers.resize(sizeWorker);
		for (string& s : workers)
			getline(in, s);

		usefulness.resize(sizeTask);
		for (vector<int>& v : usefulness)
		{
			v.resize(sizeWorker);
			for (int& i : v)
				in >> i;
		}

		loss.resize(sizeTask, vector<int>(sizeWorker));
		visitedTask.resize(sizeTask, false);
		visitedWorker.resize(sizeWorker, false);
		result.resize(sizeTask, sizeWorker);
	}

	vector<int> run()
	{
		resetVisited();

		findLoss();
		while (true)
		{
			reduction();
			while (alterChain()) {}
			if (allAssigned())
				break;

			egervary();
		}

		vector<int> result(tasks.size());
		for (int task = 0; task < tasks.size(); task++)
			result[task] = *this->result.taskGetAssigned(task);
		return result;
	}

	void print()
	{
		cout << "Задачи [" << tasks.size() << "]" << endl;
		for (const string& s : tasks)
			cout << "* " << s << endl;
		cout << endl;

		cout << "Работники [" << workers.size() << "]" << endl;
		for (const string& s : workers)
			cout << "* " << s << endl;
		cout << endl;

		for (const vector<int>& v : usefulness)
		{
			for (int i : v)
				cout << i << " ";
			cout << endl;
		}
		cout << endl;
	}

	int allUsefulness()
	{
		int allUsefulness = 0;

		for (int task = 0; task < tasks.size(); task++)
		{
			if (optional<int> worker = result.taskGetAssigned(task))
				allUsefulness += usefulness[task][*worker];
		}
		return allUsefulness;
	}

private:

	bool allAssigned()
	{
		for (int task = 0; task < tasks.size(); task++)
		{
			if (!result.taskGetAssigned(task))
				return false;
		}
		return true;
	}

	int findMax()
	{
		int m = 0;
		for (const vector<int>& v : usefulness)
			for (int i : v)
				m = max(m, i);
		return m;
	}

	void findLoss()
	{
		int max = findMax();
		for (int y = 0; y < tasks.size(); y++)
			for (int x = 0; x < workers.size(); x++)
				loss[y][x] = max - usefulness[y][x];
	}

	void reduction()
	{
		if (tasks.size() <= workers.size())
			for (vector<int>& v : loss)
			{
				int minEl = INT_MAX;
				for (int i : v)
				{
					minEl = min(minEl, i);
					if (minEl == 0)
						break;
				}
				if (minEl == 0)
					continue;
				for (int& i : v)
					i -= minEl;
			}

		if (workers.size() <= tasks.size())
			for (int x = 0; x < workers.size(); x++)
			{
				int minEl = INT_MAX;
				for (int y = 0; y < tasks.size(); y++)
				{
					minEl = min(minEl, loss[y][x]);
					if (minEl == 0)
						break;
				}
				if (minEl == 0)
					break;
				for (int y = 0; y < tasks.size(); y++)
					loss[y][x] -= minEl;
			}
	}

	bool alterChain()
	{
		bool found = false;
		for (int task = 0; task < tasks.size(); task++)
		{
			resetVisited();
			if (helpAlterChainMain(task))
				found = true;
		}
		return found;
	}
	bool helpAlterChainMain(int task)
	{
		if (visitedTask[task])
			return false;
		visitedTask[task] = true;

		for (int worker = 0; worker < workers.size(); worker++)
		{
			if (loss[task][worker] != 0)
				continue;

			if (!result.workerGetAssigned(worker) || helpAlterChainSide(worker))
			{
				result.assign(task, worker);
				return true;
			}
		}
		return false;
	}
	bool helpAlterChainSide(int worker)
	{
		if (visitedWorker[worker])
			return false;
		visitedWorker[worker] = true;

		if (optional<int> task = result.workerGetAssigned(worker))
		{
			if (helpAlterChainMain(*task))
				return true;
		}
		return false;
	}

	void resetVisited()
	{
		for (auto i : visitedTask)
			i = false;
		for (auto i : visitedWorker)
			i = false;
	}

	void egervary()
	{
		set<int> checkedTasks;
		set<int> checkedWorkers;

		for (int task = 0; task < tasks.size(); task++)
		{
			if (!result.taskGetAssigned(task))
				checkedTasks.insert(task);
		}

		while (true)
		{
			bool isChanged = false;

			for (int task : checkedTasks)
				for (int worker = 0; worker < workers.size(); worker++)
					if (loss[task][worker] == 0)
						if(checkedWorkers.insert(worker).second)
							isChanged = true;

			if (!isChanged)
				break;
			isChanged = false;

			for (int worker : checkedWorkers)
				if (optional<int> task = result.workerGetAssigned(worker))
					if(checkedTasks.insert(*task).second)
						isChanged = true;

			if (!isChanged)
				break;
		}

		int minIntersection = INT_MAX;
		for (int task : checkedTasks)
			for (int worker = 0; worker < workers.size(); worker++)
				if (checkedWorkers.find(worker) == checkedWorkers.end())
					minIntersection = min(minIntersection, loss[task][worker]);

		for (int worker : checkedWorkers)
			for (int task = 0; task < tasks.size(); task++)
				loss[task][worker] += minIntersection;

		for (int task : checkedTasks)
			for (int worker = 0; worker < workers.size(); worker++)
				loss[task][worker] -= minIntersection;
	}

	vector<vector<int>> usefulness;
	vector<vector<int>> loss;
	AssignmentManager result;
	vector<bool> visitedTask;
	vector<bool> visitedWorker;

	vector <string> tasks;
	vector <string> workers;
};

int main()
{
	setLocale();

	ifstream fin("input.txt");
	if (!fin)
	{
		cout << "can't read file" << endl;
		return 1;
	}

	Egervary e;
	e.read(fin);
	e.print();

	vector<int> assignment = e.run();

	for (int task = 0; task < assignment.size(); task++)
		cout << "Задача " << task + 1 << " - работник " << assignment[task] + 1 << endl;
	cout << endl;

	cout << "Оценка эффективности назначения: " << e.allUsefulness() << endl;
}