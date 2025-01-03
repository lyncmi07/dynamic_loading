#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdbool.h>

int main() {
	printf("Hello world!\n");


	while (true) {
		const char* libExample = "./libexample.so";
		void *exampleFileHandle = dlopen(libExample, RTLD_LAZY);
		if (!exampleFileHandle) {
			fprintf(stderr, "Error: %s\n", dlerror());
			exit(EXIT_FAILURE);
		}

		dlerror(); // Clear error

		typedef int (*operation_t)(int, int);
		operation_t add = (operation_t)dlsym(exampleFileHandle, "add");

		// Check for errors
		const char* error = dlerror();
		if(error != NULL) {
			fprintf(stderr, "Error: %s\n", error);
			dlclose(exampleFileHandle);
			exit(EXIT_FAILURE);
		}
	
		int result = add(5, 3);
		printf("Result of add: %d\n", result);
	

		dlclose(exampleFileHandle);


		int a;
		printf("Type a number to continue: ");
		scanf("%d", &a);
	}

	return 0;
}
