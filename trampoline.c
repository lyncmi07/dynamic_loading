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

unsigned char otherAddMachineCode[] = {
    0xf3, 0x0f, 0x1e, 0xfa, // endbr64
    0x55, 0x48, 0x89, 0xe5, // push %rbp
    0x48, 0x89, 0xe5,		// mov %rsp, %rbp
    0x89, 0x7d, 0xfc,		// move %edi,-0x4(%rbp)
    0x89, 0x75, 0xf8,       // mov    %esi,-0x8(%rbp)
    0x8b, 0x55, 0xfc,       // mov    -0x4(%rbp),%edx
    0x8b, 0x45, 0xf8,       // mov    -0x8(%rbp),%eax
    0x01, 0xd0,             // add    %edx,%eax
    0x83, 0xc0, 0x0a,       // add    $0xa,%eax
    0x5d,                   // pop    %rbp
    0xc3,                   // ret

    // 30:
    0xf3, 0x0f, 0x1e, 0xfa, // endbr64
    0x55, 0x48, 0x89, 0xe5, // push %rbp
    0x48, 0x89, 0xe5,		// mov %rsp, %rbp
    0x89, 0x7d, 0xfc,		// move %edi,-0x4(%rbp)
    0x89, 0x75, 0xf8,       // mov    %esi,-0x8(%rbp)
    0x8b, 0x55, 0xfc,       // mov    -0x4(%rbp),%edx
    0x8b, 0x45, 0xf8,       // mov    -0x8(%rbp),%eax
    0x01, 0xd0,             // add    %edx,%eax
    0x83, 0xc0, 0x0a,       // add    $0xa,%eax
    0x5d,                   // pop    %rbp
    0xc3,                   // ret
};

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

operation_t mapMachineCodeToFunction(unsigned char* code, unsigned int codeSize) {
    size_t page_size = sysconf(_SC_PAGESIZE);

    if (page_size < codeSize) {
      perror("Code frag too large");
      return NULL;
    }

    void* execMem = mmap(NULL, page_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (execMem == MAP_FAILED) {
      perror("mmap");
      return NULL;
    }

    memcpy(execMem, code, codeSize);

	if (mprotect(execMem, page_size, PROT_READ | PROT_EXEC) == -1) {
		perror("mprotect read/exec");
		return NULL;
	}

    return (operation_t)execMem;
}

int main() {
	printf("Hello world!\n");

	printf("Calling add(5, 15): %d\n", add(5, 15));

	hook_function(add, addPatched);

	printf("Calling add(5, 15): %d\n", add(5, 15));

    operation_t addFunc = add;

	printf("Calling addFunc(5, 15): %d\n", addFunc(5, 15));

    addFunc = mapMachineCodeToFunction(otherAddMachineCode, sizeof(otherAddMachineCode));

    void* mappedMemory = (void*)addFunc;
//    mappedMemory += 30; // Find the next function.

    addFunc = (operation_t)(mappedMemory + 30); // Find the next function;

	printf("Calling addFunc(5, 15): %d\n", addFunc(5, 15));
}
