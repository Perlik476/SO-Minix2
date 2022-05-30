#include <vector>
#include <iostream>
#include <functional>
#include <cassert>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <lib.h>
#include <string.h>
#include <errno.h>
#include <minix/rs.h>
#include <sys/resource.h>

void work(double units) {
	uint64_t num_iters = uint64_t(units * 1.5e9);
	uint64_t a = 1;
	while (num_iters--)
		a *= 3;
	static volatile uint64_t work_result;
	work_result = a;
}

std::vector<int> get_order(std::vector<std::function<void ()>> thread_functions, std::vector<int> initial_buckets) {
	static int test_id = 0;
	std::cout << "running test " << test_id++ << "..." << std::endl;

	size_t n = thread_functions.size();
	assert(thread_functions.size() == initial_buckets.size());

	std::vector<int> pids(n);
	for (size_t i = 0; i < n; ++i) {
		pids[i] = fork();
		assert(pids[i] >= 0);
		if (pids[i] == 0) {
			if (initial_buckets[i] != -1)
				assert(set_bucket(initial_buckets[i]) == 0);
			work(3e-5);
			sleep(1);
			thread_functions[i]();
			_exit(0);
		}
	}

	std::vector<int> order(n, -1);
	for (int i = 0; i < n; ++i) {
		int wstatus;
		int child_pid = wait(&wstatus);
		assert(child_pid != -1);

		for (int j = 0; j < n; ++j)
			if (child_pid == pids[j])
				order[i] = j;
		assert(order[i] != -1);
	}
	return order;
}

std::function<void ()> just_work_for(double units) {
	return [=] {
		work(units);
	};
}

void print(std::vector<int> v) {
	std::cout << "{ ";
	for(int x : v)
		std::cout << x << ' ';
	std::cout << "}\n";
}

int main() {
	std::cout << "Each test should run in at most 60 seconds." << std::endl;

	// Test from 4-example.c (first testcase).
	assert(
		get_order(
			{just_work_for(15), just_work_for(0.5), just_work_for(0.5), just_work_for(0.5), just_work_for(0.5)},
			{1, 2, 2, 2, 2}
		).back() == 0
	);

	// Test from 4-example.c (second testcase).
	assert(
		get_order(
			{just_work_for(3), just_work_for(2), just_work_for(2), just_work_for(2), just_work_for(2)},
			{1, 2, 2, 2, 2}
		)[0] == 0
	);

	// Checking whether the default bucket of a process is bucket 0.
	assert(
		get_order(
			{just_work_for(1), just_work_for(1.5)},
			{-1, 1}
		)[0] == 1 
	); // (because there are some background user processes in the default bucket)

	// Checking whether changing buckets works.
	assert(
		get_order(
			{
				[] {
					assert(set_bucket(1) == 0);
					assert(set_bucket(2) == 0);
					assert(set_bucket(3) == 0);
					assert(set_bucket(4) == 0);
					work(4);
				},
				just_work_for(7),
				just_work_for(6),
				just_work_for(12)
			},
			{9, 3, 4, 5}
		) == (std::vector<int>{1, 0, 2, 3})
	);


	assert(
		get_order(
			{
				[] {
					assert(set_bucket(1) == 0);
					work(1);
					assert(set_bucket(2) == 0);
					work(1);
					assert(set_bucket(3) == 0);
					work(1);
				},
				just_work_for(5),
				just_work_for(5),
				just_work_for(5),
				just_work_for(7)
			},
			{9, 1, 2, 3, 4}
		)[4] == 4
	);

	assert(
		get_order(
			{
				[] {
					assert(set_bucket(1) == 0);
					work(2);
					assert(set_bucket(2) == 0);
					work(4);
				},
				just_work_for(20),
				just_work_for(20),
				just_work_for(20),
			},
			{9, 1, 2, 3}
		) == (std::vector<int>{0, 3, 1, 2})
	);

	// Changing buckets in presence of other processes.
	get_order(
		{
			[] {
				for (int i = 0; i < 1000; ++i)
					assert(set_bucket(1 + (i % 2)) == 0);
			},
			just_work_for(1),
			just_work_for(1),
			just_work_for(1),
			just_work_for(1),
			just_work_for(1),
		},
		{9, 1, 1, 1, 2, 2}
	);

	// Changing to all possible buckets.
	get_order(
		{
			[] {
				for (int i = 0; i < 1000; ++i)
					assert(set_bucket(i % NR_BUCKETS) == 0);
			},
			just_work_for(1)
		},
		{9, 1}
	);

	// Checking illegal bucket numbers.
	assert(set_bucket(-1) == -1);
	assert(errno == EINVAL);
	assert(set_bucket(NR_BUCKETS) == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(set_bucket(0) == 0);
	assert(errno == 0);

	// Checking setting and reading priority
	int pid = getpid();

	int priority = getpriority(PRIO_PROCESS, pid);
	assert(priority == 0);
	assert(errno == 0);

	printf("The following results don't have 1 correct value but are supposed to make sense.\n");
	printf("Set nice return value: %d\n", setpriority(PRIO_PROCESS, pid, 0));
	printf("Set nice errno: %d\n", errno);
	printf("Errno meaning: %s\n", strerror(errno));

	assert(
		get_order(
			{
				[] {
					int f1 = fork();
					int f2 = fork();
					work(2);
					if (f2) {
						wait(0);
					}
					else {
						exit(0);
					}
					if (f1) {
						wait(0);
					}
					else {
						exit(0);
					}
				},
				just_work_for(2),
				just_work_for(4)
			},
			{1, 1, 2}
		)[0] == 2
	);

	// Tests from 4-example.c, but for each pair of buckets greater than 0.
	for (int b0 = 1; b0 < NR_BUCKETS; ++b0)
		for (int b1 = 1; b1 < NR_BUCKETS; ++b1)
			if (b0 != b1) {
				assert(
					get_order(
						{just_work_for(15), just_work_for(0.5), just_work_for(0.5), just_work_for(0.5), just_work_for(0.5)},
						{b0, b1, b1, b1, b1}
					).back() == 0
				);
				assert(
					get_order(
						{just_work_for(3), just_work_for(2), just_work_for(2), just_work_for(2), just_work_for(2)},
						{b0, b1, b1, b1, b1}
					)[0] == 0
				);
			}

	std::cout << "OK\n";
}
