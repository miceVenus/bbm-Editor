bbmEditor: bbmEditor.c
	$(CC) bbmEditor.c -o bbmEditor -Wall -Wextra -pedantic -std=c99

test: test.c
	$(CC) test.c -o test -Wall -Wextra -pedantic -std=c99