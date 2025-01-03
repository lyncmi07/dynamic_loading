#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>

int add(int a, int b) {
	int value = a + b;
	value = value + a + b;

	return value;
}

int addPatched(int a, int b) {
	return a + b;
}

typedef int (*operation_t)(int, int);

int hook_function(operation_t originalFunc, operation_t newFunc) {
	size_t page_size = sysconf(_SC_PAGESIZE);
	void *aligned_address = (void*)((uintptr_t)originalFunc & ~(page_size - 1));

	// Change memory protection of original function
	if (mprotect(aligned_address, page_size, PROT_READ | PROT_EXEC | PROT_WRITE) == -1) {
		perror("mprotect write");
		return -1;
	}

	// Overwrite the original func tion with a jump to replacement
	unsigned char *func_bytes = (unsigned char*)originalFunc;
	func_bytes[0] = 0xE9; // JMP opcode
			      //
	// (-func_bytes+5) is because JMP instruction is relative to current position.
	// 1 for JMP instruction itself
	// +4 for the relative address the JMP instruction is pointing to.
	*(int32_t*)(func_bytes + 1) = (int32_t)((uintptr_t)newFunc - (uintptr_t)(func_bytes + 5));

	// Restore memory protection
	if (mprotect(aligned_address, page_size, PROT_READ | PROT_EXEC) == -1) {
		perror("mprotect read/exec");
		return -1;
	}

	return 0;

}

int main() {
	printf("Hello world!\n");

	printf("Calling add(5, 15): %d\n", add(5, 15));

	hook_function(add, addPatched);

	printf("Calling add(5, 15): %d\n", add(5, 15));
}
